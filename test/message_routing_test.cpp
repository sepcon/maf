#include <maf/messaging/ProcessorEx.h>
#include <maf/messaging/Routing.h>
#include <maf/utils/StringifyableEnum.h>

#include <atomic>
#include <iostream>

#define CATCH_CONFIG_MAIN
#include "catch/catch_amalgamated.hpp"

using namespace maf::messaging;
using namespace maf::messaging::routing;

using namespace std::chrono_literals;

static constexpr auto MasterID = "id.master";
static constexpr auto SlaveID = "id.slave.";

static const std::string MasterGreetString = "Master Hello";

struct HasSenderMessage {
  HasSenderMessage(ProcessorInstance sender = {}) : sender{std::move(sender)} {
    if (!this->sender) {
      this->sender = this_processor::instance();
    }
  }
  ProcessorInstance sender;
};

struct TaskInputRequest : public HasSenderMessage {};
struct TaskSubmitted {};
using Text = std::string;

static auto master = ProcessorInstance{};
static auto slaves = std::vector<AsyncProcessor>{};

static std::atomic_uint greetCount = 0;
static std::atomic_uint taskSubmittedCount = 0;

// clang-format off
MC_MAF_STRINGIFYABLE_ENUM(BinOp, char,
                          Add,
                          Sub,
                          Mul,
                          Dev
                          )
// clang-format on

using Task = std::tuple<BinOp, int, int>;
using TaskResult = std::tuple<ProcessorInstance, BinOp, int>;

static constexpr Task TasksForSlaves[] = {{BinOp::Add, 1, 222},
                                          {BinOp::Sub, 111, 2},
                                          {BinOp::Mul, 2, 2},
                                          {BinOp::Dev, -610, 1}};
static constexpr auto TaskCount = sizeof(TasksForSlaves) / sizeof(Task);
static int sumAll = 0;

static constexpr int eval(const Task &task) {
  auto [op, first, second] = task;
  auto output = 0;
  switch (op) {
    case BinOp::Add:
      output = first + second;
      break;
    case BinOp::Sub:
      output = first - second;
      break;
    case BinOp::Mul:
      output = first * second;
      break;
    case BinOp::Dev:
      output = first / second;
      break;
    default:
      break;
  }
  return output;
}

struct CompLogRecord {
  CompLogRecord() = default;
  CompLogRecord(CompLogRecord &&) = default;
  CompLogRecord(const CompLogRecord &) = delete;
  ~CompLogRecord() { std::cout << oss.str() << std::endl; }
  template <typename T>
  CompLogRecord &operator<<(const T &val) {
    oss << val;
    return *this;
  }
  std::ostringstream oss;
};

CompLogRecord log() { return CompLogRecord(); }

static void setupSlaves() {
  for (unsigned i = 0; i < TaskCount; ++i) {
    auto slave = Processor::create(SlaveID + std::to_string(i));
    slaves.emplace_back(slave);
    slave->connect<std::string>([](const auto &msg) {
      ++greetCount;
      log() << "Slave " << this_processor::instance()->id()
            << " received string msg from master: " << msg;
      post(MasterID, TaskInputRequest{});
    });

    slave->connect<Task>([](const Task &task) {
      post(MasterID, TaskResult{this_processor::instance(), std::get<0>(task),
                                eval(task)});
    });
    slave->connect<TaskSubmitted>([](auto) {
      log() << "Task submitted to master then quit!";
      taskSubmittedCount++;
      this_processor::stop();
    });
  }
}

static void setupMaster() {
  master = Processor::create(MasterID);
  master->connect<ProcessorStatusUpdateMsg>(
      [](const ProcessorStatusUpdateMsg &msg) {
        static int availableSlaves = 0;
        if (msg.ready()) {
          if (++availableSlaves == TaskCount) {
            postToAll(std::string{"Master hellos all slaves!"});
          }
        } else {
          if (--availableSlaves == 0) {
            log() << "Sum all value = " << sumAll;
            this_processor::stop();
          }
        }
      });
  master->connect<TaskInputRequest>([](TaskInputRequest req) {
    static int i = 0;
    req.sender->post(
        TasksForSlaves[i % (sizeof(TasksForSlaves) / sizeof(Task))]);
    ++i;
  });
  master->connect<TaskResult>([](const TaskResult &result) {
    auto &[slave, op, output] = result;
    log() << "Slave " << slave->id() << " done operation " << op
          << " with output = " << output;
    sumAll += output;
    slave->post<TaskSubmitted>();
  });
}

TEST_CASE("routing") {
  // clang-format off
  SECTION("message_routing")
  {
    setupMaster();
    setupSlaves();

    constexpr auto sumAllTasks = [] {
      int sum = 0;
      for (const auto &task : TasksForSlaves) {
          sum += eval(task);
      }
      return sum;
    };

    for (auto &slave : slaves) {
      slave.launch();
    }

    master->run();
    REQUIRE(sumAllTasks() == sumAll);
    REQUIRE(greetCount == TaskCount);
    REQUIRE(taskSubmittedCount == TaskCount);
  }

  // clang-format on

  for (auto &slave : slaves) {
    slave.wait();
  }
}

TEST_CASE("receiverStatus") {
  using namespace std;
  vector<AsyncProcessor> asyncProcessors;
  static const int asyncCount = 10;

  auto mainProcessor = Processor::create("main");
  int totalAvailable = 0;
  int totalUnavailable = 0;

  mainProcessor->connect<routing::ProcessorStatusUpdateMsg>(
      [&totalAvailable,
       &totalUnavailable](const routing::ProcessorStatusUpdateMsg &msg) {
        if (msg.ready()) {
          ++totalAvailable;
          msg.compref.lock()->stop();
        } else {
          ++totalUnavailable;
          if (totalUnavailable == asyncCount) {
            this_processor::stop();
          }
        }
      });

  for (int i = 0; i < asyncCount; ++i) {
    asyncProcessors.emplace_back(Processor::create("async" + to_string(i)));
  }

  for (auto &comp : asyncProcessors) {
    comp.launch();
  }

  mainProcessor->run();

  REQUIRE(totalAvailable == asyncCount);
  REQUIRE(totalUnavailable == asyncCount);

  for (auto &comp : asyncProcessors) {
    comp.stopAndWait();
  }
}

TEST_CASE("sendMessage") {
  struct waitable_msg {};
  int count = 0;
  const int totalCount = 10;
  AsyncProcessor logic = Processor::create("logic");
  for (int i = 0; i < totalCount; ++i) {
    logic->connect<waitable_msg>([&count] { ++count; });
  }
  logic.launch();

  while (!routing::findProcessor("logic")) {
  };

  SECTION("routing_send_message") {
    routing::sendToAll<waitable_msg>()
        .then([] { log() << "msg is all handled"; })
        .wait();
    REQUIRE(totalCount == count);
  }

  logic->disconnect<waitable_msg>();

  count = 0;
  SECTION("routing_send_message") {
    auto handledSignal = routing::sendToAll<waitable_msg>().then(
        [] { log() << "msg is all handled"; });
    REQUIRE(!handledSignal.valid());
    REQUIRE(0 == count);
  }

  logic.stopAndWait();
}
