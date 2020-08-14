#include <maf/messaging/AsyncComponent.h>
#include <maf/messaging/Routing.h>
#include <maf/utils/StringifyableEnum.h>

#include <atomic>

#include "test.h"

using namespace maf::messaging;
using namespace maf::messaging::routing;

using namespace std::chrono_literals;

static constexpr auto MasterID = "id.master";
static constexpr auto SlaveID = "id.slave.";

static const std::string MasterGreetString = "Master Hello";

struct HasSenderMessage {
  HasSenderMessage(ReceiverInstance sender = {}) : sender{std::move(sender)} {
    if (!this->sender) {
      this->sender = this_component::instance();
    }
  }
  ReceiverInstance sender;
};

struct TaskInputRequest : public HasSenderMessage {};
struct TaskSubmitted {};
using Text = std::string;

static auto master = ComponentInstance{};
static auto slaves = std::vector<AsyncComponent>{};

static std::atomic_int greetCount = 0;
static std::atomic_int taskSubmittedCount = 0;

// clang-format off
MC_MAF_STRINGIFYABLE_ENUM(BinOp, char,
                          Add,
                          Sub,
                          Mul,
                          Dev
                          );
// clang-format on

using Task = std::tuple<BinOp, int, int>;
using TaskResult = std::tuple<ComponentInstance, BinOp, int>;

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
  };
  std::ostringstream oss;
};

CompLogRecord log() { return CompLogRecord(); }

static void setupSlaves() {
  for (unsigned i = 0; i < TaskCount; ++i) {
    auto slave = Component::create(SlaveID + std::to_string(i));
    slaves.emplace_back(slave);
    slave->onMessage<std::string>([](const auto &msg) {
      ++greetCount;
      log() << "Slave " << this_component::instance()->id()
            << " received string msg from master: " << msg;
      routeMessage(MasterID, TaskInputRequest{});
    });

    slave->onMessage<Task>([](const Task &task) {
      routeMessage(MasterID, TaskResult{this_component::instance(),
                                        std::get<0>(task), eval(task)});
    });
    slave->onMessage<TaskSubmitted>([](auto) {
      log() << "Task submitted to master then quit!";
      taskSubmittedCount++;
      this_component::stop();
    });
  }
}

static void setupMaster() {
  master = Component::create(MasterID);
  master->onMessage<receiver_status_update>(
      [](const receiver_status_update &msg) {
        static int availableSlaves = 0;
        if (msg.is_available()) {
          if (++availableSlaves == TaskCount) {
            broadcast(std::string{"Master hellos all slaves!"});
          }
        } else {
          if (--availableSlaves == 0) {
            log() << "Sum all value = " << sumAll;
            this_component::stop();
          }
        }
      });
  master->onMessage<TaskInputRequest>([](TaskInputRequest req) {
    static int i = 0;
    req.sender->post(
        TasksForSlaves[i % (sizeof(TasksForSlaves) / sizeof(Task))]);
    ++i;
  });
  master->onMessage<TaskResult>([](const TaskResult &result) {
    auto &[slave, op, output] = result;
    log() << "Slave " << slave->id() << " done operation " << op
          << " with output = " << output;
    sumAll += output;
    slave->post<TaskSubmitted>();
  });
}

static void routingTest() {
  // clang-format off
  MAF_TEST_CASE_BEGIN(message_routing)
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
      slave.run();
    }

    master->run();
    MAF_TEST_EXPECT(sumAllTasks() == sumAll)
    MAF_TEST_EXPECT(greetCount == TaskCount)
    MAF_TEST_EXPECT(taskSubmittedCount == TaskCount)
  }
  MAF_TEST_CASE_END()
  // clang-format on

  for (auto &slave : slaves) {
    slave.wait();
  }
}

static void receiverStatusTest() {
  using namespace std;
  vector<AsyncComponent> asyncComponents;
  static const int asyncCount = 10;
  for (int i = 0; i < asyncCount; ++i) {
    asyncComponents.emplace_back(Component::create("async" + to_string(i)));
  }

  for (auto &comp : asyncComponents) {
    comp.run();
  }

  auto mainComponent = Component::create("main");
  int totalAvailable = 0;
  int totalUnavailable = 0;

  mainComponent->onMessage<routing::receiver_status_update>(
      [&totalAvailable,
       &totalUnavailable](const routing::receiver_status_update &msg) {
        if (msg.is_available()) {
          ++totalAvailable;
          msg.receiver.lock()->stop();
        } else {
          ++totalUnavailable;
          if (totalUnavailable == asyncCount) {
            this_component::stop();
          }
        }
      });

  mainComponent->run();

  MAF_TEST_CASE_BEGIN(receiver_status) {
    MAF_TEST_EXPECT(totalAvailable == asyncCount);
    MAF_TEST_EXPECT(totalUnavailable == asyncCount);
  }

  MAF_TEST_CASE_END(receiver_status)
  for (auto &comp : asyncComponents) {
    comp.stopAndWait();
  }
}

int main() {
  maf::test::init_test_cases();
  routingTest();
  receiverStatusTest();
}
