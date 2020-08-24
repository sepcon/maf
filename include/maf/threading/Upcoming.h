#pragma once

#include <future>
#include <optional>

namespace maf {
namespace threading {
namespace details {

using namespace std;
template <class Resource>
class UpcomingBase {
 protected:
  using ResourceType = Resource;
  using ResourceSinkType = future<Resource>;

  ResourceSinkType resourceSink_;

 public:
  UpcomingBase<Resource>() = default;
  UpcomingBase(ResourceSinkType sink) : resourceSink_{move(sink)} {}

  bool valid() const { return resourceSink_.valid(); }
  void wait() const { resourceSink_.wait(); }

  decltype(auto) get() { return resourceSink_.get(); }

  template <class Duration>
  decltype(auto) waitFor(const Duration& timeout) {
    return resourceSink_.wait_for(timeout);
  }

  template <class TimePoint>
  decltype(auto) waitUntil(const TimePoint& tp) {
    return resourceSink_.wait_until(tp);
  }
};

template <class Resource>
class Upcoming : public UpcomingBase<Resource> {
 public:
  using Base = UpcomingBase<Resource>;
  using Base::Base;
  using OptionalResource = optional<typename Base::ResourceType>;

  template <class ResourceProcess>
  decltype(auto) then(ResourceProcess process) {
    using NextResourceType = decltype(process(declval<Resource>()));
    if (this->resourceSink_.valid()) {
      auto nextResourceSink = async(
          launch::deferred,
          [process{move(process)}, sink{move(this->resourceSink_)}]() mutable {
            return process(sink.get());
          });

      return Upcoming<NextResourceType>{move(nextResourceSink)};
    } else {
      return Upcoming<NextResourceType>{};
    }
  }

  OptionalResource get() {
    try {
      return this->resourceSink_.get();
    } catch (const future_error&) {
      return {};
    }
  }
};

template <>
class Upcoming<void> : public UpcomingBase<void> {
 public:
  using Base = UpcomingBase<void>;
  using Base::Base;

  template <class ResourceProcess>
  decltype(auto) then(ResourceProcess process) {
    using NextResourceType = decltype(process());
    if (this->resourceSink_.valid()) {
      auto nextResourceSink = async(
          launch::deferred,
          [process{move(process)}, outSink{move(resourceSink_)}]() mutable {
            outSink.get();
            return process();
          });

      return Upcoming<NextResourceType>{move(nextResourceSink)};
    } else {
      return Upcoming<NextResourceType>{};
    }
  }
};

}  // namespace details

template <class Resource>
using Upcoming = details::Upcoming<Resource>;

}  // namespace threading
}  // namespace maf
