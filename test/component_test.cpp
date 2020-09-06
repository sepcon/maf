#include <maf/messaging/Component.h>
#include <maf/messaging/ComponentEx.h>
#include <maf/messaging/ComponentRequest.h>
#include <maf/messaging/MessageHandler.h>
#include <maf/messaging/Routing.h>
#include <maf/utils/TimeMeasurement.h>

#include <cstring>
#include <map>

#include "test.h"

using namespace maf::messaging;
using namespace maf::util;

struct program_start_msg {};
struct program_end_msg {};

struct data_convert_req_msg {
  ComponentInstance requester;
  std::string integer_string;
};

struct data_produced_msg {
  int integer;
};

struct cal_sume_done_msg {};

struct data_sum_msg {
  std::string component_name;
  int sum;
};

static thread_local int total = 0;
static int sum_of_all_consumers = 0;
static int range_begin = 1;
static int range_end = 1000;

static void postingMessages() {
  auto producerComp = Component::create("producer");

  std::vector<ComponentInstance> consumerComponents;
  consumerComponents.push_back(Component::create("consumer"));
  consumerComponents.push_back(Component::create("consumer2"));

  auto mainComponent = Component::create("main");

  for (auto consumerComp : consumerComponents) {
    consumerComp->connect<program_start_msg>([producerComp](auto) {
      producerComp->post(data_convert_req_msg{this_component::instance(), "0"});
    });

    consumerComp->connect<data_produced_msg>(
        [producerComp, mainComponent](data_produced_msg msg) {
          static thread_local int currentInteger = range_begin;
          total += msg.integer;
          if (currentInteger <= range_end) {
            producerComp->post(data_convert_req_msg{
                this_component::instance(), std::to_string(currentInteger++)});
          } else {
            mainComponent->post(cal_sume_done_msg{});
          }
        });

    consumerComp->connect<program_end_msg>([mainComponent](auto) {
      auto currentComponent = this_component::instance();
      mainComponent->post(data_sum_msg{currentComponent->id(), total});
      this_component::stop();
    });

    consumerComp->post(program_start_msg{});
  }

  producerComp->connect<data_convert_req_msg>([](data_convert_req_msg msg) {
    msg.requester->post(data_produced_msg{std::stoi(msg.integer_string)});
  });

  mainComponent->connect<cal_sume_done_msg>([&consumerComponents](auto) {
    static size_t totalConsumerDone = 0;
    if (++totalConsumerDone == consumerComponents.size()) {
      for (auto consumer : consumerComponents) {
        consumer->post(program_end_msg{});
      }
    }
  });

  mainComponent->connect<data_sum_msg>(
      [totalMsgCount = consumerComponents.size()](data_sum_msg msg) {
        static size_t totalMsgCame = 0;
        sum_of_all_consumers += msg.sum;
        if (++totalMsgCame == totalMsgCount) {
          this_component::stop();
        }
      });

  auto stopSignals = std::vector<AsyncComponent::StoppedSignal>{};

  stopSignals.emplace_back(AsyncComponent::launchComponent(producerComp));

  for (auto& consumer : consumerComponents) {
    stopSignals.emplace_back(AsyncComponent::launchComponent(consumer));
  }

  mainComponent->run();
  producerComp->stop();
}

void testPostingMessages() {
  TEST_CASE_B(tes_with_range_1_1000) {
    range_begin = 1;
    range_end = 1000;
    postingMessages();
    EXPECT(sum_of_all_consumers == (range_end * (range_end + 1)))
  }
  TEST_CASE_E()
}

void messageHandlerTest() {
  using namespace std;
  struct StringRequest {};
  struct SomeMsg {};

  struct HasParamMsg {
    int i;
    string s;
    double d;
  };

  AsyncComponent c = Component::create();
  c.launch();
  ComponentRequestSync<string, StringRequest> request(c.instance());
  ComponentRequestSync<void, HasParamMsg> hasParamRequest(c.instance());

  TEST_CASE_B(send_with_param) {
    HasParamMsg TestMsg = {1, "hello", 1.00};
    auto equal = false;
    RequestHandler<void, HasParamMsg> handler(c.instance());
    handler.connect([&TestMsg, &equal](const HasParamMsg& msg) {
      equal = msg.i == TestMsg.i && msg.s == TestMsg.s && msg.d == TestMsg.d;
    });

    hasParamRequest.send(TestMsg.i, TestMsg.s, TestMsg.d).wait();
    EXPECT(equal);
  }
  TEST_CASE_E(send_with_param)

  TEST_CASE_B(msg_handler) {
    {
      auto msgHandled = false;
      MessageHandler<SomeMsg> someMsgHandler{c.instance()};
      someMsgHandler.connect([&msgHandled] { msgHandled = true; });
      c->send<SomeMsg>().wait();
      EXPECT(msgHandled);
    }
    auto msgHandledSignal = c->send<SomeMsg>();
    // invalid due to request has already disconnected when handler goes out of
    // scope
    EXPECT(!msgHandledSignal.valid());
  }
  TEST_CASE_E(msg_handler)

  TEST_CASE_B(request_handler) {
    {
      const auto TestString = "This is test string";
      RequestHandler<string, StringRequest> stringRequestHandler{c.instance()};
      stringRequestHandler.connect([TestString](auto) { return TestString; });

      auto duplicateConnection = false;
      try {
        stringRequestHandler.connect([](auto) { return ""; });
      } catch (const DuplicateHandlerEx&) {
        duplicateConnection = true;
      }

      auto ret = request.send().get();
      EXPECT(duplicateConnection);

      EXPECT(ret.has_value() && ret.value() == TestString);
    }

    auto upcoming = request.send();
    // invalid due to request has already disconnected when handler goes out of
    // scope
    EXPECT(!upcoming.valid());
  }
  TEST_CASE_E(request_handler)
}
void requestTest() {
  using namespace std;
  using namespace std::chrono_literals;
  struct waitable_msg {};
  auto msgHandled = false;

  AsyncComponent logic = Component::create("logic");
  RequestHandlerGroup requestHandlers{logic.instance()};

  auto voidRequest = ComponentRequestSync<void, waitable_msg>{logic.instance()};
  auto intRequest = ComponentRequestSync<int, waitable_msg>{logic.instance()};

  logic.launch({});

  static constexpr auto WAIT_TIME = 1ms;
  {
    TimeMeasurement tm([](auto elapsedUS) {
      TEST_CASE_B(sync_execute) { EXPECT(elapsedUS > WAIT_TIME); }
      TEST_CASE_E(sync_execute)
    });

    requestHandlers.connect<waitable_msg>(
        [](waitable_msg) { this_thread::sleep_for(WAIT_TIME); });

    voidRequest.send(waitable_msg{}).wait();
    requestHandlers.disconnect();
  }

  TEST_CASE_B(sync_request) {
    requestHandlers.connect<waitable_msg>(
        [&msgHandled, &requestHandlers](const auto&) {
          this_thread::sleep_for(1ms);
          msgHandled = true;
          requestHandlers.disconnect();
        });
    auto upcoming = voidRequest.send(waitable_msg{});

    EXPECT(upcoming.valid());
    upcoming.get();
    EXPECT(msgHandled);
  }
  TEST_CASE_E(sync_request)

  TEST_CASE_B(async_request) {
    auto outputSource = promise<string>{};
    auto outputSink = outputSource.get_future();
    static const auto teststring = string{"hello world"};
    requestHandlers.connect<waitable_msg>([](auto) { return teststring; });

    auto asyncRequest =
        ComponentRequestAsync<string, waitable_msg>(logic.instance());
    asyncRequest.send(
        waitable_msg{},
        [&outputSource](string output) {
          outputSource.set_value(move(output));
        },
        logic->getExecutor());

    EXPECT(outputSink.valid());
    outputSink.wait_for(10ms);
    try {
      EXPECT(outputSink.get() == teststring);
    } catch (...) {
    }
    requestHandlers.disconnect();
  }
  TEST_CASE_E(async_request)

  TEST_CASE_B(stop_async_component) {
    requestHandlers.connect<waitable_msg>([](auto) {
      maf::test::log_rec() << "This will never show!";
      return 1;
    });

    logic->execute([] {
      std::this_thread::sleep_for(3ms);
      this_component::stop();
    });

    auto upcomingInt = intRequest.send(waitable_msg{});

    EXPECT(upcomingInt.valid());
    EXPECT(!upcomingInt.get().has_value());

    requestHandlers.disconnect();

    auto fired = false;
    logic->connect<waitable_msg>([&fired](const auto&) {
      fired = true;
      this_thread::sleep_for(100000h);
    });

    auto upcoming = voidRequest.send(waitable_msg{});

    EXPECT(!upcoming.valid());
    EXPECT(!fired);

    logic.wait();
    EXPECT(!logic.running())
  }
  TEST_CASE_E(stop_async_component)
}

void testRegisterUnregisterHandlers() {
  auto c = Component::create();

  static std::map<int, int> num2countMap;

  auto reg1 = c->connect<int>([](int x) { num2countMap[x]++; });
  auto reg2 = c->connect<int>([](int x) { num2countMap[x]++; });
  auto reg3 = c->connect<int>([](int x) { num2countMap[x]++; });

  ConnectionID reg4;
  reg4 = c->connect<int>([&](int x) {
    num2countMap[x]++;
    switch (x) {
      case 1:
        this_component::instance()->disconnect(reg1);
        this_component::post(2);
        break;
      case 2:
        this_component::instance()->disconnect(reg2);
        this_component::post(3);
        break;
      case 3:
        this_component::instance()->disconnect(reg3);
        this_component::post(4);
        break;
      case 4:
        this_component::stop();
        break;
      default:
        this_component::stop();
        break;
    }
  });

  c->run([] { this_component::post(1); });

  TEST_CASE_B(register_unregister) {
    EXPECT(num2countMap[1] == 4);
    EXPECT(num2countMap[2] == 3);
    EXPECT(num2countMap[3] == 2);
    EXPECT(num2countMap[4] == 1);
    EXPECT(num2countMap[5] == 0);
  }
  TEST_CASE_E(register_unregister)
}

void testAutoUnregister() {
  using namespace std;
  auto comp = Component::create();
  auto handlersMgr = new MessageHandlerGroup{comp};

  static map<int, int> i2count;
  static map<long, int> ld2count;
  static map<double, int> lf2count;
  static map<string, int> s2count;

  handlersMgr->connect<int>([](int x) { i2count[x]++; })
      .connect<int>([](int x) { i2count[x]++; })
      .connect<string>([](const string& s) { s2count[s]++; })
      .connect<string>([](const string& s) { s2count[s]++; })
      .connect<double>([](const double d) { lf2count[d]++; })
      .connect<long>([](const long l) { ld2count[l]++; })
      .connect<long>([](const long l) { ld2count[l]++; });

  comp->connect<string>([handlersMgr](const string& s) {
    if (s == "unreg_s") {
      handlersMgr->disconnect<string>();
    } else if (s == "unreg_i") {
      handlersMgr->disconnect<int>();
    } else if (s == "unreg_all") {
      delete handlersMgr;  // delete to unreg all
    }
  });

  comp->connect<string>([](const string& s) {
    if (s == "quit") {
      this_component::stop();
    }
  });

  comp->run([] {
    this_component::post(1);
    this_component::post(2);
    this_component::post(2.0);
    this_component::post(string{"hello"});
    this_component::post(string{"world"});
    this_component::post(-1);

    this_component::post(long(1));
    this_component::post(long(2));

    this_component::post(string{"unreg_i"});
    this_component::post(100);
    this_component::post(long(3));

    this_component::post(string{"unreg_s"});
    this_component::post<string>("after_unreg");

    this_component::post(string{"unreg_all"});
    this_component::post(100);
    this_component::post(long(100));
    this_component::post(100.00);
    this_component::post("last_string");

    this_component::post(string{"quit"});
  });

  TEST_CASE_B(auto_unreg) {
    EXPECT(i2count[1] == 2);
    EXPECT(i2count[2] == 2);
    EXPECT(ld2count[1] == 2);
    EXPECT(ld2count[2] == 2);
    EXPECT(ld2count[3] == 2);
    EXPECT(ld2count[100] == 0);
    EXPECT(lf2count[2.0] == 1);
    EXPECT(i2count[100] == 0);
    EXPECT(lf2count[100.0] == 0);
    EXPECT(s2count["hello"] == 2);
    EXPECT(s2count["world"] == 2);
    EXPECT(s2count["after_unreg"] == 0);
    EXPECT(s2count["last_string"] == 0);
  }
  TEST_CASE_E(auto_unreg)
}

void sendMessageTest() {
  struct waitable_msg {};
  struct non_waitable_msg {};
  struct sum_int_msg {
    int index;
  };

  int count = 0;
  int sum = 0;
  bool gotHandlingDoneSignal = false;

  const int totalCount = 10;

  AsyncComponent logic = Component::create("logic");
  for (int i = 0; i < totalCount; ++i) {
    logic->connect<waitable_msg>([&count] { ++count; });
  }

  logic->connect<sum_int_msg>(
      [&sum](const sum_int_msg& msg) { sum += msg.index; });

  logic.launch();

  TEST_CASE_B(component_send_message) {
    logic->send<waitable_msg>()
        .then([&gotHandlingDoneSignal] { gotHandlingDoneSignal = true; })
        .wait();

    EXPECT(totalCount == count);
    EXPECT(gotHandlingDoneSignal);
    gotHandlingDoneSignal = false;

    auto handledSignal = logic->send<non_waitable_msg>().then(
        [&gotHandlingDoneSignal] { gotHandlingDoneSignal = true; });

    EXPECT(!handledSignal.valid());

    std::vector<Component::MessageHandledSignal> handledSignals;
    for (int i = 1; i <= 10; ++i) {
      handledSignals.push_back(logic->send<sum_int_msg>(i));
    }

    for (auto& sig : handledSignals) {
      sig.waitFor(std::chrono::milliseconds{1});
    }

    EXPECT(sum == 10 * 11 / 2);
  }
  TEST_CASE_E(component_send_message)

  TEST_CASE_B(send_message_from_current_component) {
    struct NotBlockedMsg {};
    auto msgHandledEventSource = std::make_shared<std::promise<int> >();

    auto msgHandledSignal = msgHandledEventSource->get_future();
    logic->connect<NotBlockedMsg>(
        [event = std::move(msgHandledEventSource)]() mutable {
          event->set_value(100);
        });
    logic->execute(
        [] { this_component::instance()->send<NotBlockedMsg>().wait(); });

    EXPECT(msgHandledSignal.get() == 100);
  }
  TEST_CASE_E(send_message_from_current_component)
}

int main() {
  //  using namespace maf::logging;
  //  maf::logging::init(LOG_LEVEL_FROM_WARN | LOG_LEVEL_VERBOSE,
  //                     [](const auto& msg) { std::cout << msg << std::endl;
  //                     });
  maf::test::init_test_cases();
  messageHandlerTest();
  requestTest();
  testPostingMessages();
  testRegisterUnregisterHandlers();
  testAutoUnregister();
  sendMessageTest();

  return 0;
}
