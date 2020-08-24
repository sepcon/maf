#include <maf/LocalIPCProxy.h>
#include <maf/LocalIPCStub.h>
#include <maf/Messaging.h>
#include <maf/messaging/client-server/ServiceStatusSignal.h>
#include <maf/utils/TimeMeasurement.h>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;
namespace localipc = maf::localipc;
using namespace maf::messaging;

static const auto DataTransmissionServerAddress = Address{"SimpleStubProxy", 0};
static const auto DataTransmissionServiceID = "helloworld";

// clang-format off
#include <maf/messaging/client-server/CSContractDefinesBegin.mc.h>

REQUEST(begin_write_file)
    INPUT( (std::string, dest_file) )
ENDREQUEST(begin_copy_file)

REQUEST(write_file)
    using buffer_type = std::string;
    INPUT((buffer_type, buffer))
    OUTPUT((size_t, written_bytes_count))
ENDREQUEST(write_file)

REQUEST(end_write_file)
    INPUT((std::string, dest_file))
ENDREQUEST(end_copy_file)

#include <maf/messaging/client-server/CSContractDefinesEnd.mc.h>

// clang-format on

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Not enough argument!" << std::endl;
    return -1;
  }

  auto sourceFile = fs::u8path(argv[1]);
  auto destFile = std::string{argv[2]};

  maf::logging::init(
      maf::logging::LOG_LEVEL_DEBUG /*| maf::logging::LOG_LEVEL_VERBOSE*/ |
          maf::logging::LOG_LEVEL_FROM_WARN,
      [](const std::string &msg) { std::cout << "[MAF] " << msg << std::endl; },
      [](const std::string &msg) {
        std::cerr << "[MAF] " << msg << std::endl;
      });

  constexpr auto MAX_BUFFER_SIZE = size_t{500000};

  MAF_LOGGER_DEBUG("Start copying file ", sourceFile, " to ", destFile);

  auto proxy = localipc::createProxy(DataTransmissionServerAddress,
                                     DataTransmissionServiceID);

  AsyncComponent serverComponent = Component::create("Server");

  auto stub = localipc::createStub(DataTransmissionServerAddress,
                                   DataTransmissionServiceID,
                                   serverComponent->getExecutor());

  auto dataTransmissionServiceStatusSignal = serviceStatusSignal(proxy);

  std::ofstream writeStream;
  stub->registerRequestHandler<begin_write_file_request::input>(
      [&writeStream](
          localipc::Request<begin_write_file_request::input> request) {
        if (auto input = request.getInput()) {
          auto filepath = input->get_dest_file();
          if (!filepath.empty()) {
            writeStream.open(filepath, std::ios_base::binary);
            if (writeStream.good()) {
              MAF_LOGGER_DEBUG("Successfully opened file ", filepath);
              request.respond();
            } else {
              request.error("Failed to open file",
                            CSErrorCode::OperationFailed);
              writeStream.close();
            }
          } else {
            request.error("File path must not be empty!",
                          CSErrorCode::InvalidParam);
          }
        } else {
          request.error("Missing input", CSErrorCode::InvalidParam);
        }
      });

  stub->registerRequestHandler<write_file_request::input>(
      [&writeStream](localipc::Request<write_file_request::input> request) {
        if (writeStream.is_open()) {
          if (auto input = request.getInput()) {
            auto &buffer = input->get_buffer();
            if (buffer.size() > 0) {
              MAF_LOGGER_DEBUG("Write chunk of ", buffer.size(), " bytes");
              writeStream.write(buffer.data(),
                                static_cast<std::streamsize>(buffer.size()));
              if (!writeStream.fail()) {
                request.respond<write_file_request::output>(buffer.size());
              }
            } else {
              request.error("buffer is empty", CSErrorCode::InvalidParam);
            }
          }
        }
      });

  stub->registerRequestHandler<end_write_file_request>(
      [&writeStream](auto request) {
        MAF_LOGGER_DEBUG("Received close fille request ");
        writeStream.close();
        request.respond();
      });

  serverComponent.launch();
  stub->startServing();

  if (dataTransmissionServiceStatusSignal->waitIfNot(Availability::Available)
          .isReady()) {
    maf::util::TimeMeasurement tm{[](auto elapsedUs) {
      std::cout << "TIME OF execution = " << elapsedUs.count() / 1000 << "ms "
                << std::endl;
    }};
    auto readStream = std::ifstream{sourceFile, std::ios_base::binary};
    if (readStream.is_open()) {
      if (auto response = proxy->sendRequest<begin_write_file_request>(
              begin_write_file_request::make_input(destFile));
          !response.isError()) {
        auto ec = std::error_code{};
        auto filesize = fs::file_size(sourceFile, ec);
        if (filesize > 0) {
          auto writeInput = write_file_request::make_input();
          auto &buffer = writeInput->get_buffer();
          auto buffersize = MAX_BUFFER_SIZE;
          if (filesize < MAX_BUFFER_SIZE) {
            buffersize = static_cast<size_t>(filesize);
          }

          auto callstatus = ActionCallStatus::Success;

          do {
            if (buffer.size() < buffersize) {
              buffer.resize(buffersize);
            }
            auto readCount = readStream
                                 .read(&(buffer[0]),
                                       static_cast<std::streamsize>(buffersize))
                                 .gcount();
            MAF_LOGGER_DEBUG("Read ", readCount, " bytes!");
            if (readCount == 0) {
              MAF_LOGGER_DEBUG("Theres no more data to read");
              break;
            } else if (readCount < buffersize) {
              MAF_LOGGER_DEBUG("Seems to get last chunk of data!(", readCount,
                               " bytes)");
              buffer.resize(static_cast<size_t>(readCount));
            }

            auto response = proxy->sendRequest<write_file_request::output>(
                writeInput, &callstatus);

            if (callstatus == ActionCallStatus::Success) {
              if (response.isError()) {
                MAF_LOGGER_DEBUG("Error writing buffer");
                break;
              }
            } else {
              MAF_LOGGER_DEBUG("Failed to send request: ", callstatus);
              break;
            }
          } while (!readStream.eof());

          if (callstatus != ActionCallStatus::ServiceUnavailable &&
              callstatus != ActionCallStatus::ReceiverUnavailable) {
            proxy->sendRequest<end_write_file_request>(
                end_write_file_request::make_input(destFile), &callstatus);
            if (callstatus == ActionCallStatus::Success) {
              MAF_LOGGER_DEBUG("Succesfully closed the file ", destFile);
            } else {
              MAF_LOGGER_ERROR("Failed to request close file ", destFile,
                               " with error ", callstatus);
            }
          }
        }
      }
    }
  }

  serverComponent->stop();
  return 0;
}
