#ifndef HEADERS_FRAMEWORK_MESSAGING_IPC_INTERFACES_CLIENTSERVERCONTRACTMACROS_H
#define HEADERS_FRAMEWORK_MESSAGING_IPC_INTERFACES_CLIENTSERVERCONTRACTMACROS_H

#   include "thaf/utils/serialization/SerializableObject.h"
#   include "thaf/messaging/ipc/ClientServerContractMacros.h"

#endif

// below section is not allowed to declare inside include_guard
#ifndef client_server_import_order
#   define client_server_import_order 0
#endif

#if  defined(client_server_contract_enum_generation)
#   if client_server_import_order != 0
#       error "Must define client_server_contract_class_generation after client_server_contract_enum_generation"
#   endif

#   undef client_server_import_order
#   define client_server_import_order 1

#   undef contract_begin
#   undef contract_end
#   undef result_object_s
#   undef result_object_e
#   undef request_object_s
#   undef request_object_e
#   undef properties
#   undef client_server_contract_object_s
#   undef client_server_contract_object_e

#   define contract_begin(ContractName)     enum Enum_OpID_##ContractName {
#   define contract_end(ContractName)       OpID_UnSuport };
#   define result_object_s(ActionName)        OpID_##ActionName,
#   define result_object_e(ActionName)
#   define request_object_s(ActionName)
#   define request_object_e(ActionName)
#   define properties(...)
#   define client_server_contract_object_s(ActionName, Type)
#   define client_server_contract_object_e(ActionName, Type)

#   undef client_server_contract_enum_generation

#elif defined(client_server_contract_class_generation)
#   if client_server_import_order != 1
#       error "Must define client_server_contract_class_generation after client_server_contract_enum_generation"
#   endif

#   undef client_server_import_order
#   define client_server_import_order 2

#   undef contract_begin
#   undef contract_end
#   undef result_object_s
#   undef result_object_e
#   undef request_object_s
#   undef request_object_e
#   undef properties
#   undef client_server_contract_object_s
#   undef client_server_contract_object_e

#   define contract_begin(ContractName)
#   define contract_end(ContractName)

#   define result_object_s(ActionName) client_server_contract_object_s(ActionName, Result)
#   define result_object_e(ActionName) client_server_contract_object_e(ActionName, Result)

#   define request_object_s(ActionName) client_server_contract_object_s(ActionName, Request)
#   define request_object_e(ActionName) client_server_contract_object_e(ActionName, Request)

#   define client_server_contract_object_s(ActionName, Type)                													\
           class ActionName##Type : public thaf::messaging::ipc::IPCMessageContentBase                                                 \
           {                                                                                                                    \
           public:                                                                                                              \
               thaf::srz::ByteArray toBytes() const override                                                                    \
               {                                                                                                                \
                   thaf::srz::BASerializer sr;                                                                                  \
                   sr << _myData;                                                                                               \
                   return std::move(sr.mutableBytes());                                                                         \
               }                                                                                                                \
               void fromBytes(const thaf::srz::ByteArray& bytes) override {                                                     \
                   thaf::srz::BADeserializer ds(bytes);                                                                         \
                   ds >> _myData;                                                                                               \
               }                                                                                                                \
               static thaf::messaging::ipc::OpID ID() { return OpID_##ActionName; }                                             \
               thaf::messaging::ipc::OpID getID() const override { return ID(); }                                               \
                                                                                                                                \
           public:                                                                                                              \
               mc_sbClass(ActionName##Type##Data)


#   define properties(...) mc_sbProperties(__VA_ARGS__)

#   define client_server_contract_object_e(ActionName, Type)                                                                    \
               mc_sbClass_end(ActionName##Type##Data)                                                                           \
           private:                                                                                                             \
               ActionName##Type##Data _myData;                                                                                  \
           public:                                                                                                              \
               ActionName##Type() = default;                                                                                    \
               template <typename... Args>                                                                                      \
               ActionName##Type(Args&&... args) : _myData(std::forward<Args>(args)...) {}                                       \
                                                                                                                                \
               template<typename ...Args>                                                                                       \
               static std::shared_ptr<ActionName##Type> create(Args&&... args) {                                                \
                   auto p = new ActionName##Type(std::forward<Args>(args)...);                                                  \
                   return std::shared_ptr<ActionName##Type>(p);                                                                 \
               }                                                                                                                \
               static std::shared_ptr<ActionName##Type> create() {                                                              \
                   auto p = new ActionName##Type;                                                                               \
                   return std::shared_ptr<ActionName##Type>(p);                                                                 \
               }                                                                                                                \
               ActionName##Type##Data& data() { return _myData; }                                                               \
               ActionName##Type##Data* operator->() { return &_myData; }                                                        \
           };

#   undef client_server_contract_class_generation

#endif


