#include <maf/Messaging.h>
#include <maf/messaging/ShortLivedHandlersManager.h>
#include <maf/utils/Pointers.h>

#include <iostream>
#include <map>

using namespace maf;
using namespace messaging;
using namespace std;

static map<string, string> globalConfig;

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
  routing::routeMessageAndWait<get_config_sync_request>("logic", move(path),
                                                        &config);
  return config;
}

static bool getConfig(std::string path,
                      std::function<void(const std::string& config)> callback) {
  auto asynCallback = [cb = move(callback),
                       executor = this_component::getExecutor()](string conf) {
    executor->execute(bind(cb, move(conf)));
  };

  return routing::routeMessage<get_config_async_request>("logic", move(path),
                                                         move(asynCallback));
}

static bool setConfig(string path, string config) {
  return routing::routeMessage<set_config_request>("logic", move(path),
                                                   move(config));
}

#include <maf/utils/TimeMeasurement.h>
int main() {
  cout.sync_with_stdio(false);
  util::TimeMeasurement tm(
      [](auto elapsed) { cout << "Total time = " << elapsed.count() << endl; });

  AsyncComponent logicComponent = Component::create("logic");
  ShortLivedHandlersManager handlersMgr(logicComponent.instance());
  handlersMgr
      .onMessage<get_config_async_request>(
          [](const get_config_async_request& request) {
            cout << "LOGIC: [" << this_thread::get_id() << "]"
                 << " Received request get_config_async_request" << endl;
            request.callback(globalConfig[request.path]);
          })
      .onMessage<get_config_sync_request>(
          [](const get_config_sync_request& request) {
            cout << "LOGIC: [" << this_thread::get_id() << "]"
                 << " Received request get_config_sync_request" << endl;
            util::assign_ptr(request.got_value, globalConfig[request.path]);
          })
      .onMessage<set_config_request>([](const set_config_request& request) {
        cout << "LOGIC: [" << this_thread::get_id() << "]"
             << " Received request set_config_request" << endl;
        globalConfig[request.path] = request.value;
      });

  logicComponent.run();

  auto guiComponent = Component::create("gui");

  guiComponent->run(
      [] {  // init
        setConfig("hello/world/sync", "sync");
        setConfig("hello/world/async", "async");

        cout << "[" << this_thread::get_id() << "]"
             << "1. Sync configuration of hello/world got: "
             << getConfig("hello/world/sync") << endl;

        cout << "[" << this_thread::get_id() << "]"
             << "2. this line should come after Sync request complete" << endl;

        getConfig("hello/world/async", [](string value) {
          cout << "[" << this_thread::get_id() << "]"
               << "4. Async configuration of hello/world/async got: " << value
               << endl;
          this_component::stop();
        });

        cout << "[" << this_thread::get_id() << "]"
             << "3. this line must come right after sending getConfig async "
                "request and before the async config received!"
             << endl;
      },
      [&logicComponent]() mutable {  // deinit
        logicComponent.stopAndWait();
      });

  return 0;
}
