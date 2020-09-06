#include <maf/Messaging.h>
#include <maf/messaging/MessageHandler.h>
#include <maf/utils/Pointers.h>

#include <future>
#include <iostream>
#include <map>

using namespace maf;
using namespace messaging;
using namespace std;

static map<string, string> globalConfig;

struct logger__ {
  logger__(const string& prefix) {
    oss_ << prefix << "(" << this_thread::get_id() << "):\t";
  }

  ~logger__() { cout << oss_.str() << endl; }

  template <class T>
  logger__& operator<<(const T& val) {
    oss_ << val;
    return *this;
  }
  ostringstream oss_;
};

static logger__ logger(const string& prefix = "") { return logger__{prefix}; }

struct SetConfigRequest {
  SetConfigRequest(string path, string value) {
    this->path = move(path);
    this->value = move(value);
  }
  SetConfigRequest(const SetConfigRequest& other) {
    path = other.path;
    value = other.value;
  };
  SetConfigRequest(SetConfigRequest&& other) {
    path = move(other.path);
    value = move(other.value);
  }
  SetConfigRequest& operator=(SetConfigRequest&& other) {
    path = move(other.path);
    value = move(other.value);
    return *this;
  }
  SetConfigRequest& operator=(const SetConfigRequest& other) {
    path = other.path;
    value = other.value;
    return *this;
  };

  SetConfigRequest() {}
  ~SetConfigRequest() {}
  string path;
  string value;
};

struct GetConfigRequest {
  string path;
};

static Upcoming<string> fetchConfig(const std::string& path) {
  auto req = routing::makeRequestSync<string, GetConfigRequest>("LGC");
  return req.send({move(path)});
}

static void fetchConfig(const std::string& path,
                        std::function<void(string)> configGetCallback) {
  routing::makeRequestAsync<string, GetConfigRequest>("LGC").send(
      {path}, move(configGetCallback));
}

static bool setConfig(string path, string config) {
  return routing::makeRequestSync<bool, SetConfigRequest>("LGC")
      .send({move(path), move(config)})
      .get()
      .value();
}

#include <maf/utils/TimeMeasurement.h>

struct stop_all_signal {};

void stopall() { routing::postToAll<stop_all_signal>(); }

int main() {
  cout.sync_with_stdio(false);
  logging::init(logging::LOG_LEVEL_FROM_INFO,
                [](const auto& msg) { logger() << msg; });
  util::TimeMeasurement tm(
      [](auto elapsed) { logger() << "Total time = " << elapsed.count(); });

  AsyncComponent logic = Component::create("LGC");
  MessageHandlerGroup msgHandlers(logic.instance());
  msgHandlers
      .connect<routing::ComponentStatusUpdateMsg>(
          [](const routing::ComponentStatusUpdateMsg& s) {
            if (auto comp = s.compref.lock()) {
              if (comp->id() == "gui") {
                logger("LGC") << "GUI thread started!";
              }
            }
          })
      .connect<stop_all_signal>(
          [] { logger("LGC") << "got stop_all_signal, then quit"; });

  auto requestHandlers = RequestHandlerGroup{logic.instance()};
  requestHandlers
      .connect<GetConfigRequest>([](GetConfigRequest req) -> string {
        logger("LGC") << "Received request config of path FIRST" << req.path;
        return globalConfig[req.path];
      })
      .connect<SetConfigRequest>([](const SetConfigRequest& req) {
        logger("LGC") << "Received request get_config_sync_request";
        globalConfig[req.path] = req.value;
        return true;
      });

  logic->connect<stop_all_signal>([] {
    logger("LGC") << "Received stop_all_signal, then quit!";
    this_component::stop();
  });

  logic.launch([] {}, [] { logger("LGC") << "Logic thread exited!"; });

  auto gui = Component::create("GUI");
  gui->connect<stop_all_signal>([](auto) {
    logger("GUI") << "Received stop_all_signal, then quit!";
    this_component::stop();
  });

  gui->connect<routing::ComponentStatusUpdateMsg>(
      [](const routing::ComponentStatusUpdateMsg& status) {
        if (auto r = status.compref.lock(); r && r->id() == "LGC") {
          setConfig("hello/world/sync", "sync");
          setConfig("hello/world/async", "async");

          logger("GUI") << "1. Sync configuration of hello/world got: "
                        << fetchConfig("hello/world/sync").get().value();

          logger("GUI")
              << "2. this line should come after Sync request complete";

          fetchConfig("hello/world/sync", [](string config) {
            logger("GUI") << "4. this is the last message, even its request "
                             "was sent before the (3.): "
                          << config;
            fetchConfig("hello/world/sync")
                .then([](std::string config) {
                  logger("Upcoming")
                      << "Received config throught ComingOutput: " << config;
                  return config.size();
                })
                .then([](size_t sizeofConfig) {
                  logger("Upcoming") << "size of sync config: " << sizeofConfig;
                  return to_string(sizeofConfig);
                })
                .then([](string sSizeOfConfig) {
                  logger("Upcoming")
                      << "size of sync config as string: " << sSizeOfConfig;
                })
                .then([] { logger("Upcoming") << "Finally do nothing"; })
                .wait();
            this_component::post<stop_all_signal>();
          });
          logger("GUI")
              << "3. this line must come right after sending getConfig async "
                 "request and before the async config received!";
        }
      });

  auto task = async(launch::async, [] {
    logger("asc") << "Got async config "
                  << fetchConfig("hello/world/sync").get().value();
  });

  gui->run();

  try {
    logic.stopAndWait();
  } catch (const exception& e) {
    logger("GUI") << "Exception in thread of logic component " << e.what();
  }

  return 0;
}
