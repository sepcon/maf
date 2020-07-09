#pragma once

#include "IPCSenderIF.h"

namespace maf {
namespace messaging {
namespace ipc {

class NamedPipeSender;

class LocalIPCSender : public IPCSenderIF {
public:
  LocalIPCSender();
  ~LocalIPCSender() override;
  ActionCallStatus send(const maf::srz::ByteArray &ba,
                        const Address &destination) override;
  Availability checkReceiverStatus(const Address &destination) const override;

private:
  std::unique_ptr<class LocalIPCSenderImpl> _pImpl;
};

} // namespace ipc
} // namespace messaging
} // namespace maf
