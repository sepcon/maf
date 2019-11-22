#pragma once

#include <maf/messaging/client-server/CSMessage.h>
#include <maf/utils/cppextension/TypeInfo.h>
#include <maf/utils/serialization/BASerializer.h>

namespace maf {
namespace messaging {

struct cs_param          : public CSMessageContentBase {};
struct cs_request        : public cs_param {};
struct cs_result         : public cs_param {};
struct cs_status         : public cs_param {};


template <class BaseParam, OpID OperationID>
struct cs_param_t : public BaseParam
{
    static constexpr maf::messaging::OpID operationID() { return OperationID; }
};

template<class parent_class>
struct serializable_cs_param_base : public parent_class
{
    virtual maf::srz::ByteArray toBytes() { return "''"; }
    virtual void fromBytes(const maf::srz::ByteArray &) {}
    bool equal(const CSMessageContentBase*) override { return true; }
};


template <class SerializableCSParamClass, class BaseParam, OpID OperationID>
struct serializable_cs_param_t :
    public cs_param_t<serializable_cs_param_base<BaseParam>, OperationID>
{
    bool equal(const CSMessageContentBase* other) override
    {
        util::debugAssertTypesEqual(this, other);
        if(other)
        {
            auto Other = static_cast<const SerializableCSParamClass*>(other);
            auto This = static_cast<const SerializableCSParamClass*>(this);
            return *Other == *This;
        }
        return false;
    }

    maf::srz::ByteArray toBytes()
    {
        maf::srz::BASerializer sr;
        sr << * static_cast<SerializableCSParamClass*>(this);
        return std::move(sr.mutableBytes());
    }

    void fromBytes(const maf::srz::ByteArray &bytes)
    {
        maf::srz::BADeserializer ds(bytes);
        ds >> * static_cast<SerializableCSParamClass*>(this);
    }
};

}
}
