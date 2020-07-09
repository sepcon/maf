#include "test.h"
#include <maf/messaging/Component.h>

using namespace maf::messaging;

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

static void test() {

  auto producerComp = Component::create("producer");

  std::vector<ComponentInstance> consumerComponents;
  consumerComponents.push_back(Component::create("consumer"));
  consumerComponents.push_back(Component::create("consumer2"));

  auto mainComponent = Component::create("main");

  for (auto consumerComp : consumerComponents) {

    consumerComp->onMessage<program_start_msg>([producerComp](auto) {
      std::cout << "Program started, sending request to producers" << std::endl;
      producerComp->post(
          data_convert_req_msg{this_component::instance(), "0"});
    });

    consumerComp->onMessage<data_produced_msg>([producerComp, mainComponent](
                                                   data_produced_msg msg) {
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

  auto waiters = std::vector<std::future<void>>{};

  waiters.emplace_back(producerComp->runAsync());

  for (auto &consumer : consumerComponents) {
    waiters.emplace_back(consumer->runAsync());
  }

  mainComponent->run();
  producerComp->stop();
}

int main() {
  maf::test::init_test_cases();

  //  MAF_TEST_CASE_BEGIN(tes_with_range_1_1000) {
  //    test();
  //    MAF_TEST_EXPECT(sum_of_all_consumers == (range_end*(range_end + 1)))
  //  }
  //  MAF_TEST_CASE_END()

  MAF_TEST_CASE_BEGIN(tes_with_range_1_100000) {
    range_begin = 1;
    range_end = 100000;
    test();
    MAF_TEST_EXPECT(sum_of_all_consumers == (range_end * (range_end + 1)))
  }
  MAF_TEST_CASE_END()
}
