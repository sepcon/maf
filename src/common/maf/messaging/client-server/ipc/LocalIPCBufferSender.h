#pragma once

#include "BufferSenderIF.h"

namespace maf {
namespace messaging {
namespace ipc {
namespace local {

class NamedPipeSender;

class LocalIPCBufferSender : public maf::messaging::ipc::BufferSenderIF {
 public:
  LocalIPCBufferSender();
  ~LocalIPCBufferSender() override;
  ActionCallStatus send(const maf::srz::Buffer &ba,
                        const Address &destination) override;
  Availability checkReceiverStatus(const Address &destination) const override;

 private:
  std::unique_ptr<class LocalIPCBufferSenderImpl> _pImpl;
};

}  // namespace local
}  // namespace ipc
}  // namespace messaging
}  // namespace maf
