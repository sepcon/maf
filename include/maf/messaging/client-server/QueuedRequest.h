#pragma once

#include "internal/cs_param.h"
#include "CSContentError.h"
#include "RequestInterface.h"
#include <maf/patterns/Patterns.h>
#include <maf/messaging/Component.h>
#include <maf/messaging/BasicMessages.h>
#include <maf/logging/Logger.h>

namespace maf {
namespace messaging {

#define mc_maf_assert_is_result_or_status(ResultOrStatus)                      \
static_assert(                                                                 \
    std::is_base_of_v<cs_output, ResultOrStatus> ||                            \
    std::is_base_of_v<cs_status, ResultOrStatus>,                              \
    "ResultOrStatus class must be type of class cs_output cs_status,"          \
    "it means that you're respond a request object or some "                   \
    "other ones to QueuedRequest::respond method"                              \
    )

#define mc_maf_assert_is_same_opid(ResultOrStatus, Input)                      \
static_assert(                                                                 \
    MessageTrait::template getOperationID<Input>() ==                          \
    MessageTrait::template getOperationID<ResultOrStatus>(),                   \
    "ResultOrStatus class must has same operationID as InputType"              \
    )

template<class MessageTrait, class InputType>
class QueuedRequest
{
    static_assert (
        std::is_base_of_v<cs_input, InputType> ||
            std::is_base_of_v<cs_request, InputType> ||
            std::is_base_of_v<cs_property, InputType>,
        "must be a cs_input or cs_request or cs_property"
        );
public:
    QueuedRequest(std::shared_ptr<RequestInterface> delegate)
        : _delegate(std::move(delegate)) {}
    OpCode getOperationCode() const
    {
        return _delegate->getOperationCode();
    }
    const OpID& getOperationID() const
    {
        return _delegate->getOperationID();
    }
    RequestID getRequestID() const
    {
        return _delegate->getRequestID();
    }
    bool valid() const
    {
        return _delegate->valid();
    }

    void setAbortRequestHandler(AbortRequestCallback abortCallback)
    {
        if(Component::getActiveSharedPtr())
        {
            auto abortHandler = [ compref = Component::getActiveWeakPtr(),
                                 abortCallback = std::move(abortCallback)] {
                if(auto component = compref.lock())
                {
                    component->post<CallbackExcMsg>(
                        std::move(abortCallback)
                        );
                }
            };
            _delegate->setAbortRequestHandler(std::move(abortHandler));
        }
        else
        {
            logging::Logger::error("trying register AbortRequestCallback from"
                                   "non-component context");
        }
    }


    std::shared_ptr<InputType> getInput()
    {
        if constexpr (MessageTrait::template encodable<InputType>())
		{
			return MessageTrait::template decode<InputType>(
				_delegate->getInput()
				);
		}
		else
		{
			return {}; // means that this request doesn't contain any input
		}
    }


    // Similar as function returns <void>
    ActionCallStatus respond()
    {
        return _delegate->respond({});
    }

    template<class ResultOrStatus>
    ActionCallStatus respond(const std::shared_ptr<ResultOrStatus>& answer)
    {
        mc_maf_assert_is_same_opid(ResultOrStatus, InputType);
        mc_maf_assert_is_result_or_status(ResultOrStatus);
        return _delegate->respond(
            MessageTrait::template encode<cs_outputbase>(answer)
            );
    }

    template<class ResultOrStatus,
             typename Arg0,
             typename... Args,
             std::enable_if_t<
                 std::is_constructible_v<ResultOrStatus, Arg0, Args...>,
                 bool> = true
             >
    ActionCallStatus respond(Arg0 resultInput0, Args&&... resultInputs)
    {
        mc_maf_assert_is_same_opid(ResultOrStatus, InputType);
        mc_maf_assert_is_result_or_status(ResultOrStatus);

        return this->respond(
            std::make_shared<ResultOrStatus>(
                std::forward<Arg0>(resultInput0),
                std::forward<Args>(resultInputs)...)
            );
    }

    ActionCallStatus error(const std::shared_ptr<CSContentError>& err)
    {
        return _delegate->respond( err );
    }

    ActionCallStatus error(
        std::string desc,
        CSContentError::ErrorCode ec =
            CSContentError::PreservedErrorCode::OperationFailed
        )
    {
        return error(
            std::make_shared<CSContentError>(
                std::move(desc),
                std::move(ec)
                )
            );
    }

private:
    std::shared_ptr<RequestInterface> _delegate;
};

} //messaging
} //maf
