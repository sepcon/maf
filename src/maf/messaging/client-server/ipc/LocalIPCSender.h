#pragma once

#include "IPCSender.h"

namespace maf {
namespace messaging {
namespace ipc {

class NamedPipeSender;

class LocalIPCSender : public IPCSender {
public:
  LocalIPCSender();
  ~LocalIPCSender() override;
  bool initConnection(const Address &addr) override;
  ActionCallStatus send(const maf::srz::ByteArray &ba,
                        const Address &destination = {}) override;
  const Address &receiverAddress() const override;
  Availability checkReceiverStatus() const override;

private:
  std::unique_ptr<class LocalIPCSenderImpl> _pImpl;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
