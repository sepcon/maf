#include "IPCMessage.h"
#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSError.h>
#include <maf/messaging/client-server/ipc/local/IncomingMsgContent.h>
#include <maf/messaging/client-server/ipc/local/OutgoingMsgContent.h>
#include <maf/utils/serialization/IByteStream.h>
#include <maf/utils/serialization/OByteStream.h>
#include <maf/utils/serialization/Serializer1.h>

namespace maf {
using namespace srz;
namespace messaging {
namespace ipc {

using Serializer = SR<OByteStream>;
using Deserializer = DSR<IByteStream>;

using ContentType = CSMessageContentBase::Type;
static Serializer &encodeAsError(Serializer &sr,
                                 const CSMsgContentBasePtr &msgContent) {
  auto error = static_cast<CSError *>(msgContent.get());
  return sr << error->description() << error->code();
}

static std::shared_ptr<CSError> decodeAsError(Deserializer &ds) {
  auto desc = std::string{};
  auto code = CSError::ErrorCode::Unknown;
  ds >> desc >> code;
  return std::shared_ptr<CSError>{new CSError{std::move(desc), code}};
}

srz::ByteArray IPCMessage::toBytes() {
  srz::OByteStream oss;
  Serializer sr(oss);

  sr.serializeBatch(serviceID(), operationID(), operationCode(), requestID(),
                    sourceAddress(),
                    (content_ ? content_->type() : ContentType::NA));

  if (content_) {
    if (content_->type() == ContentType::Error) {
      encodeAsError(sr, content_);
    } else if (content_->type() != ContentType::NA) {
      auto ipcContent = static_cast<OutgoingMsgContent *>(content_.get());
      ipcContent->serialize(oss);
    } else {
      sr << ByteArray{};
    }
  } else {
    sr << ByteArray{};
  }

  return std::move(oss.bytes());
}

bool IPCMessage::fromBytes(ByteArray &&bytes) noexcept {
  auto iss = std::make_shared<IByteStream>(std::move(bytes));
  Deserializer ds(*iss);
  try {
    ContentType contentType = ContentType::NA;
    ds >> serviceID_ >> operationID_ >> operationCode_ >> requestID_ >>
        sourceAddress_ >> contentType;
    if (contentType == ContentType::Error) {
      setContent(decodeAsError(ds));
    } else {
      setContent(
          std::make_shared<IncomingMsgContent>(contentType, std::move(iss)));
    }
    return true;
  } catch (const std::exception &e) {
    MAF_LOGGER_ERROR(
        "Error occurred when trying to translate IPCMessage from bytes: ",
        e.what());
  }

  return false;
}

} // namespace ipc
} // namespace messaging
} // namespace maf
