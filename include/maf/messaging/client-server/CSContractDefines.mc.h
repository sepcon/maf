#pragma once

#include <maf/messaging/client-server/CSTypes.h>
#include "ipc/IPCMessageTrait.h"

#define result_object_s(OperationName) result_object_s_(OperationName)
#define result_object_no_props(OperationName) result_object_no_props_(OperationName)
#define result_object_e(OperationName) client_server_contract_object_e(OperationName, Result)
#define request_object_s(OperationName) client_server_contract_object_s(OperationName, Request)
#define request_object_e(OperationName) client_server_contract_object_e(OperationName, Request)

#define result_object_s_(OperationName) \
    generate_operation_id(OperationName) \
    client_server_contract_object_s(OperationName, Result)

#define result_object_no_props_(OperationName) \
    result_object_s(OperationName) \
        properties((bool, has_no_property, true)) \
    result_object_e(OperationName)

#define generate_operation_id(OperationName) static constexpr maf::messaging::OpID CSC_OpID_##OperationName = __LINE__;
#define client_server_contract_object_s(OperationName, Type) \
class OperationName##Type : public maf::messaging::ipc::SerializableMessageContentBase \
{ \
public: \
    maf::srz::ByteArray toBytes() override \
    { \
        maf::srz::BASerializer sr; \
        sr << _myProperties; \
        return std::move(sr.mutableBytes()); \
    } \
    void fromBytes(const maf::srz::ByteArray& bytes) override { \
            maf::srz::BADeserializer ds(bytes); \
            ds >> _myProperties; \
    } \
    static constexpr maf::messaging::OpID sOperationID() { return CSC_OpID_##OperationName; } \
    maf::messaging::OpID operationID() const override { return sOperationID(); }



#define properties(...) \
private: \
    mc_tpl_class(Properties) \
    mc_tpl_properties(__VA_ARGS__)

#define client_server_contract_object_e(OperationName, Type) \
mc_tpl_class_end(Properties) \
private: \
    Properties _myProperties; \
    template <typename... Args> \
    OperationName##Type(Args&&... args) : _myProperties(std::forward<Args>(args)...) {} \
public: \
    OperationName##Type() = default; \
    template<typename ...Args> \
    static std::shared_ptr<OperationName##Type> create(Args&&... args) { \
        auto p = new OperationName##Type(std::forward<Args>(args)...); \
        return std::shared_ptr<OperationName##Type>(p); \
    } \
    static std::shared_ptr<OperationName##Type> create() { \
        auto p = new OperationName##Type; \
        return std::shared_ptr<OperationName##Type>(p); \
    } \
    Properties& props() { return _myProperties; } \
    Properties* operator->() { return &_myProperties; } \
};
