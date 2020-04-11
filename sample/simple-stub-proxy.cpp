#include <filesystem>
#include <iostream>
#include <maf/messaging/SyncCallbackExecutor.h>
#include <maf/messaging/client-server/ServiceStatusSignal.h>
#include <maf/messaging/client-server/ipc/LocalIPCProxy.h>
#include <maf/messaging/client-server/ipc/LocalIPCStub.h>
#include <maf/utils/TimeMeasurement.h>
#include <maf/utils/serialization/SerializationTrait.h>

namespace fs = std::filesystem;

template <> struct maf::srz::SerializationTrait<fs::path, void> {
  using SizeTypeSerializer = SerializationTrait<SizeType>;
  using ValueType = fs::path;
  using StringType = std::basic_string<fs::path::value_type>;
  inline static SizeType serializeSizeOf(const ValueType &value) noexcept {
    return SerializationTrait<StringType>::serializeSizeOf(
        value.string<fs::path::value_type>());
  }

  inline static SizeType serialize(char *startp,
                                   const ValueType &value) noexcept {
    return SerializationTrait<StringType>::serialize(startp, value);
  }

  inline static ValueType
  deserialize(const char **startp, const char **lastp,
              const RequestMoreBytesCallback &requestMoreBytes = {}) {
    ValueType value = SerializationTrait<StringType>::deserialize(
        startp, lastp, requestMoreBytes);
    return value;
  }
};

template <> struct maf::srz::DumpHelper<fs::path, void> {
  inline static void dump(const fs::path &value, int /*indentLevel*/,
                          std::string &strOut) noexcept {
    // must be taken care for case of wstring
    strOut += hlp::quoteAllEscapes(value.string<char>());
  }
};

// clang-format off
#include <maf/messaging/client-server/CSContractDefinesBegin.mc.h>
REQUEST(list_dir)
    INPUT(
        (fs::path, dir),
        (int, index)
        )
    OUTPUT(
        (std::vector<fs::path>, paths),
        (int, index)
        )
ENDREQUEST(list_dir)

SIGNAL(client_request_list_dir)
    ATTRIBUTES((fs::path, dir))
ENDSIGNAL(client_request_list_dir)

#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>

// clang-format on

using namespace maf::messaging;
using namespace maf::messaging::ipc::local;
using namespace std::chrono_literals;

auto requestListDir(ProxyPtr proxy, const fs::path &dir)
    -> std::vector<fs::path> {
  auto response = proxy->sendRequest<list_dir_request::output>(
      list_dir_request::make_input(dir.string<char>(), 0));
  if (auto output = response.getOutput()) {
    return output->get_paths();
  } else if (auto error = response.getError()) {
    MAF_LOGGER_DEBUG("Got Error from service ", error->dump());
  }
  return {};
};

void listDir(const fs::path &dir, ProxyPtr proxy) {
  auto paths = requestListDir(proxy, dir);
  for (const auto &p : paths) {
    listDir(p, proxy);
  }
}

static const auto SERVER_ADDRESS = Address{"SimpleStubProxy", 0};
static const auto SID_HELLO_WORLD = "helloworld";

int main() {
  maf::logging::init(
      maf::logging::LOG_LEVEL_DEBUG | maf::logging::LOG_LEVEL_VERBOSE |
          maf::logging::LOG_LEVEL_FROM_WARN,
      [](const std::string &msg) { std::cout << "[MAF] " << msg << std::endl; },
      [](const std::string &msg) {
        std::cerr << "[MAF] " << msg << std::endl;
      });

  // In same thread, proxy must be created prior to stub to make sure server
  // object is destructed before client object. by that server will have chance
  // to clean up the resource before client sending the last message when it is
  // destructed, by sending message after listening thread of server has already
  // exited but the server object has not been destructed yet, will block the
  // client forever, the program will have no chance to exit!
  auto proxy = createProxy(SERVER_ADDRESS, SID_HELLO_WORLD);

  auto stub = createStub(SERVER_ADDRESS, SID_HELLO_WORLD);
  static int totalRequestReceived = 0;
  static int totalSignalReceived = 0;

  // We can create different stub with other executor to execute the callback
  stub->with(syncExecutor())->registerRequestHandler<list_dir_request::input>(
      [stub](Request<list_dir_request::input> request) {
        if (auto input = request.getInput()) {
          ++totalRequestReceived;
          stub->broadcastSignal(client_request_list_dir_signal::make_attributes(
              input->get_dir()));
          auto dir = input->get_dir();
          auto ec = std::error_code{};
          if (fs::exists(dir, ec)) {
            if (fs::is_directory(dir, ec)) {
              auto output = list_dir_request::make_output(
                  std::vector<fs::path>{}, input->get_index());
              for (auto dirEntry : fs::directory_iterator(dir, ec)) {
                if (dirEntry.is_regular_file(ec)) {
                  output->get_paths().emplace_back(dirEntry.path());
                }
              }
              request.respond(std::move(output));

            } else if (fs::is_regular_file(dir, ec)) {
              request.error(dir.string<char>() + " is a file not a dir",
                            CSErrorCode::InvalidParam);
            } else {
              request.error("Unknown type of " + dir.string<char>());
            }
          } else {
            request.error(dir.string<char>() + " doesn't exist!",
                          CSErrorCode::InvalidParam);
          }
        }
      });

  // startServing to make service provider ready for client to send requests
  stub->startServing();

  if (serviceStatusSignal(proxy)->waitTill(Availability::Available) ) {
    auto tm = maf::util::TimeMeasurement([](auto t) {
      MAF_LOGGER_DEBUG("Total time is: ", static_cast<double>(t.count()) / 1000,
                       "ms");
    });

    if (proxy->with(syncExecutor())
            ->registerSignal<client_request_list_dir_signal::attributes>(
                [](client_request_list_dir_signal::attributes_ptr attr) {
                  totalSignalReceived++;
                  auto path = attr ? attr->get_dir() : fs::path{};
                  MAF_LOGGER_DEBUG("Received signal from server: ", path);
                })
            .valid()) {
      listDir(R"(.)", proxy);
    }
  }

  assert(totalSignalReceived == totalRequestReceived);
  stub->stopServing();

  return 0;
}
