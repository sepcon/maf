#include "IPCMessage.h"
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/BytesCarrier.h>
#include <maf/messaging/client-server/CSError.h>
#include <maf/utils/serialization/BASerializer.h>

namespace maf {
using namespace srz;

namespace messaging {
namespace ipc {

using ContentType = CSMessageContentBase::Type;
static Serializer &encodeAsError(Serializer &sr,
                                 const CSMsgContentBasePtr &msgContent) {
  auto error = static_cast<CSError *>(msgContent.get());
  return sr << error->description() << error->code();
}

static Deserializer &decodeError(Deserializer &ds,
                                 const std::shared_ptr<CSError> &error) {
  std::string desc;
  CSError::ErrorCode code;
  ds >> desc >> code;
  error->setDescription(std::move(desc));
  error->setCode(std::move(code));
  return ds;
}
srz::ByteArray IPCMessage::toBytes() {
  BASerializer sr;

  sr << serviceID() << operationID() << operationCode() << requestID()
     << sourceAddress() << (content() ? content()->type() : ContentType::NA);

  if (content()) {
    if (content()->type() == ContentType::Error) {
      encodeAsError(sr, content());
    } else if (content()->type() != ContentType::NA) {
      auto ipcContent = static_cast<BytesCarrier *>(content().get());

      sr << ipcContent->payload();
    } else {
      sr << ByteArray{};
    }
  } else {
    sr << ByteArray{};
  }

  return std::move(sr.mutableBytes());
}

bool IPCMessage::fromBytes(
    const std::shared_ptr<srz::ByteArray> &bytes) noexcept {
  BADeserializer ds(*bytes);
  try {
    ContentType contentType = ContentType::NA;
    ds >> _serviceID >> _operationID >> _operationCode >> _requestID >>
        _sourceAddress >> contentType;

    if (contentType == ContentType::Error) {
      auto error = std::make_shared<CSError>();
      decodeError(ds, error);
      setContent(std::move(error));
    } else {
      srz::ByteArray payload;
      ds >> payload;
      setContent(
          std::make_shared<BytesCarrier>(contentType, std::move(payload)));
    }
    return true;
  } catch (const std::exception &e) {
    MAF_LOGGER_ERROR(
        "Error occurred when trying to decode IPCMessage from bytes: ",
        e.what());
  }

  return false;
}

} // namespace ipc
} // namespace messaging
} // namespace maf
