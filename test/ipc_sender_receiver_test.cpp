
#include <maf/logging/Logger.h>
#include <maf/threading/AtomicObject.h>

#include <atomic>
#include <thread>
#include <vector>

#include "../src/common/maf/messaging/client-server/ipc/LocalIPCBufferReceiver.h"
#include "../src/common/maf/messaging/client-server/ipc/LocalIPCBufferSender.h"
#include "test.h"

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

void test() {
  Address receiverAddr{"nocpes.github.com", 0};

  auto sender = local::LocalIPCBufferSender{};
  auto receiver = local::LocalIPCBufferReceiver{};

  std::thread receiverThread;
  MAF_TEST_CASE_BEGIN(check_receiver_status) {
    MAF_TEST_EXPECT(receiver.init(receiverAddr))
    receiverThread = std::thread{[&receiver] { receiver.start(); }};
    std::this_thread::sleep_for(10ms);
    MAF_TEST_EXPECT(sender.checkReceiverStatus(receiverAddr) ==
                    Availability::Available)
  }

  MAF_TEST_CASE_END()

  MAF_TEST_CASE_BEGIN(send_buffer) {
    const auto SenderThreadsCount = size_t{40};
    const std::vector<Buffer> buffers = {
        "Hello world Nocpes", "sdfdsferererserer",
        "1111111111111111111111111111111111111",
        "fd2313231322323232332323232323232332323232323232323"};
    const auto BufferCount = buffers.size();

    auto byteComeObserver = WaitableBytesComeObserver{};
    receiver.setObserver(&byteComeObserver);

    auto sendBuffers = [&sender, &buffers, &receiverAddr] {
      for (const auto& buffer : buffers) {
        maf::test::log_rec() << sender.send(buffer, receiverAddr);
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
    MAF_TEST_EXPECT(receivedBuffers->size() ==
                    SenderThreadsCount * BufferCount);
    MAF_TEST_EXPECT(receivedBufferCount == SenderThreadsCount * BufferCount);
  }
  MAF_TEST_CASE_END()

  receiver.stop();
  receiverThread.join();
}
int main() {
  using namespace maf::logging;
  maf::logging::init(LogLevel::LOG_LEVEL_ERROR | LogLevel::LOG_LEVEL_INFO,
                     [](const auto& msg) { std::cout << msg << std::endl; });
  maf::test::init_test_cases();
  test();
}
