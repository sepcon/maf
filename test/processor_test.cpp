#include <maf/messaging/Processor.h>
#include <maf/messaging/ProcessorEx.h>
#include <maf/messaging/Routing.h>
#include <maf/utils/TimeMeasurement.h>

#include <cstring>
#include <map>

#define CATCH_CONFIG_MAIN
#include "catch/catch_amalgamated.hpp"

using namespace maf::messaging;
using namespace maf::util;
using namespace std::chrono_literals;

struct program_start_msg {};
struct program_end_msg {};

struct data_convert_req_msg {
  ProcessorInstance requester;
  std::string integer_string;
};

struct data_produced_msg {
  int integer;
};

struct cal_sume_done_msg {};

struct data_sum_msg {
  std::string messageprocessor_name;
  int sum;
};

static thread_local int total = 0;
static int sum_of_all_consumers = 0;
static int range_begin = 1;
static int range_end = 1000;

static void postingMessages() {
  auto producerComp = Processor::create("producer");

  std::vector<ProcessorInstance> consumerProcessors;
  consumerProcessors.push_back(Processor::create("consumer"));
  consumerProcessors.push_back(Processor::create("consumer2"));

  auto mainProcessor = Processor::create("main");

  for (auto consumerComp : consumerProcessors) {
    consumerComp->connect<program_start_msg>([producerComp](auto) {
      producerComp->post(data_convert_req_msg{this_processor::instance(), "0"});
    });

    consumerComp->connect<data_produced_msg>(
        [producerComp, mainProcessor](data_produced_msg msg) {
          static thread_local int currentInteger = range_begin;
          total += msg.integer;
          if (currentInteger <= range_end) {
            producerComp->post(data_convert_req_msg{
                this_processor::instance(), std::to_string(currentInteger++)});
          } else {
            mainProcessor->post(cal_sume_done_msg{});
          }
        });

    consumerComp->connect<program_end_msg>([mainProcessor](auto) {
      auto currentProcessor = this_processor::instance();
      mainProcessor->post(data_sum_msg{currentProcessor->id(), total});
      this_processor::stop();
    });

    consumerComp->post(program_start_msg{});
  }

  producerComp->connect<data_convert_req_msg>([](data_convert_req_msg msg) {
    msg.requester->post(data_produced_msg{std::stoi(msg.integer_string)});
  });

  mainProcessor->connect<cal_sume_done_msg>([&consumerProcessors](auto) {
    static size_t totalConsumerDone = 0;
    if (++totalConsumerDone == consumerProcessors.size()) {
      for (auto consumer : consumerProcessors) {
        consumer->post(program_end_msg{});
      }
    }
  });

  mainProcessor->connect<data_sum_msg>(
      [totalMsgCount = consumerProcessors.size()](data_sum_msg msg) {
        static size_t totalMsgCame = 0;
        sum_of_all_consumers += msg.sum;
        if (++totalMsgCame == totalMsgCount) {
          this_processor::stop();
        }
      });

  auto stopSignals = std::vector<AsyncProcessor::StoppedSignal>{};

  stopSignals.emplace_back(AsyncProcessor::launchProcessor(producerComp));

  for (auto& consumer : consumerProcessors) {
    stopSignals.emplace_back(AsyncProcessor::launchProcessor(consumer));
  }

  mainProcessor->run();
  producerComp->stop();
}

TEST_CASE("tes_with_range_1_1000") {
  auto processor = makeProcessor();
  int sum = 0;
  static const int ExpectedCount = 100;
  processor->connect<int>([&](int i) { sum += i; });
  processor->run([] {
    for (int i = 0; i < ExpectedCount; ++i) {
      this_processor::post(1);
    }
    this_processor::instance()->executeAsync([] { this_processor::stop(); });
  });
  REQUIRE(sum == ExpectedCount);
}

TEST_CASE("register_unregister") {
  auto c = Processor::create();
  static std::map<int, int> num2countMap;

  auto reg1 = c->connect<int>([](int x) { num2countMap[x]++; });
  auto reg2 = c->connect<int>([](int x) { num2countMap[x]++; });
  auto reg3 = c->connect<int>([](int x) { num2countMap[x]++; });

  MsgConnection reg4;
  reg4 = c->connect<int>([&](int x) {
    num2countMap[x]++;
    switch (x) {
      case 1:
        reg1.disconnect();
        this_processor::post(2);
        break;
      case 2:
        reg2.disconnect();
        this_processor::post(3);
        break;
      case 3:
        reg3.disconnect();
        this_processor::post(4);
        break;
      case 4:
        this_processor::stop();
        break;
      default:
        this_processor::stop();
        break;
    }
  });

  c->run([] { this_processor::post(1); });

  REQUIRE(num2countMap[1] == 4);
  REQUIRE(num2countMap[2] == 3);
  REQUIRE(num2countMap[3] == 2);
  REQUIRE(num2countMap[4] == 1);
  REQUIRE(num2countMap[5] == 0);
}

TEST_CASE("sendMessage") {
  struct waitable_msg {};
  struct non_waitable_msg {};
  struct sum_int_msg {
    int index;
  };

  int count = 0;
  int sum = 0;
  bool gotHandlingDoneSignal = false;

  const int totalCount = 10;

  AsyncProcessor logic = Processor::create("logic");
  for (int i = 0; i < totalCount; ++i) {
    logic->connect<waitable_msg>([&count] { ++count; });
  }

  logic->connect<sum_int_msg>(
      [&sum](const sum_int_msg& msg) { sum += msg.index; });

  logic.launch();

  SECTION("messageprocessor_send_message") {
    logic->waitablePost<waitable_msg>()
        .then([&gotHandlingDoneSignal] { gotHandlingDoneSignal = true; })
        .wait();

    REQUIRE(totalCount == count);
    REQUIRE(gotHandlingDoneSignal);
    gotHandlingDoneSignal = false;

    auto handledSignal = logic->waitablePost<non_waitable_msg>().then(
        [&gotHandlingDoneSignal] { gotHandlingDoneSignal = true; });

    REQUIRE(!handledSignal.valid());

    std::vector<Processor::CompleteSignal> handledSignals;
    for (int i = 1; i <= 10; ++i) {
      handledSignals.push_back(logic->waitablePost<sum_int_msg>(i));
    }

    for (auto& sig : handledSignals) {
      sig.waitFor(std::chrono::milliseconds{1});
    }

    REQUIRE(sum == 10 * 11 / 2);
  }

  SECTION("send_message_from_current_messageprocessor") {
    struct NotBlockedMsg {};
    auto msgHandledEventSource = std::make_shared<std::promise<int> >();

    auto msgHandledSignal = msgHandledEventSource->get_future();
    logic->connect<NotBlockedMsg>(
        [event = std::move(msgHandledEventSource)]() mutable {
          event->set_value(100);
        });
    logic->executeAsync(
        [] { this_processor::instance()->waitablePost<NotBlockedMsg>().wait(); });

    REQUIRE(msgHandledSignal.get() == 100);
  }
}

TEST_CASE("blockingExecution") {
  AsyncProcessor comp;
  comp.launch();
  int firedCount = 0;
  comp->waitableExecute([&firedCount] {
        std::this_thread::sleep_for(1ms);
        ++firedCount;
      })
      .then([&firedCount] { ++firedCount; })
      .wait();

  REQUIRE(firedCount == 2);

  comp->stop();
  // reset
  firedCount = 0;
  bool gotException = false;
  try {
    comp->waitableExecute([&firedCount] {
          std::this_thread::sleep_for(1ms);
          ++firedCount;
        })
        .then([&firedCount] { ++firedCount; })
        .wait();
  } catch (const std::future_error&) {
    gotException = true;
  }

  REQUIRE(firedCount == 0);
  REQUIRE(gotException == true);
}
