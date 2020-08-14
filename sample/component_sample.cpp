#include <maf/Messaging.h>
#include <maf/messaging/MessageHandlerManager.h>
#include <maf/utils/Pointers.h>

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

struct set_config_request {
  set_config_request(string path, string value) {
    this->path = move(path);
    this->value = move(value);
  }
  set_config_request(const set_config_request& other) {
    path = other.path;
    value = other.value;
  };
  set_config_request(set_config_request&& other) {
    path = move(other.path);
    value = move(other.value);
  }
  set_config_request& operator=(set_config_request&& other) {
    path = move(other.path);
    value = move(other.value);
    return *this;
  }
  set_config_request& operator=(const set_config_request& other) {
    path = other.path;
    value = other.value;
    return *this;
  };

  set_config_request() {}
  ~set_config_request() {}
  string path;
  string value;
};

struct get_config_async_request {
  string path;
  std::function<void(std::string)> callback;
};

struct get_config_sync_request {
  string path;
  string* got_value;
};

static string getConfig(std::string path) {
  string config;
  routing::routeMessageAndWait<get_config_sync_request>("LGC", move(path),
                                                        &config);
  return config;
}

static bool getConfig(std::string path,
                      std::function<void(const std::string& config)> callback) {
  logger("INF") << "Sending request geget_config_async_request for path: "
                << path;
  auto asynCallback = [cb = move(callback),
                       compref = this_component::ref()](string conf) {
    if (auto comp = compref.lock()) {
      comp->execute(bind(cb, move(conf)));
    } else {
      logger("ERR") << "The component has been stopped!";
    }
  };

  if (!routing::routeMessage<get_config_async_request>("LGC", move(path),
                                                       asynCallback)) {
    asynCallback("");
    return false;
  }
  return true;
}

static bool setConfig(string path, string config) {
  return routing::routeMessage<set_config_request>("LGC", move(path),
                                                   move(config));
}

#include <maf/utils/TimeMeasurement.h>

struct stop_all_signal {};

void stopall() { routing::broadcast<stop_all_signal>(); }

int main() {
  cout.sync_with_stdio(false);
  logging::init(logging::LOG_LEVEL_FROM_INFO,
                [](const auto& msg) { logger() << msg; });
  util::TimeMeasurement tm(
      [](auto elapsed) { logger() << "Total time = " << elapsed.count(); });

  AsyncComponent logicComponent = Component::create("LGC");
  MessageHandlerManager handlersMgr(logicComponent.instance());
  handlersMgr
      .onMessage<routing::receiver_status_update>(
          [](const routing::receiver_status_update& s) {
            if (auto comp = s.receiver.lock()) {
              if (comp->id() == "gui") {
                logger("LGC") << "GUI thread started!";
              }
            }
          })
      .onMessage<get_config_async_request>(
          [](const get_config_async_request& request) {
            logger("LGC") << "Received request get_config_async_request";
            request.callback(globalConfig[request.path]);
          })
      .onMessage<get_config_sync_request>(
          [](const get_config_sync_request& request) {
            logger("LGC") << "Received request get_config_sync_request";
            util::assign_ptr(request.got_value, globalConfig[request.path]);
          })
      .onMessage<set_config_request>([](const set_config_request& request) {
        logger("LGC") << "Received request set_config_request";
        globalConfig[request.path] = request.value;
      })
      .onMessage<stop_all_signal>(
          [](auto) { logger("LGC") << "got stop_all_signal, then quit"; });

  auto guiComponent = Component::create("GUI");

  logicComponent.run({}, [] {
    logger("LGC") << "Logic thread exited!";
    this_component::stop();
    stopall();
  });

  guiComponent->onMessage<stop_all_signal>(
      [](auto) { this_component::stop(); });
  guiComponent->onMessage<routing::receiver_status_update>(
      [](const routing::receiver_status_update& status) {
        if (auto r = status.receiver.lock(); r && r->id() == "LGC") {
          setConfig("hello/world/sync", "sync");
          setConfig("hello/world/async", "async");

          logger("GUI") << "1. Sync configuration of hello/world got: "
                        << getConfig("hello/world/sync");

          logger("GUI")
              << "2. this line should come after Sync request complete";

          getConfig("hello/world/async", [](string value) {
            logger("GUI") << "4. Async configuration of hello/world/async got: "
                          << value;
            this_component::stop();
          });

          logger("GUI")

              << "3. this line must come right after sending getConfig async "
                 "request and before the async config received!";
        }
      });

  auto task = async(launch::async, [] {
    auto config = getConfig("hello/world/sync");
    logger("asc") << "Got async config " << config;
  });

  guiComponent->run({},                            // init nothing
                    [&logicComponent]() mutable {  // deinit
                      logger("GUI") << "stop and wait event exception";
                      try {
                        logicComponent.stopAndWait();
                      } catch (const exception& e) {
                        logger("GUI")
                            << "Exception in thread of logic component "
                            << e.what();
                      }
                    });

  return 0;
}
