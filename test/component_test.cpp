#include <maf/messaging/AsyncComponent.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/MessageHandlerManager.h>
#include <maf/utils/TimeMeasurement.h>

#include <map>

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
      for (auto consumer : consumerComponents) {
        consumer->post(program_end_msg{});
      }
    }
  });

  mainComponent->onMessage<data_sum_msg>(
      [totalMsgCount = consumerComponents.size()](data_sum_msg msg) {
        static size_t totalMsgCame = 0;
        sum_of_all_consumers += msg.sum;
        if (++totalMsgCame == totalMsgCount) {
          this_component::stop();
        }
      });

  auto stopSignals = std::vector<AsyncComponent::StoppedSignal>{};

  stopSignals.emplace_back(AsyncComponent::runAsync(producerComp));

  for (auto& consumer : consumerComponents) {
    stopSignals.emplace_back(AsyncComponent::runAsync(consumer));
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
  struct waitable_msg {};
  auto msgHandled = false;

  AsyncComponent logicComponent = Component::create("logic");
  logicComponent.run();

  static constexpr auto WAIT_TIME = 1ms;
  {
    TimeMeasurement tm([](auto elapsedUS) {
      MAF_TEST_CASE_BEGIN(sync_execute) {
        MAF_TEST_EXPECT(elapsedUS > WAIT_TIME);
      }
      MAF_TEST_CASE_END(sync_execute)
    });

    logicComponent.instance()->executeAndWait(
        [] { this_thread::sleep_for(WAIT_TIME); });
  }

  MAF_TEST_CASE_BEGIN(post_sync_message) {
    logicComponent.instance()->onMessage<waitable_msg>(
        [&msgHandled](const auto&) {
          this_thread::sleep_for(1ms);
          msgHandled = true;
          this_component::unregisterAllHandlers<waitable_msg>();
        });

    MAF_TEST_EXPECT(logicComponent.instance()->postAndWait<waitable_msg>());
    MAF_TEST_EXPECT(msgHandled);
  }
  MAF_TEST_CASE_END(post_sync_message)

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

  MAF_TEST_CASE_BEGIN(non_block_posting_wait_from_this_component) {
    auto firedCount = 0;
    logicComponent.instance()->onMessage<waitable_msg>(
        [&firedCount](const auto&) {
          firedCount++;
          if (firedCount < 2) {
            this_component::instance()->postAndWait(waitable_msg{});
            this_component::unregisterAllHandlers<waitable_msg>();
          }
        });

    logicComponent.instance()->postAndWait(waitable_msg{});
    MAF_TEST_EXPECT(firedCount == 2);
  }
  MAF_TEST_CASE_END(non_block_posting_wait_from_this_component)
  MAF_TEST_CASE_BEGIN(stop_async_component) {
    logicComponent.instance()->execute([] {
      std::this_thread::sleep_for(3ms);
      this_component::stop();
    });

    auto success = logicComponent.instance()->executeAndWait(
        [] { maf::test::log_rec() << "This will never show!"; });

    MAF_TEST_EXPECT(!success);

    auto fired = false;
    logicComponent.instance()->onMessage<waitable_msg>([&fired](const auto&) {
      fired = true;
      this_thread::sleep_for(100000h);
    });

    success = logicComponent.instance()->postAndWait<waitable_msg>();

    MAF_TEST_EXPECT(!success);
    MAF_TEST_EXPECT(!fired);

    logicComponent.wait();
    MAF_TEST_EXPECT(!logicComponent.running())
  }
  MAF_TEST_CASE_END(stop_async_component)
}

void testRegisterUnregisterHandlers() {
  auto c = Component::create();

  static std::map<int, int> num2countMap;

  auto reg1 = c->onMessage<int>([](int x) { num2countMap[x]++; });
  auto reg2 = c->onMessage<int>([](int x) { num2countMap[x]++; });
  auto reg3 = c->onMessage<int>([](int x) { num2countMap[x]++; });

  HandlerRegID reg4;
  reg4 = c->onMessage<int>([&](int x) {
    num2countMap[x]++;
    switch (x) {
      case 1:
        this_component::instance()->unregisterHandler(reg1);
        this_component::post(2);
        break;
      case 2:
        this_component::instance()->unregisterHandler(reg2);
        this_component::post(3);
        break;
      case 3:
        this_component::instance()->unregisterHandler(reg3);
        this_component::post(4);
        break;
      case 4:
        this_component::stop();
        break;
      default:
        this_component::stop();
        break;
    }
  });

  c->run([] { this_component::post(1); });

  MAF_TEST_CASE_BEGIN(register_unregister) {
    MAF_TEST_EXPECT(num2countMap[1] == 4);
    MAF_TEST_EXPECT(num2countMap[2] == 3);
    MAF_TEST_EXPECT(num2countMap[3] == 2);
    MAF_TEST_EXPECT(num2countMap[4] == 1);
    MAF_TEST_EXPECT(num2countMap[5] == 0);
  }
  MAF_TEST_CASE_END(register_unregister)
}

void testAutoUnregister() {
  using namespace std;
  auto comp = Component::create();
  auto handlersMgr = new MessageHandlerManager{comp};

  static map<int, int> i2count;
  static map<long, int> ld2count;
  static map<double, int> lf2count;
  static map<string, int> s2count;

  handlersMgr->onMessage<int>([](int x) { i2count[x]++; })
      .onMessage<int>([](int x) { i2count[x]++; })
      .onMessage<string>([](const string& s) { s2count[s]++; })
      .onMessage<string>([](const string& s) { s2count[s]++; })
      .onMessage<double>([](const double d) { lf2count[d]++; })
      .onMessage<long>([](const long l) { ld2count[l]++; })
      .onMessage<long>([](const long l) { ld2count[l]++; });

  comp->onMessage<string>([handlersMgr](const string& s) {
    if (s == "unreg_s") {
      handlersMgr->unregisterHandler<string>();
    } else if (s == "unreg_i") {
      handlersMgr->unregisterHandler<int>();
    } else if (s == "unreg_all") {
      delete handlersMgr;  // delete to unreg all
    }
  });

  comp->onMessage<string>([](const string& s) {
    if (s == "quit") {
      this_component::stop();
    }
  });

  comp->run([] {
    this_component::post(1);
    this_component::post(2);
    this_component::post(2.0);
    this_component::post(string{"hello"});
    this_component::post(string{"world"});
    this_component::post(-1);

    this_component::post(long(1));
    this_component::post(long(2));

    this_component::post(string{"unreg_i"});
    this_component::post(100);
    this_component::post(long(3));

    this_component::post(string{"unreg_s"});
    this_component::post<string>("after_unreg");

    this_component::post(string{"unreg_all"});
    this_component::post(100);
    this_component::post(long(100));
    this_component::post(100.00);
    this_component::post("last_string");

    this_component::post(string{"quit"});
  });

  MAF_TEST_CASE_BEGIN(auto_unreg) {
    MAF_TEST_EXPECT(i2count[1] == 2);
    MAF_TEST_EXPECT(i2count[2] == 2);
    MAF_TEST_EXPECT(ld2count[1] == 2);
    MAF_TEST_EXPECT(ld2count[2] == 2);
    MAF_TEST_EXPECT(ld2count[3] == 2);
    MAF_TEST_EXPECT(ld2count[100] == 0);
    MAF_TEST_EXPECT(lf2count[2.0] == 1);
    MAF_TEST_EXPECT(i2count[100] == 0);
    MAF_TEST_EXPECT(lf2count[100.0] == 0);
    MAF_TEST_EXPECT(s2count["hello"] == 2);
    MAF_TEST_EXPECT(s2count["world"] == 2);
    MAF_TEST_EXPECT(s2count["after_unreg"] == 0);
    MAF_TEST_EXPECT(s2count["last_string"] == 0);
  }
  MAF_TEST_CASE_END(auto_unreg)
}
int main() {
  maf::test::init_test_cases();
  testSyncExecution();
  testPostingMessages();
  testRegisterUnregisterHandlers();
  testAutoUnregister();

  return 0;
}
