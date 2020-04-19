#pragma once

#include <cassert>
#include <maf/messaging/client-server/CSMessage.h>
#include <maf/utils/serialization/SerializableIF.h>

namespace maf {
namespace messaging {

// clang-format off
struct cs_operation
{
    std::string dump() { return "cs_param"; }
};

struct cs_param             : public CSMessageContentBase
{
    cs_param(): CSMessageContentBase(Type::Data){ }
    std::string dump() { return "cs_param"; }
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

// clang-format on

template <class SerializableCSParamClass, class cs_param_type>
struct serializable_cs_param_t : public cs_param_type,
                                 public srz::SerializableIF {
  bool equal(const CSMessageContentBase *other) const override {
    if (other && (this->type() == other->type())) {
      assert(typeid(*this) == typeid(*other));
      auto Other = static_cast<const SerializableCSParamClass *>(other);
      auto This = static_cast<const SerializableCSParamClass *>(this);
      return *Other == *This;
    }
    return false;
  }

  CSMessageContentBase *clone() const override {
    return new SerializableCSParamClass(
        *static_cast<const SerializableCSParamClass *>(this));
  }

  bool serialize(srz::OByteStream &os) const override {
    maf::srz::SR sr(os);
    sr << *static_cast<const SerializableCSParamClass *>(this);
    return !os.fail();
  }

  bool deserialize(srz::IByteStream &is) override {
    maf::srz::DSR ds(is);
    ds >> *static_cast<SerializableCSParamClass *>(this);
    return !is.fail();
  }
};

} // namespace messaging
} // namespace maf
