#include <maf/Messaging.h>
#include <maf/Observable.h>
#include <maf/messaging/SignalTimer.h>
#include <maf/utils/TimeMeasurement.h>
#include <maf/utils/serialization/DumpHelper.h>

#include <iostream>

using namespace maf::util;
using namespace maf::signal_slots;
using namespace maf::messaging;
using namespace std;

static mutex m;

#define LOG(expr)         \
  {                       \
    lock_guard lock(m);   \
    cout << expr << endl; \
  }

static Observable<vector<string>, size_t> locations;
static Observable<bool> stopped = false;

void startLocationsFetcher() {
  thread{[] {
    do {
      cout << "start fetching data from server..." << endl;
      this_thread::sleep_for(49ms);
      auto mutableLocations = locations.mut();
      auto& locationList = mutableLocations.get<0>();
      locationList.push_back(
          "new place: " +
          to_string(chrono::system_clock::now().time_since_epoch().count()));
      mutableLocations.get<1>() = locationList.size();
    } while (!stopped.get());
  }}.detach();
}

int main() {
  startLocationsFetcher();
  auto mainComponent = Component::create();
  locations.connect(
      [](const vector<string>& loc, size_t modificationPos) {
        cout << "Got new list of locations: modificationPos = "
             << modificationPos << " \n"
             << maf::srz::dump(loc) << endl;
      },
      mainComponent->getExecutor());

  mainComponent->run([] {
    stopped.connect(
        [](bool yes) {
          if (yes) {
            this_component::stop();
          }
        },
        this_component::getBlockingExecutor());
    Timer::timeoutAfter(100ms, [] { stopped = true; });
  });
  return 0;
}
