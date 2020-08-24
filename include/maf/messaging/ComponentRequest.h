#pragma once

#include <maf/threading/Upcoming.h>

#include <cassert>

#include "Component.h"

namespace maf {
namespace messaging {
namespace details {

using namespace std;
using threading::Upcoming;

enum class RequestType : char { Sync, Async };

template <class Output_, class Input_>
struct RequestMsg_ {
  using Input = Input_;
  using Output = Output_;
  using OutputProcessingCallback = function<void(Output)>;

  RequestMsg_(Input in, OutputProcessingCallback&& callback)
      : input(std::move(in)), outputProcessing(std::move(callback)) {}

  template <class Callback>
  void onOutputGeneratorCallback(const Callback& cb) const {
    outputProcessing(cb(input));
  }

  Input input;
  OutputProcessingCallback outputProcessing;
};

template <class Input_>
struct RequestMsg_<void, Input_> {
  using Input = Input_;
  using Output = void;
  using OutputProcessingCallback = function<void()>;

  RequestMsg_(Input in, OutputProcessingCallback&& callback)
      : input(std::move(in)), outputProcessing(std::move(callback)) {}

  template <class Callback>
  void onOutputGeneratorCallback(const Callback& cb) const {
    cb(input);
    outputProcessing();
  }

  Input input;
  OutputProcessingCallback outputProcessing;
};

template <class Output_, class Input_, RequestType type>
class ComponentRequest;

template <class Output_, class Input_>
class ComponentRequest<Output_, Input_, RequestType::Sync> {
  ComponentInstance comp_;

 public:
  using Input = Input_;
  using Output = Output_;
  using UpcomingOutput = Upcoming<Output>;
  using OutputProcessingCallback =
      typename RequestMsg_<Output, Input>::OutputProcessingCallback;

  explicit ComponentRequest(ComponentInstance comp) : comp_(move(comp)) {
    assert(comp_ && "Given component must not be null");
  }
  UpcomingOutput send(Input in = {}) const {
    using namespace std;
    using RequestMsg = RequestMsg_<Output, Input>;
    using OutputSource = promise<Output>;
    if (comp_) {
      auto outputSource = make_shared<OutputSource>();
      auto outputSink = outputSource->get_future();

      OutputProcessingCallback processOutput;
      if constexpr (is_same_v<void, Output>) {
        processOutput = [outputSource{move(outputSource)}]() mutable {
          outputSource->set_value();
        };
      } else {
        processOutput =
            [outputSource{move(outputSource)}](Output&& out) mutable {
              outputSource->set_value(move(out));
            };
      }

      return comp_->post<RequestMsg>(move(in), move(processOutput))
                 ? UpcomingOutput{move(outputSink)}
                 : UpcomingOutput{};
    }
    return UpcomingOutput{};
  }

  template <class Arg0, class... Args,
            enable_if_t<!is_same_v<Arg0, Input>, bool> = true>
  UpcomingOutput send(Arg0&& arg0, Args&&... args) const {
    return send(Input{forward<Arg0>(arg0), forward<Args>(args)...});
  }
};

template <class Output_, class Input_>
class ComponentRequest<Output_, Input_, RequestType::Async> {
  ComponentInstance comp_;

 public:
  using Input = Input_;
  using Output = Output_;
  using UpcomingOutput = Upcoming<Output>;
  using OutputProcessingCallback =
      typename RequestMsg_<Output, Input>::OutputProcessingCallback;

  explicit ComponentRequest(ComponentInstance comp) : comp_(move(comp)) {
    assert(comp_ && "Given component must not be null");
  }

  bool send(Input input, OutputProcessingCallback&& callback,
            ExecutorPtr executor = {}) const {
    using RequestMsg = RequestMsg_<Output, Input>;
    if (!executor) {
      executor = this_component::getExecutor();
      if (!executor) {
        return false;
      }
    }

    if (comp_) {
      OutputProcessingCallback responseExecution;
      if constexpr (is_same_v<Output, void>) {
        responseExecution = [callback{move(callback)},
                             executor{move(executor)}] {
          executor->execute(move(callback));
        };
      } else {
        responseExecution = [callback{move(callback)},
                             executor{move(executor)}](Output&& output) {
          executor->execute([output{move(output)}, callback{move(callback)}] {
            callback(output);
          });
        };
      }

      return comp_->post<RequestMsg>(move(input), move(responseExecution));
    } else {
      return false;
    }
  }
};
}  // namespace details

using details::ComponentRequest;
using details::RequestType;
using details::Upcoming;
template <class Output, class Input>
using ComponentRequestSync = ComponentRequest<Output, Input, RequestType::Sync>;
template <class Output, class Input>
using ComponentRequestAsync =
    ComponentRequest<Output, Input, RequestType::Async>;

}  // namespace messaging
}  // namespace maf
