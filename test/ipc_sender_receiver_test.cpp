
#include <maf/logging/Logger.h>
#include <maf/threading/AtomicObject.h>

#include <atomic>
#include <thread>
#include <vector>

#include "../src/common/maf/messaging/client-server/ipc/LocalIPCBufferReceiver.h"
#include "../src/common/maf/messaging/client-server/ipc/LocalIPCBufferSender.h"

#define CATCH_CONFIG_MAIN
#include "catch/catch_amalgamated.hpp"

using namespace maf::messaging;
using namespace maf::messaging::ipc;
using namespace maf::srz;
using namespace maf::threading;
using namespace std::chrono_literals;

std::atomic_size_t receivedBufferCount = 0;
static AtomicObject<std::vector<Buffer>> receivedBuffers;

struct WaitableBytesComeObserver : public BytesComeObserver {
  void onBytesCome(Buffer&& buff) override {
    receivedBuffers->push_back(std::move(buff));
    ++receivedBufferCount;
  }
};

TEST_CASE("Sender receiver test for library internal") {
  Address receiverAddr{"nocpes.github.com", 0};

  auto sender = local::LocalIPCBufferSender{};
  auto receiver = local::LocalIPCBufferReceiver{};

  std::thread receiverThread;

  REQUIRE(receiver.init(receiverAddr));
  receiverThread = std::thread{[&receiver] { receiver.start(); }};
  std::this_thread::sleep_for(10ms);
  REQUIRE(sender.checkReceiverStatus(receiverAddr) == Availability::Available);

  const auto SenderThreadsCount = size_t{40};
  const std::vector<Buffer> buffers = {
      "Hello world Nocpes", "sdfdsferererserer",
      "1111111111111111111111111111111111111",
      "fd2313231322323232332323232323232332323232323232323"};
  const auto BufferCount = buffers.size();

  auto byteComeObserver = WaitableBytesComeObserver{};
  receiver.setObserver(&byteComeObserver);

  auto sendBuffers = [&] {
    for (const auto& buffer : buffers) {
      sender.send(buffer, receiverAddr);
    }
  };
  std::vector<std::thread> senderthreads;
  for (size_t i = 0; i < SenderThreadsCount; ++i) {
    senderthreads.emplace_back(sendBuffers);
  }
  for (auto& th : senderthreads) {
    th.join();
  }

  std::this_thread::sleep_for(10ms);
  // Due to insert same buffers to std::set, then it should contain only one
  // buffer
  REQUIRE(receivedBuffers->size() == SenderThreadsCount * BufferCount);
  REQUIRE(receivedBufferCount == SenderThreadsCount * BufferCount);

  receiver.stop();
  receiverThread.join();
}
