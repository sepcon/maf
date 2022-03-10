#include "LocalIPCMessage.h"

#include <maf/logging/Logger.h>
#include <maf/messaging/client-server/CSError.h>
#include <maf/messaging/client-server/ipc/local/IncomingPayload.h>
#include <maf/messaging/client-server/ipc/local/OutgoingPayload.h>
#include <maf/utils/serialization/IByteStream.h>
#include <maf/utils/serialization/OByteStream.h>
#include <maf/utils/serialization/Serializer.h>

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

using namespace maf::srz;

using Serializer = SR<OByteStream>;
using Deserializer = DSR<IByteStream>;

using ContentType = CSPayloadType;
static Serializer &encodeAsError(Serializer &sr,
                                 const CSPayloadIFPtr &msgContent) {
  auto error = static_cast<CSError *>(msgContent.get());
  return sr << error->description() << error->code();
}

static std::shared_ptr<CSError> decodeAsError(Deserializer &ds) {
  auto desc = std::string{};
  auto code = CSError::ErrorCode::Unknown;
  ds >> desc >> code;
  return std::shared_ptr<CSError>{new CSError{std::move(desc), code}};
}

srz::Buffer LocalIPCMessage::toBytes() noexcept {
  srz::OByteStream oss;
  Serializer sr(oss);

  sr.serializeBatch(serviceID(), operationID(), operationCode(), requestID(),
                    sourceAddress(),
                    (payload_ ? payload_->type() : ContentType::NA));

  if (payload_) {
    if (payload_->type() == ContentType::Error) {
      encodeAsError(sr, payload_);
    } else if (payload_->type() != ContentType::NA) {
      auto ipcContent = static_cast<OutgoingPayload *>(payload_.get());
      ipcContent->serialize(oss);
    }
  }
  return std::move(oss.bytes());
}

bool LocalIPCMessage::fromBytes(Buffer &&bytes) noexcept {
  using namespace std;
  auto iss = make_shared<IByteStream>(std::move(bytes));
  Deserializer ds(*iss);
  try {
    ContentType contentType = ContentType::NA;
    ds >> serviceID_ >> operationID_ >> operationCode_ >> requestID_ >>
        sourceAddress_ >> contentType;
    if (contentType == ContentType::Error) {
      setPayload(decodeAsError(ds));
    } else {
      setPayload(make_shared<IncomingPayload>(std::move(iss)));
    }
    return true;
  } catch (const exception &e) {
    MAF_LOGGER_ERROR(
        "Error occurred when trying to translate LocalIPCMessage from bytes: ",
        e.what());
  }

  return false;
}

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
