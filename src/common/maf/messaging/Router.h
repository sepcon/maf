#pragma once

#include <maf/messaging/Processor.h>
#include <maf/messaging/Routing.h>
#include <maf/patterns/Patterns.h>
#include <maf/threading/Lockable.h>

#include <mutex>
#include <set>

namespace maf {
namespace messaging {
namespace details {
using namespace routing;

struct ProcessorCompare {
  typedef int is_transparent;
  bool operator()(const ProcessorInstance &r1,
                  const ProcessorInstance &r2) const {
    return r1 != r2 && r1->id() < r2->id();
  }

  bool operator()(const ProcessorInstance &r, const ProcessorID &id) const {
    return r->id() < id;
  }

  bool operator()(const ProcessorID &id, const ProcessorInstance &r) const {
    return id < r->id();
  }
};

using Processors = std::set<ProcessorInstance, ProcessorCompare>;

class Router : public pattern::SingletonObject<Router> {
 public:
  Router(Invisible) noexcept {}
  bool post(const ProcessorID &messageprocessorID, Message &&msg);
  Processor::CompleteSignal send(const ProcessorID &messageprocessorID, Message msg);
  bool postToAll(const Message &msg);
  Processor::CompleteSignal sendToAll(const Message &msg);

  ProcessorInstance findProcessor(const ProcessorID &id) const;
  bool addProcessor(ProcessorInstance comp);
  bool removeProcessor(const ProcessorInstance &comp);

 private:
  using AtomicProcessors = threading::Lockable<Processors, std::mutex>;

  AtomicProcessors messageprocessors_;
};

}  // namespace details

using details::Router;

}  // namespace messaging
}  // namespace maf
