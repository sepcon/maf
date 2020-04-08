#pragma once

#include "NamedPipeReceiverBase.h"

namespace maf {
namespace messaging {
namespace ipc {

struct PipeInstance {
  srz::ByteArray ba;
  OVERLAPPED oOverlap;
  HANDLE hPipeInst;
  bool fPendingIO;
};

using BytesComeCallback = std::function<void(const ByteArrayPtr &)>;
class LocalIPCReceiverImpl : public NamedPipeReceiverBase {
public:
  using PipeInstances = std::vector<std::unique_ptr<PipeInstance>>;
  using Handles = std::vector<HANDLE>;
  LocalIPCReceiverImpl();
  bool stopListening();
  void registerObserver(BytesComeCallback);

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
