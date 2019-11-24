#pragma once

#include <maf/messaging/client-server/CSMessage.h>
#include <maf/utils/cppextension/TypeInfo.h>
#include <maf/utils/serialization/BASerializer.h>

namespace maf {
namespace messaging {

struct cs_param             : public CSMessageContentBase {};
struct cs_request           : public cs_param {};
struct cs_property          : public cs_param {};
struct cs_input             : public cs_param {};
struct cs_output            : public cs_param {};
struct cs_status            : public cs_param {};


template<class cs_param_type>
struct serializable_cs_param_base : public cs_param_type
{
    virtual maf::srz::ByteArray toBytes() { return "''"; }
    virtual void fromBytes(const maf::srz::ByteArray &) {}
    bool equal(const CSMessageContentBase*) override { return true; }
};


template <class SerializableCSParamClass, class cs_param_type>
struct serializable_cs_param_t :
    public serializable_cs_param_base<cs_param_type>
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
