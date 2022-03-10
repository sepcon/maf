#include <maf/ITCProxy.h>
#include <maf/ITCStub.h>
#include <maf/LocalIPCProxy.h>
#include <maf/LocalIPCStub.h>
#include <maf/Messaging.h>
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/ServiceStatusSignal.h>
#include <maf/threading/AtomicObject.h>
#include <maf/utils/DirectExecutor.h>
#include <maf/utils/TimeMeasurement.h>

#include <algorithm>
#include <future>
#include <iostream>
#include <map>
#include <set>

#define CATCH_CONFIG_MAIN
#include "catch/catch_amalgamated.hpp"

// clang-format off
#include <maf/messaging/client-server/CSContractDefinesBegin.mc.h>
REQUEST(string)
    INPUT((std::string, string_input))
    OUTPUT((std::string, string_output))
ENDREQUEST(string_request)

REQUEST(convert_ints_2_strings)
    INPUT((std::vector<int>, ints))
    OUTPUT((std::vector<std::string>, sints))
ENDREQUEST(convert_ints_2_strings)

REQUEST(map_string_vector_2_string)
using the_map = std::map<std::string, std::vector<int>>;
    INPUT((the_map, the_map))
    OUTPUT((std::string, map_as_string))
ENDREQUEST(map_string_vector)

VOID_REQUEST(to_be_aborted)

VOID_REQUEST(no_response)

SIGNAL(simple_string)
    ATTRIBUTES((std::string, str, "hello"))
ENDSIGNAL(simple_string)

VOID_SIGNAL(nothing)

SIGNAL(server_notify)
    using DetailsMap = std::map<std::string, std::string>;
    ATTRIBUTES
    (
        (std::string, notification),
        (DetailsMap, details)
    )
ENDSIGNAL(server_notify)

PROPERTY(some_string)
    STATUS((std::string, its_status))
ENDPROPERTY()

PROPERTY(varied_string)
	STATUS((std::string, its_status))
ENDPROPERTY()

PROPERTY(tobe_removed)
        STATUS((std::string, its_status, "default"))
ENDPROPERTY()

PROPERTY(not_set)
    enum type { _1, _2};
    STATUS((type, the_type))
ENDPROPERTY(not_set)
// clang-format on

#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>

using namespace maf::messaging;
using namespace maf::util;
namespace localipc = maf::localipc;
namespace itc = maf::itc;
using namespace std::chrono_literals;

static auto& serverProcessor() {
  static AsyncProcessor comp;
  return comp;
}

template <class PTrait>
class Tester {
  using Stub = maf::messaging::BasicStub<PTrait>;
  using Proxy = maf::messaging::BasicProxy<PTrait>;
  template <class Input>
  using Request = typename Stub::template Request<Input>;
  template <class Output>
  using Response = typename Proxy::template Response<Output>;

 public:
  Tester(std::shared_ptr<Stub> stub, std::shared_ptr<Proxy> proxy)
      : stub_{std::move(stub)}, proxy_{std::move(proxy)} {}

  void test() {
    maf::util::TimeMeasurement tm([](auto elapsed) {
      std::cout << "Total time for test = " << elapsed.count() / 1000 << "ms"
                << std::endl;
    });

    std::shared_ptr<Request<no_response_request>> noResponseRequestKeeper;

    auto stub = stub_->with(serverProcessor()->getExecutor());
    auto proxy = proxy_->with(maf::util::directExecutor());
    Availability serviceStatus;
    stub->template registerRequestHandler<string_request::input>(
        [](Request<string_request::input> request) {
          auto input = request.getInput();
          // 0. Confirm input is not null
          assert(input != nullptr);
          if (input->get_string_input() != "ignore_me") {
            // 1. Respond same value as input string
            request.template respond<string_request::output>(
                input->get_string_input());
          }
        });

    stub->template registerRequestHandler<
        convert_ints_2_strings_request::input>(
        [](Request<convert_ints_2_strings_request::input> request) {
          // 0. Confirm input is not null
          auto input = request.getInput();
          assert(input);
          // 1. create output object
          auto output = convert_ints_2_strings_request::make_output();
          // 2. Get reference to input data vector of integers
          const auto& ints = input->get_ints();
          // 3. transform integers to corresponding strings
          std::transform(std::begin(ints), std::end(ints),
                         std::back_inserter(output->get_sints()),
                         [](int val) { return std::to_string(val); });

          // 4. Respond output to request
          request.respond(std::move(output));
        });

    stub->template registerRequestHandler<
        map_string_vector_2_string_request::input>(
        [](Request<map_string_vector_2_string_request::input> request) {
          auto input = request.getInput();
          // 0. Confirm input is not null
          assert(input);
          // 1. create output object
          auto output = map_string_vector_2_string_request::make_output();
          // 3. dump the map to json like string
          output->set_map_as_string(input->dump());
          // 4. Respond output to request
          request.respond(std::move(output));
        });

    stub->template registerRequestHandler<no_response_request>(
        [&noResponseRequestKeeper, stub](auto request) {
          // Keep request to make it never be able to respond
          noResponseRequestKeeper =
              std::make_shared<Request<no_response_request>>(
                  std::move(request));
          stub->stopServing();
        });

    stub_->startServing();

    std::promise<void> serviceStatusSource;
    auto ftServiceStatusChangedSignal = serviceStatusSource.get_future();
    proxy->onServiceStatusChanged(
        [&serviceStatus, &serviceStatusSource](auto, Availability newStatus) {
          serviceStatus = newStatus;
          serviceStatusSource.set_value();
        });

    serviceStatusSignal(proxy)->waitIfNot(Availability::Available);

    SECTION("not_set_status_get") {
      auto status = proxy->template getStatus<not_set_property::status>();
      REQUIRE(!status);
    }

    SECTION("service_status") {
      REQUIRE(ftServiceStatusChangedSignal.wait_for(10ms) ==
              std::future_status::ready);
      REQUIRE(serviceStatus == Availability::Available);
    }

    SECTION("request_response_string") {
      // 1. Send string_request and expect same response
      auto inputString = std::string{"Hello world"};
      auto response = proxy->template sendRequest<string_request::output>(
          string_request::make_input(inputString));
      // 2. Confirm having output
      REQUIRE(response.isOutput());
      // 3. Confirm response's output == inputString
      REQUIRE(response.getOutput()->get_string_output() == inputString);
    }

    SECTION("request_response_vector") {
      auto ints = std::vector<int>{1, 2, 3};
      auto expectedStrings = std::vector<std::string>{};
      std::transform(std::begin(ints), std::end(ints),
                     std::back_inserter(expectedStrings),
                     [](int v) { return std::to_string(v); });

      // 1. Send integers to server
      auto response =
          proxy->template sendRequest<convert_ints_2_strings_request::output>(
              convert_ints_2_strings_request::make_input(std::move(ints)));
      // 2. Confirm having output
      REQUIRE(response.isOutput());

      // 3. Confirm response's output == expectedStrings
      REQUIRE(response.getOutput()->get_sints() == expectedStrings);
    }

    SECTION("request_response_map_string_2_vector") {
      map_string_vector_2_string_request::the_map themap = {
          {"one two three", {1, 2, 3}}, {"four five six", {4, 5, 6}}};

      auto input =
          map_string_vector_2_string_request::make_input(std::move(themap));
      // 1. Send themap to server
      auto response = proxy->template sendRequest<
          map_string_vector_2_string_request::output>(input);
      // 2. Confirm having output
      REQUIRE(response.isOutput());

      // 3. Confirm response's output == expectedResponseString
      REQUIRE(response.getOutput()->get_map_as_string() == input->dump());
    }

    SECTION("request_but_failed_to_response") {
      // 1. Send string_request and expect same response
      auto inputString = std::string{"ignore_me"};
      auto response = proxy->template sendRequest<string_request::output>(
          string_request::make_input(inputString));
      // 2. Confirm having output
      REQUIRE(response.isError());
      // 3. Confirm response's output == inputString
      REQUIRE(response.getError()->code() == CSErrorCode::ResponseIgnored);
    }

    SECTION("abort_request") {
      bool aborted = false;
      auto requestComeEventSource = std::make_shared<std::promise<void>>();
      auto requestComeEvent = requestComeEventSource->get_future();
      std::shared_ptr<Request<to_be_aborted_request>> requestHolder;
      stub->template registerRequestHandler<to_be_aborted_request>(
          [&aborted, &requestComeEventSource,
           &requestHolder](Request<to_be_aborted_request> request) {
            requestComeEventSource->set_value();
            request.onAborted([&aborted] { aborted = true; }, directExecutor());

            // Keep the request to make it not auto respond error in its
            // destructor
            requestHolder = std::make_shared<Request<to_be_aborted_request>>(
                std::move(request));
          });

      auto gotResponse = false;
      auto regid = proxy->template sendRequestAsync<to_be_aborted_request>(
          [&gotResponse](auto) { gotResponse = true; });

      REQUIRE(requestComeEvent.wait_for(10ms) == std::future_status::ready);

      REQUIRE(aborted == false);

      proxy->abortRequest(regid);
      std::this_thread::sleep_for(1ms);

      REQUIRE(aborted == true);

      // After aborted expect that the Request<to_be_aborted_request> cant
      // respond anymore
      REQUIRE(requestHolder->respond() == ActionCallStatus::InvalidCall);

      //        std::this_thread::sleep_for(1ms);

      // Due to request is aborted, then gotResponse will never meet
      REQUIRE(gotResponse == false);
    }

    SECTION("broad_cast_status_signal") {
      // 1. Send string_request and expect same response
      auto inputString = std::string{"ignore_me"};
      server_notify_signal::DetailsMap details = {{"key", "value"},
                                                  {"key1", "value1"}};
      std::promise<server_notify_signal::attributes> receivedAttribute;
      auto receivedAttributeFuture = receivedAttribute.get_future();
      auto sentAttribute =
          server_notify_signal::make_attributes(inputString, details);

      auto regid =
          proxy->template registerSignal<server_notify_signal::attributes>(
              [&receivedAttribute](server_notify_signal::attributes_cptr attr) {
                receivedAttribute.set_value(std::move(*attr));
              });

      // wait for signal register comes to server
      std::this_thread::sleep_for(1ms);

      stub->broadcastSignal(sentAttribute);

      REQUIRE(receivedAttributeFuture.wait_for(1000ms) ==
              std::future_status::ready);
      REQUIRE(receivedAttributeFuture.get() == *sentAttribute);

      proxy->unregister(regid);

      auto sentStatus = some_string_property::make_status("this is status");

      using PropertyReg =
          std::tuple<RegID, std::future<some_string_property::status>>;
      std::vector<PropertyReg> propertyRegs;
      constexpr size_t MAX_REGISTERS = 10;

      for (size_t i = 0; i < MAX_REGISTERS; ++i) {
        // The property's status will be cached in client side, then it must be
        // shared between all registers
        auto someStringStatusProms =
            std::make_shared<std::promise<some_string_property::status>>();
        auto someStringStatusFuture = someStringStatusProms->get_future();
        auto regID =
            proxy->template registerStatus<some_string_property::status>(
                [proms = std::move(someStringStatusProms)](
                    some_string_property::status_cptr status) mutable {
                  proms->set_value(std::move(*status));
                });

        propertyRegs.emplace_back(std::move(regID),
                                  std::move(someStringStatusFuture));
      }

      for (size_t i = 0; i < MAX_REGISTERS - 1; ++i) {
        auto& regIDi = std::get<0>(propertyRegs[i]);
        auto& regIDi1 = std::get<0>(propertyRegs[i + 1]);
        REQUIRE(regIDi != regIDi1);
      }

      stub_->setStatus(sentStatus);

      for (auto& [regID, propFuture] : propertyRegs) {
        SECTION("status_updated") {
          REQUIRE(propFuture.wait_for(100ms) == std::future_status::ready);
          REQUIRE(propFuture.get().dump() == sentStatus->dump());
        }
      }

      for (auto& [regID, propFuture] : propertyRegs) {
        proxy->unregister(regID);
      }

      sentStatus->set_its_status("sdfdsfdsfdsfds");
      stub_->setStatus(sentStatus);

      auto getBackedStatus =
          stub_->template getStatus<some_string_property::status>();
      REQUIRE(getBackedStatus);
      REQUIRE(*getBackedStatus == *sentStatus);

      std::this_thread::sleep_for(10ms);

      auto gotStatus =
          proxy->template getStatus<some_string_property::status>();
      REQUIRE(gotStatus);

      REQUIRE(*gotStatus == *sentStatus);

      std::set<std::string> statusesToUpdate = {"1", "2", "3", "4", "5"};
      maf::threading::AtomicObject<std::set<std::string>> updatedStatuses;
      auto getAllSignalSource = std::make_shared<std::promise<void>>();
      auto getAllSignal = getAllSignalSource->get_future();

      proxy->template registerStatus<varied_string_property::status>(
          [&updatedStatuses, getAllSignalSource,
           totalUpdate = statusesToUpdate.size()](
              varied_string_property::status_ptr status) {
            updatedStatuses->insert(status->get_its_status());
            if (updatedStatuses->size() == totalUpdate) {
              getAllSignalSource->set_value();
            }
          });

      for (auto& s : statusesToUpdate) {
        stub->template setStatus<varied_string_property::status>(s);
        std::this_thread::sleep_for(1ms);
      }

      REQUIRE(getAllSignal.wait_for(10ms) == std::future_status::ready);
      REQUIRE(statusesToUpdate == updatedStatuses.lockee());
    }

    SECTION("remove_property") {
      std::vector<std::string> expectedStatuses = {"set_status", ""};

      std::vector<std::string> statuses;
      stub->setStatus(tobe_removed_property::make_status("set_status"));
      proxy->template registerStatus<tobe_removed_property::status>(
          [&](tobe_removed_property::status_ptr status) {
            if (status) {
              statuses.push_back(status->get_its_status());
            } else {
              statuses.push_back("");
            }
          });
      std::this_thread::sleep_for(10ms);

      stub->template removeProperty<tobe_removed_property>(true);
      std::this_thread::sleep_for(5ms);

      REQUIRE(statuses == expectedStatuses);

      statuses.clear();
      stub->setStatus(tobe_removed_property::make_status("set_status"));
      std::this_thread::sleep_for(10ms);
      stub->template removeProperty<tobe_removed_property>(false);
      std::this_thread::sleep_for(5ms);
      expectedStatuses.pop_back();
      REQUIRE(statuses == expectedStatuses);
    }

    auto callstatus = ActionCallStatus{};
    auto response =
        proxy->template sendRequest<no_response_request>(&callstatus);

    SECTION("stopable_sync_request") {
      REQUIRE(callstatus == ActionCallStatus::ActionBroken);
    }

    SECTION("service_status") {
      REQUIRE(serviceStatus == Availability::Unavailable);
    }

    SECTION("server_side_notification") {
      auto dstub = stub->with(maf::util::directExecutor());
      const std::string someString = "server_side_notification";
      
      std::shared_ptr < std::promise<std::string>> prom;
      std::future<std::string> f;
      auto resetPromise = [&] {
        prom = std::make_shared<std::promise<std::string>>();
        f = prom->get_future();
      };
      resetPromise();
      
      auto con =
          dstub->template registerNotification<some_string_property::status>(
              [&](const some_string_property::status_ptr& ptr) {
                prom->set_value(ptr->get_its_status());
              });

      dstub->template setStatus<some_string_property::status>(someString);
      REQUIRE(f.wait_for(1ms) == std::future_status::ready);
      REQUIRE(f.get() == someString);

      resetPromise();
      dstub->template setStatus<some_string_property::status>(someString);
      REQUIRE(f.wait_for(1ms) == std::future_status::timeout);

      resetPromise();

      // disconnect won't received the noti
      con.disconnect();
      dstub->template setStatus<some_string_property::status>(someString + "new");
      REQUIRE(f.wait_for(1ms) == std::future_status::timeout);
      
      //for signal
      auto sigcon =
          dstub
              ->template registerNotification<simple_string_signal::attributes>(
                  [&](const simple_string_signal::attributes_ptr& ptr) {
                    prom->set_value(ptr->get_str());
                  });

      auto sigcon1 =
          dstub
              ->template registerNotification<nothing_signal>(
                  [&]{ prom->set_value("");
                  });

      dstub->template broadcastSignal<simple_string_signal::attributes>(someString);
      REQUIRE(f.wait_for(1ms) == std::future_status::ready);
      REQUIRE(f.get() == someString);

      resetPromise();
      dstub->template broadcastSignal<nothing_signal>();
      REQUIRE(f.wait_for(1ms) == std::future_status::ready);
      REQUIRE(f.get() == "");
      resetPromise();
      // disconnect won't received the noti
      sigcon.disconnect();
      dstub->template broadcastSignal<simple_string_signal::attributes>(
          someString +
                                                              "new");
      REQUIRE(f.wait_for(1ms) == std::future_status::timeout);
    }
  }

 private:
  std::shared_ptr<Stub> stub_;
  std::shared_ptr<Proxy> proxy_;
};

static constexpr auto ServiceIDTest = "request_response_test.service";
static struct TestInitiliazer {
  TestInitiliazer() { serverProcessor().launch(); }
  ~TestInitiliazer() { serverProcessor()->stop(); }
} testInitializer;

TEST_CASE("local.ipc.test") {
  using namespace localipc;
  Address addr{"maf.request_response_test.name", 0};
  auto stub = createStub(addr, ServiceIDTest);
  while (!stub) {
    std::this_thread::sleep_for(10ms);
    stub = createStub(addr, ServiceIDTest);
  };
  Tester<localipc::ParamTrait> tester{stub, createProxy(addr, ServiceIDTest)};
  tester.test();
}

TEST_CASE("itc.test") {
  using namespace itc;
  Tester<itc::ParamTrait> tester{createStub(ServiceIDTest),
                                 createProxy(ServiceIDTest)};
  tester.test();
}
