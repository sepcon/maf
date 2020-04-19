#ifndef MAF_MESSAGING_CLIENT_SERVER_IPC_LOCAL_INCOMINGMSGCONTENT_H
#define MAF_MESSAGING_CLIENT_SERVER_IPC_LOCAL_INCOMINGMSGCONTENT_H

#include <maf/messaging/client-server/CSMessageContentBase.h>
#include <maf/utils/serialization/IByteStream.h>
#include <memory>

namespace maf {
namespace messaging {
namespace ipc {

class IncomingMsgContent : public CSMessageContentBase {
  using IStreamPtr = std::shared_ptr<srz::IByteStream>;
  IStreamPtr stream_;

public:
  IncomingMsgContent(Type type, IStreamPtr stream)
      : CSMessageContentBase(type), stream_{std::move(stream)} {}
  bool equal(const CSMessageContentBase * /*other*/) const override {
    return false;
  }
  CSMessageContentBase *clone() const override { return nullptr; }
  IStreamPtr stream() const { return stream_; }
};

} // namespace ipc
} // namespace messaging
} // namespace maf

#endif // MAF_MESSAGING_CLIENT_SERVER_IPC_LOCAL_INCOMINGMSGCONTENT_H
