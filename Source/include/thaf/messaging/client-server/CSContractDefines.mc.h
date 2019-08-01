#pragma once

#include "thaf/messaging/client-server/CSTypes.h"
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

#define generate_operation_id(OperationName) static constexpr thaf::messaging::OpID CSC_OpID_##OperationName = __LINE__;
#define client_server_contract_object_s(OperationName, Type) \
class OperationName##Type : public thaf::messaging::ipc::SerializableMessageContentBase \
{ \
public: \
    thaf::srz::ByteArray toBytes() override \
    { \
        thaf::srz::BASerializer sr; \
        sr << _myProperties; \
        return std::move(sr.mutableBytes()); \
    } \
    void fromBytes(const thaf::srz::ByteArray& bytes) override { \
            thaf::srz::BADeserializer ds(bytes); \
            ds >> _myProperties; \
    } \
    static constexpr thaf::messaging::OpID sOperationID() { return CSC_OpID_##OperationName; } \
    thaf::messaging::OpID operationID() const override { return sOperationID(); }



#define properties(...) \
private: \
    mc_sbClass(Properties) \
    mc_sbProperties(__VA_ARGS__)

#define client_server_contract_object_e(OperationName, Type) \
mc_sbClass_end(Properties) \
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
