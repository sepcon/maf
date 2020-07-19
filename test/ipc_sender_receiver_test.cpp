
#include <maf/logging/Logger.h>

#include <future>
#include <thread>

#include "../src/common/maf/messaging/client-server/ipc/LocalIPCBufferReceiver.h"
#include "../src/common/maf/messaging/client-server/ipc/LocalIPCBufferSender.h"
#include "test.h"

using namespace maf::messaging;
using namespace maf::messaging::ipc;
using namespace maf::srz;
using namespace std::chrono_literals;

struct WaitableBytesComeObserver : public BytesComeObserver {
  void onBytesCome(Buffer&& bytes) override {
    bytesSetter.set_value(std::move(bytes));
  }

  std::promise<Buffer> bytesSetter;
  std::future<Buffer> bytes = bytesSetter.get_future();
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
    Buffer buffers[] = {"Hello world Nocpes", "sdfdsferererserer",
                        "1111111111111111111111111111111111111",
                        "fd2313231322323232332323232323232332323232323232323"};

    for (const auto& buffer : buffers) {
      auto byteComeObserver = WaitableBytesComeObserver{};
      receiver.setObserver(&byteComeObserver);
      sender.send(buffer, receiverAddr);
      MAF_TEST_EXPECT(byteComeObserver.bytes.wait_for(2ms) ==
                      std::future_status::ready)
      MAF_TEST_EXPECT(byteComeObserver.bytes.get() == buffer)
    }
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
