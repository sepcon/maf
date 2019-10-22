#ifndef CSCONTRACTDEFINESBEGIN_MC_H
#   define CSCONTRACTDEFINESBEGIN_MC_H
#   include <maf/messaging/client-server/CSTypes.h>
#   include <maf/messaging/client-server/ipc/IPCMessageTrait.h>
#endif // CSCONTRACTDEFINESBEGIN_MC_H

// The rest of this file must be putted outside include guard
// Make it to be use with multiple files
#	ifdef ENDFUNC
#		pragma push_macro("ENDFUNC")
#		define maf_restore_macro_endfunc
#	endif
#	ifdef FUNCTION
#		pragma push_macro("FUNCTION")
#		define maf_restore_macro_FUNCTION
#	endif
#	ifdef mc_maf_csc_function_params
#		pragma push_macro("mc_maf_csc_function_params")
#		define maf_restore_macro_mc_maf_csc_function_params
#	endif
#	ifdef mc_maf_csc_function_params_empty
#		pragma push_macro("mc_maf_csc_function_params_empty")
#		define maf_restore_macro_mc_maf_csc_function_params_empty
#	endif
#	ifdef MESSAGE
#		pragma push_macro("MESSAGE")
#		define maf_restore_macro_MESSAGE
#	endif
#	ifdef EMPTY_MESSAGE
#		pragma push_macro("EMPTY_MESSAGE")
#		define maf_restore_macro_EMPTY_MESSAGE
#	endif
#	ifdef REQUEST_MESSAGE
#		pragma push_macro("REQUEST_MESSAGE")
#		define maf_restore_macro_REQUEST_MESSAGE
#	endif
#	ifdef EMPTY_REQUEST_MESSAGE
#		pragma push_macro("EMPTY_REQUEST_MESSAGE")
#		define maf_restore_macro_EMPTY_REQUEST_MESSAGE
#	endif
#	ifdef RESULT_MESSAGE
#		pragma push_macro("RESULT_MESSAGE")
#		define maf_restore_macro_RESULT_MESSAGE
#	endif
#	ifdef EMPTY_RESULT_MESSAGE
#		pragma push_macro("EMPTY_RESULT_MESSAGE")
#		define maf_restore_macro_EMPTY_RESULT_MESSAGE
#	endif
#	ifdef EVENT_MESSAGE
#		pragma push_macro("EVENT_MESSAGE")
#		define maf_restore_macro_EVENT_MESSAGE
#	endif

#include <maf/utils/serialization/MafObjectBegin.mc.h>


#define FUNCTION(FunctionName) mc_maf_csc_declare_function(FunctionName)
#define ENDFUNC(...) };

#define MESSAGE                     mc_maf_csc_function_params
#define REQUEST_MESSAGE(...)        MESSAGE(Request, __VA_ARGS__)
#define RESULT_MESSAGE(...)         MESSAGE(Result, __VA_ARGS__)
#define EVENT_MESSAGE(...)          MESSAGE(Event, __VA_ARGS__)

#define EMPTY_MESSAGE               mc_maf_csc_function_params_empty
#define EMPTY_REQUEST_MESSAGE()     EMPTY_MESSAGE(Request)
#define EMPTY_RESULT_MESSAGE()      EMPTY_MESSAGE(Result)
#define EMPTY_EVENT_MESSAGE()       EMPTY_MESSAGE(Event)


#define mc_maf_csc_declare_function(Function) \
struct Function { \
    static constexpr maf::messaging::OpID ID() { \
        return __LINE__; \
    }

#define mc_maf_csc_function_params(Type, ...) \
OBJECT(Type, maf::messaging::ipc::SerializableMessageContentBase) \
public: \
    maf::srz::ByteArray toBytes() override \
    { \
        maf::srz::BASerializer sr; \
        sr << *this; \
        return std::move(sr.mutableBytes()); \
    } \
    void fromBytes(const maf::srz::ByteArray& bytes) override { \
            maf::srz::BADeserializer ds(bytes); \
            ds >> *this; \
    } \
    static constexpr maf::messaging::OpID sOperationID() { \
        return ID(); \
    } \
    maf::messaging::OpID operationID() const override { \
        return sOperationID(); \
    } \
    PROPERTIES(__VA_ARGS__) }; \
    \
    template<typename ...Args> \
    static std::shared_ptr<Type> make##Type(Args&&... args) { \
        std::shared_ptr<Type> ptr{ new Type { std::forward<Args>(args)... } }; \
        return ptr; \
    }

#define mc_maf_csc_function_params_empty(Type) \
    struct Type : public maf::messaging::ipc::SerializableMessageContentBase { \
        static constexpr maf::messaging::OpID sOperationID() { \
            return ID(); \
        } \
    }; \
    template<typename ...Args> \
    static std::shared_ptr<Type> make##Type(Args&&... args) { \
        std::shared_ptr<Type> ptr{ new Type { std::forward<Args>(args)... } }; \
        return ptr; \
    }

