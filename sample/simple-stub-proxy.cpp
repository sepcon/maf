#include <filesystem>
#include <iostream>
#include <maf/messaging/SyncCallbackExecutor.h>
#include <maf/messaging/client-server/ServiceStatusSignal.h>
#include <maf/messaging/client-server/ipc/local/Proxy.h>
#include <maf/messaging/client-server/ipc/local/Stub.h>
#include <maf/utils/TimeMeasurement.h>
#include <maf/utils/serialization/Serializer1.h>

namespace fs = std::filesystem;

template <class OByteStream, class IByteStream>
struct maf::srz::Serializer<OByteStream, IByteStream, fs::path, void> {
  using SrType = fs::path;
  using CharType = SrType::value_type;
  using StringType = std::basic_string<CharType>;

  SizeType serializedSize(const SrType &path) {
    return static_cast<SizeType>(std::char_traits<CharType>::length(path.c_str()) * sizeof(CharType));
  }

  void serialize(OByteStream &os, const SrType &value) {
    Serializer<OByteStream, IByteStream, StringType>{}.serialize(
        os, value.string<CharType>());
  }

  SrType deserialize(IByteStream &is, bool &good) {
    return Serializer<OByteStream, IByteStream, StringType>{}.deserialize(is,
                                                                          good);
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
    using Path = std::string;
    using Paths = std::vector<Path>;
    INPUT(
        (Path, dir),
        (int, index)
        )
    OUTPUT(
        (Paths, paths),
        (int, index)
        )
ENDREQUEST(list_dir)

SIGNAL(client_request_list_dir)
    ATTRIBUTES((fs::path, dir))
ENDSIGNAL(client_request_list_dir)

REQUEST(write_bigdata)
    INPUT((std::shared_ptr<std::vector<std::string>>, string_list))
ENDREQUEST(write_bigdata)
#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>

// clang-format on

using namespace maf::messaging;
using namespace maf::messaging::ipc::local;
using namespace std::chrono_literals;

auto requestListDir(ProxyPtr proxy, const fs::path &dir)
    -> list_dir_request::Paths {
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

static auto createBigString() {
  auto vec = std::make_shared<std::vector<std::string>>();
  for (int i = 0; i < 10; ++i) {
    vec->push_back("hello world, this is cody.nguyen");
  }
  return vec;
}

static const auto SERVER_ADDRESS = Address{"SimpleStubProxy", 0};
static const auto SID_HELLO_WORLD = "helloworld";
static auto BigStringList = createBigString();
#include <iostream>

int main() {
  maf::logging::init(
      maf::logging::LOG_LEVEL_DEBUG /*| maf::logging::LOG_LEVEL_VERBOSE |
          maf::logging::LOG_LEVEL_FROM_WARN*/
      ,
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

    stub->with(syncExecutor())
        ->registerRequestHandler<write_bigdata_request::input>(
            [](Request<write_bigdata_request::input> request) {
              if (auto input = request.getInput()) {
                std::cout << "Total string = "
                          << input->get_string_list()->size() << std::endl;
                request.respond();
              } else {
                request.error("No input", CSErrorCode::InvalidParam);
              }
            });
    // We can create different stub with other executor to execute the callback
    stub->with(syncExecutor())
        ->registerRequestHandler<list_dir_request::input>(
            [stub](Request<list_dir_request::input> request) {
              if (auto input = request.getInput()) {
                ++totalRequestReceived;
                stub->broadcastSignal(
                    client_request_list_dir_signal::make_attributes(
                        input->get_dir()));
                auto dir = input->get_dir();
                auto ec = std::error_code{};
                if (fs::exists(dir, ec)) {
                  if (fs::is_directory(dir, ec)) {
                    auto output = list_dir_request::make_output(
                        list_dir_request::Paths{}, input->get_index());
                    for (auto dirEntry : fs::directory_iterator(dir, ec)) {
                      if (dirEntry.is_regular_file(ec)) {
                        output->get_paths().emplace_back(
                            dirEntry.path().string<char>());
                      }
                    }
                    request.respond(std::move(output));

                  } else if (fs::is_regular_file(dir, ec)) {
                    request.error(dir + " is a file not a dir",
                                  CSErrorCode::InvalidParam);
                  } else {
                    request.error("Unknown type of " + dir);
                  }
                } else {
                  request.error(dir + " doesn't exist!",
                                CSErrorCode::InvalidParam);
                }
              }
            });

    // startServing to make service provider ready for client to send requests
    stub->startServing();

    auto sssignal = serviceStatusSignal();
    proxy->registerServiceStatusObserver(sssignal);

    if (sssignal->waitTill(Availability::Available, 1000)) {
      auto tm = maf::util::TimeMeasurement([](auto t) {
        MAF_LOGGER_DEBUG(
            "Total time is: ", static_cast<double>(t.count()) / 1000, "ms");
      });

      auto px = proxy->with(syncExecutor());

      for (int i = 0; i < 100; ++i)
        px->sendRequest<write_bigdata_request>(
            write_bigdata_request::make_input(BigStringList));

      std::cout << "Complete request" << std::endl;
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
