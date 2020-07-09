#pragma once

#include "NamedPipeReceiverBase.h"

namespace maf {
namespace messaging {
namespace ipc {

struct PipeInstance {
  srz::ByteArray ba;
  OVERLAPPED oOverlap;
  HANDLE hPipeInst;
  bool fPendingIO = false;
};

using BytesComeCallback = std::function<void(srz::ByteArray &&)>;

class LocalIPCReceiverImpl : public NamedPipeReceiverBase {
public:
  using Base = NamedPipeReceiverBase;
  using PipeInstances = std::vector<std::unique_ptr<PipeInstance>>;
  using Handles = std::vector<HANDLE>;

  LocalIPCReceiverImpl();
  bool stop();
  void setObserver(BytesComeCallback &&);
  bool init(const Address &address);

private:
  bool initPipes();
  void listningThreadFunction();
  void disconnectAndReconnect(size_t index);
  bool readOnPipe(size_t index);

  BytesComeCallback _bytesComeCallback;
  PipeInstances _pipeInstances;
  Handles _hEvents;
};
} // namespace ipc
} // namespace messaging
} // namespace maf
