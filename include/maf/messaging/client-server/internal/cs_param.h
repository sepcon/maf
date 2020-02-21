#pragma once

#include <maf/messaging/client-server/CSMessage.h>
#include <maf/utils/cppextension/TypeInfo.h>
#include <maf/utils/serialization/BASerializer.h>

namespace maf {
namespace messaging {

struct cs_operation {};

struct cs_param             : public CSMessageContentBase
{
    cs_param() { setType(Type::Data); }
};


struct cs_request           : public cs_operation   {};
struct cs_property          : public cs_operation   {};
struct cs_signal            : public cs_operation   {};

struct cs_inputbase         : public cs_param       {};
struct cs_input             : public cs_inputbase   {};

struct cs_outputbase        : public cs_param       {};
struct cs_output            : public cs_outputbase  {};
struct cs_status            : public cs_outputbase  {};
struct cs_attributes        : public cs_outputbase  {};


template<class cs_param_type>
struct serializable_cs_param_base : public cs_param_type
{
    using cs_param_type::cs_param_type;
    virtual maf::srz::ByteArray toBytes() { return "''"; }
    virtual void fromBytes(const maf::srz::ByteArray &) {}
};


template <class SerializableCSParamClass, class cs_param_type>
struct serializable_cs_param_t :
    public serializable_cs_param_base<cs_param_type>
{
    bool equal(const CSMessageContentBase* other) const override
    {
        util::debugAssertTypesEqual(this, other);
        if(other && (this->type() == other->type()))
        {
            auto Other = static_cast<const SerializableCSParamClass*>(other);
            auto This = static_cast<const SerializableCSParamClass*>(this);
            return *Other == *This;
        }
        return false;
    }

    CSMessageContentBase* clone() const override
    {
            return new SerializableCSParamClass(
                *static_cast<const SerializableCSParamClass*>(this)
            );
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
