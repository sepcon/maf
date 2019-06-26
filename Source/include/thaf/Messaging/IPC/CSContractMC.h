#pragma once

#include "ClientServerContract.h"

#define result_object_s(ActionName) \
static constexpr thaf::messaging::ipc::OpID CSC_OpID_##ActionName = __LINE__; \
client_server_contract_object_s(ActionName, Result)

#define result_object_e(ActionName) client_server_contract_object_e(ActionName, Result)

#define request_object_s(ActionName) client_server_contract_object_s(ActionName, Request)
#define request_object_e(ActionName) client_server_contract_object_e(ActionName, Request)

#define client_server_contract_object_s(ActionName, Type)                                                            \
class ActionName##Type : public thaf::messaging::ipc::IPCDataCarrier                                                 \
{                                                                                                                    \
public:                                                                                                              \
    thaf::srz::ByteArray toBytes() const override                                                                    \
    {                                                                                                                \
        thaf::srz::BASerializer sr;                                                                                  \
        sr << _myProperties;                                                                                               \
        return std::move(sr.mutableBytes());                                                                         \
    }                                                                                                                \
    void fromBytes(const thaf::srz::ByteArray& bytes) override {                                                     \
        thaf::srz::BADeserializer ds(bytes);                                                                         \
        ds >> _myProperties;                                                                                               \
    }                                                                                                                \
    static constexpr thaf::messaging::ipc::OpID ID() { return CSC_OpID_##ActionName; }                                         \
    thaf::messaging::ipc::OpID getID() const override { return ID(); }                                               \



#define properties(...)                                                                                             \
private:                                                                                                             \
    mc_sbClass(Properties)                                                                              \
    mc_sbProperties(__VA_ARGS__)

#define client_server_contract_object_e(ActionName, Type)                                                           \
mc_sbClass_end(Properties)                                                                              \
private:                                                                                                            \
    Properties _myProperties;                                                                                 \
    template <typename... Args>                                                                                     \
    ActionName##Type(Args&&... args) : _myProperties(std::forward<Args>(args)...) {}                                      \
public:                                                                                                             \
    ActionName##Type() = default;                                                                                   \
                                                                                                                    \
    template<typename ...Args>                                                                                      \
    static std::shared_ptr<ActionName##Type> create(Args&&... args) {                                               \
        auto p = new ActionName##Type(std::forward<Args>(args)...);                                                 \
        return std::shared_ptr<ActionName##Type>(p);                                                                \
    }                                                                                                               \
    static std::shared_ptr<ActionName##Type> create() {                                                             \
        auto p = new ActionName##Type;                                                                              \
        return std::shared_ptr<ActionName##Type>(p);                                                                \
    }                                                                                                               \
    Properties& props() { return _myProperties; }                                                              \
    Properties* operator->() { return &_myProperties; }                                                       \
};
