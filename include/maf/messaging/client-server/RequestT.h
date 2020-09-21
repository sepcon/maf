#pragma once

#include <maf/logging/Logger.h>
#include <maf/utils/ExecutorIF.h>

#include <cassert>

#include "CSError.h"
#include "CSParamConstrains.h"
#include "RequestIF.h"

namespace maf {
namespace messaging {
using namespace paco;

template <class PTrait, class Input>
class RequestT {
  template <class MT>
  friend class BasicStub;

#define mc_maf_reqt_assert_is_output(Output) \
  static_assert(IsOutput<PTrait, Output>,    \
                "the param must be a kind of output or status")

#define mc_maf_reqt_assert_is_same_opid(Output, Input) /* \
static_assert(PTrait::template getOperationID<Input>() == \
PTrait::template getOperationID<Output>(),                \
"Output class must has same operationID as Input")*/

  RequestT(std::shared_ptr<RequestIF> delegate)
      : delegate_(std::move(delegate)) {}

  RequestT(const RequestT &) = delete;
  RequestT &operator=(const RequestT &) = delete;

 public:
  RequestT(RequestT &&) = default;
  RequestT &operator=(RequestT &&) = default;

  ~RequestT() {
    if (delegate_ && valid()) {
      error("Handler failed to explicitly respond to this request",
            CSErrorCode::ResponseIgnored);
    }
  }
  OpCode getOperationCode() const { return delegate_->getOperationCode(); }
  const OpID &getOperationID() const { return delegate_->getOperationID(); }
  RequestID getRequestID() const { return delegate_->getRequestID(); }
  bool valid() const { return delegate_->valid(); }

  void onAborted(AbortRequestCallback abortCallback,
                 util::ExecutorIFPtr executor) {
    assert(executor && "Executor must not be null");
    delegate_->onAborted(std::bind(&ExecutorIF::execute, std::move(executor),
                                   std::move(abortCallback)));
  }

  std::shared_ptr<Input> getInput() const {
    if constexpr (PTrait::template encodable<Input>()) {
      auto input = PTrait::template translate<Input>(delegate_->getInput());
      MAF_LOGGER_VERBOSE("Input of request: `", getOperationID(),
                         "`: ", PTrait::template dump<Input>(input));
      return input;
    } else {
      return {};  // means that this request doesn't contain any input
    }
  }

  // Similar as function returns <void>
  ActionCallStatus respond() { return delegate_->respond({}); }

  template <class Output>
  ActionCallStatus respond(const std::shared_ptr<Output> &answer) {
    mc_maf_reqt_assert_is_same_opid(Output, Input);
    mc_maf_reqt_assert_is_output(Output);

    MAF_LOGGER_VERBOSE("Responds to request `", delegate_->getOperationID(),
                       "`: ", PTrait::template dump(answer));

    return delegate_->respond(PTrait::template translate(answer));
  }

  template <class Output, typename Arg0, typename... Args,
            std::enable_if_t<std::is_constructible_v<Output, Arg0, Args...>,
                             bool> = true>
  ActionCallStatus respond(Arg0 resultInput0, Args &&... resultInputs) {
    mc_maf_reqt_assert_is_same_opid(Output, Input);
    mc_maf_reqt_assert_is_output(Output);

    auto answer = std::make_shared<Output>(std::forward<Arg0>(resultInput0),
                                           std::forward<Args>(resultInputs)...);
    return this->respond(std::move(answer));
  }

  ActionCallStatus error(const std::shared_ptr<CSError> &err) {
    MAF_LOGGER_ERROR("Respond an error to request: `",
                     delegate_->getOperationID(), "`: ", err->dump());
    return delegate_->respond(err);
  }

  ActionCallStatus error(std::string desc,
                         CSError::ErrorCode ec = CSErrorCode::OperationFailed) {
    return error(std::make_shared<CSError>(std::move(desc), std::move(ec)));
  }

 private:
  std::shared_ptr<RequestIF> delegate_;
};

}  // namespace messaging
}  // namespace maf
