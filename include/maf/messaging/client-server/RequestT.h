#pragma once

#include "CSError.h"
#include "CSParamConstrains.h"
#include "RequestIF.h"

#include <cassert>
#include <maf/logging/Logger.h>
#include <maf/messaging/CallbackExecutorIF.h>

namespace maf {
namespace messaging {
using namespace paco;

template <class MTrait, class Input> class RequestT {
  template <class MT> friend class Stub;

#define mc_maf_reqt_assert_is_output(Output)                                   \
  static_assert(IsOutput<MTrait, Output>,                                      \
                "the param must be a kind of output or status")

#define mc_maf_reqt_assert_is_same_opid(Output, Input)                         \
  static_assert(MTrait::template getOperationID<Input>() ==                    \
                    MTrait::template getOperationID<Output>(),                 \
                "Output class must has same operationID as Input")

  RequestT(std::shared_ptr<RequestIF> delegate)
      : delegate_(std::move(delegate)) {}

public:
  OpCode getOperationCode() const { return delegate_->getOperationCode(); }
  const OpID &getOperationID() const { return delegate_->getOperationID(); }
  RequestID getRequestID() const { return delegate_->getRequestID(); }
  bool valid() const { return delegate_->valid(); }

  void setAbortRequestHandler(AbortRequestCallback abortCallback,
                              std::shared_ptr<CallbackExecutorIF> executor) {

    assert(executor && "Executor must not be null");
    delegate_->setAbortRequestHandler(std::bind(&CallbackExecutorIF::execute,
                                                std::move(executor),
                                                std::move(abortCallback)));
  }

  std::shared_ptr<Input> getInput() {
    if constexpr (MTrait::template encodable<Input>()) {
      return MTrait::template decode<Input>(delegate_->getInput());
    } else {
      return {}; // means that this request doesn't contain any input
    }
  }

  // Similar as function returns <void>
  ActionCallStatus respond() { return delegate_->respond({}); }

  template <class Output>
  ActionCallStatus respond(const std::shared_ptr<Output> &answer) {
    mc_maf_reqt_assert_is_same_opid(Output, Input);
    mc_maf_reqt_assert_is_output(Output);

    MAF_LOGGER_INFO("Responds to request `", delegate_->getOperationID(),
                    "`: ", MTrait::template dump(answer));

    return delegate_->respond(MTrait::template encode(answer));
  }

  template <class Output, typename Arg0, typename... Args,
            std::enable_if_t<std::is_constructible_v<Output, Arg0, Args...>,
                             bool> = true>
  ActionCallStatus respond(Arg0 resultInput0, Args &&... resultInputs) {
    mc_maf_reqt_assert_is_same_opid(Output, Input);
    mc_maf_reqt_assert_is_output(Output);

    auto answer = std::make_shared<Output>(std::forward<Arg0>(resultInput0),
                                           std::forward<Args>(resultInputs)...);

    MAF_LOGGER_INFO("Responds to request `", delegate_->getOperationID(),
                    "`: ", MTrait::template dump(answer));

    return this->respond(std::move(answer));
  }

  ActionCallStatus error(const std::shared_ptr<CSError> &err) {
    MAF_LOGGER_ERROR("Responds to request `", delegate_->getOperationID(),
                     "`: ", err->dump());
    return delegate_->respond(err);
  }

  ActionCallStatus error(std::string desc,
                         CSError::ErrorCode ec = CSErrorCode::OperationFailed) {
    return error(std::make_shared<CSError>(std::move(desc), std::move(ec)));
  }

private:
  std::shared_ptr<RequestIF> delegate_;
};

} // namespace messaging
} // namespace maf