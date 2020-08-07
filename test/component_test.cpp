#include <maf/messaging/AsyncComponent.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/Timer.h>
#include <maf/utils/TimeMeasurement.h>

#include "test.h"

using namespace maf::messaging;
using namespace maf::util;

struct program_start_msg {};
struct program_end_msg {};

struct data_convert_req_msg {
  ComponentInstance requester;
  std::string integer_string;
};

struct data_produced_msg {
  int integer;
};

struct cal_sume_done_msg {};

struct data_sum_msg {
  std::string component_name;
  int sum;
};

static thread_local int total = 0;
static int sum_of_all_consumers = 0;
static int range_begin = 1;
static int range_end = 1000;

static void postingMessages() {
  auto producerComp = Component::create("producer");

  std::vector<ComponentInstance> consumerComponents;
  consumerComponents.push_back(Component::create("consumer"));
  consumerComponents.push_back(Component::create("consumer2"));

  auto mainComponent = Component::create("main");

  for (auto consumerComp : consumerComponents) {
    consumerComp->onMessage<program_start_msg>([producerComp](auto) {
      std::cout << "Program started, sending request to producers" << std::endl;
      producerComp->post(data_convert_req_msg{this_component::instance(), "0"});
    });

    consumerComp->onMessage<data_produced_msg>(
        [producerComp, mainComponent](data_produced_msg msg) {
          static thread_local int currentInteger = range_begin;
          total += msg.integer;
          if (currentInteger <= range_end) {
            producerComp->post(data_convert_req_msg{
                this_component::instance(), std::to_string(currentInteger++)});
          } else {
            mainComponent->post(cal_sume_done_msg{});
          }
        });

    consumerComp->onMessage<program_end_msg>([mainComponent](auto) {
      auto currentComponent = this_component::instance();
      mainComponent->post(data_sum_msg{currentComponent->id(), total});

      std::cout << "component " << currentComponent->id()
                << " received program_end_msg then stop!" << std::endl;
      this_component::stop();
    });

    consumerComp->post(program_start_msg{});
  }

  producerComp->onMessage<data_convert_req_msg>([](data_convert_req_msg msg) {
    msg.requester->post(data_produced_msg{std::stoi(msg.integer_string)});
  });

  mainComponent->onMessage<cal_sume_done_msg>([&consumerComponents](auto) {
    static size_t totalConsumerDone = 0;
    if (++totalConsumerDone == consumerComponents.size()) {
      std::cout << "all consumers done, then notify program end msg and "
                   "collect the result"
                << std::endl;
      for (auto consumer : consumerComponents) {
        consumer->post(program_end_msg{});
      }
    }
  });

  mainComponent->onMessage<data_sum_msg>(
      [totalMsgCount = consumerComponents.size()](data_sum_msg msg) {
        static size_t totalMsgCame = 0;
        std::cout << "sum from component " << msg.component_name << " = "
                  << msg.sum << std::endl;
        sum_of_all_consumers += msg.sum;
        if (++totalMsgCame == totalMsgCount) {
          std::cout << "Received enough msg, then stop the compoent"
                    << std::endl;
          this_component::stop();
        }
      });

  auto stopSignals = std::vector<AsyncComponent::StoppedSignal>{};

  stopSignals.emplace_back(AsyncComponent::run(producerComp));

  for (auto &consumer : consumerComponents) {
    stopSignals.emplace_back(AsyncComponent::run(consumer));
  }

  mainComponent->run();
  producerComp->stop();
}

void testPostingMessages() {
  MAF_TEST_CASE_BEGIN(tes_with_range_1_1000) {
    range_begin = 1;
    range_end = 1000;
    postingMessages();
    MAF_TEST_EXPECT(sum_of_all_consumers == (range_end * (range_end + 1)))
  }
  MAF_TEST_CASE_END()
}

void testSyncExecution() {
  using namespace std;
  using namespace std::chrono_literals;

  AsyncComponent logicComponent = Component::create("logic");
  logicComponent.run();

  static constexpr auto WAIT_TIME = 1ms;
  {
    TimeMeasurement tm([](auto elapsedUS) {
      MAF_TEST_CASE_BEGIN(sync_execute) {
        maf::test::log_rec()
            << "Total time to response = " << elapsedUS.count();
        MAF_TEST_EXPECT(elapsedUS > WAIT_TIME);
      }
      MAF_TEST_CASE_END(sync_execute)
    });

    logicComponent.instance()->executeAndWait(
        [] { this_thread::sleep_for(WAIT_TIME); });
  }

  MAF_TEST_CASE_BEGIN(sync_execute_with_exception) {
    bool caughtException = false;
    try {
      logicComponent.instance()->executeAndWait([] { throw 1; });
    } catch (int) {
      caughtException = true;
    }

    MAF_TEST_EXPECT(caughtException);
  }
  MAF_TEST_CASE_END(sync_execute_with_exception)


  MAF_TEST_CASE_BEGIN(stop_async_component)
  {
      logicComponent.instance()->execute([]{
          std::this_thread::sleep_for(3ms);
          this_component::stop();
      });


    auto success = logicComponent.instance()->executeAndWait(
        [] { maf::test::log_rec() << "This will never show!"; });

    MAF_TEST_EXPECT(!success);

    logicComponent.wait();
    MAF_TEST_EXPECT(!logicComponent.running())
  }
  MAF_TEST_CASE_END(stop_async_component)

}

int main() {
  maf::test::init_test_cases();
  testSyncExecution();
  testPostingMessages();
}
