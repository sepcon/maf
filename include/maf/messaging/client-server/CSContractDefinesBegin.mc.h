#ifndef CSCONTRACTDEFINESBEGIN_MC_H
#   define CSCONTRACTDEFINESBEGIN_MC_H
#   include <maf/messaging/client-server/CSTypes.h>
#   include <maf/messaging/client-server/SerializableMessageTrait.h>
#   include <maf/messaging/client-server/internal/cs_param.h>

// The rest of this file must be putted outside include guard
// Make it to be use with multiple files
#    ifdef ALIAS
#        pragma push_macro("ALIAS")
#        define maf_restore_macro_ALIAS
#    endif
#    ifdef EMPTY_REQUEST
#        pragma push_macro("EMPTY_REQUEST")
#        define maf_restore_macro_EMPTY_REQUEST
#    endif
#    ifdef EMPTY_RESULT
#        pragma push_macro("EMPTY_RESULT")
#        define maf_restore_macro_EMPTY_RESULT
#    endif
#    ifdef EMPTY_STATUS
#        pragma push_macro("EMPTY_STATUS")
#        define maf_restore_macro_EMPTY_STATUS
#    endif
#    ifdef ENDFUNC
#        pragma push_macro("ENDFUNC")
#        define maf_restore_macro_ENDFUNC
#    endif
#    ifdef ENDSERVICE
#        pragma push_macro("ENDSERVICE")
#        define maf_restore_macro_ENDSERVICE
#    endif
#    ifdef FUNCTION
#        pragma push_macro("FUNCTION")
#        define maf_restore_macro_FUNCTION
#    endif
#    ifdef mc_maf_csc_declare_function
#        pragma push_macro("mc_maf_csc_declare_function")
#        define maf_restore_macro_mc_maf_csc_declare_function
#    endif
#    ifdef mc_maf_csc_function_params
#        pragma push_macro("mc_maf_csc_function_params")
#        define maf_restore_macro_mc_maf_csc_function_params
#    endif
#    ifdef mc_maf_csc_function_params_empty
#        pragma push_macro("mc_maf_csc_function_params_empty")
#        define maf_restore_macro_mc_maf_csc_function_params_empty
#    endif
#    ifdef REQUEST
#        pragma push_macro("REQUEST")
#        define maf_restore_macro_REQUEST
#    endif
#    ifdef RESULT
#        pragma push_macro("RESULT")
#        define maf_restore_macro_RESULT
#    endif
#    ifdef SERVICE
#        pragma push_macro("SERVICE")
#        define maf_restore_macro_SERVICE
#    endif
#    ifdef STATUS
#        pragma push_macro("STATUS")
#        define maf_restore_macro_STATUS
#    endif

#include <maf/utils/serialization/MafObjectBegin.mc.h>


#   define SERVICE(service) namespace service##_contract {
#   define ENDSERVICE(service)                      };

#   define FUNCTION(FunctionName)       mc_maf_csc_declare_function(FunctionName)
#   define ENDFUNC(...) };


#   define REQUEST(...)                 mc_maf_csc_function_params(request, __VA_ARGS__)
#   define RESULT(...)                  mc_maf_csc_function_params(result, __VA_ARGS__)
#   define STATUS(...)                  mc_maf_csc_function_params(status, __VA_ARGS__)

#   define EMPTY_REQUEST()              mc_maf_csc_function_params_empty(request)
#   define EMPTY_RESULT()               mc_maf_csc_function_params_empty(result)
#   define EMPTY_STATUS()               mc_maf_csc_function_params_empty(status)

#   define mc_maf_csc_declare_function(Function) \
    struct Function { \
    static constexpr maf::messaging::OpID ID = __LINE__;

#   define mc_maf_csc_function_params(type, ...) \
    private: \
    template <class sb_param_type> \
    using my_sb_param##type = maf::messaging::serializable_cs_param_t<sb_param_type, maf::messaging::cs_##type, ID>; \
    public: OBJECT(type, my_sb_param##type<type>) \
    PROPERTIES(__VA_ARGS__) }; \
    using type##_ptr = std::shared_ptr<type>; \
    template<typename ...Args> \
    static type##_ptr make_##type(Args&&... args) { \
        type##_ptr ptr{ new type { std::forward<Args>(args)... } }; \
        return ptr; \
    }

#   define mc_maf_csc_function_params_empty(type) \
    struct type : public maf::messaging::cs_param_t<maf::messaging::serializable_cs_param_base<maf::messaging::cs_##type>, ID> { \
        std::string dump(int = 1) const { return "{}"; } \
        bool operator<(const type&) { return false; } \
        bool operator==(const type&) { return true; } \
        bool operator!=(const type&) { return false; } \
    }; \
    template<typename ...Args> \
    static std::shared_ptr<type> make##type(Args&&... args) { \
        std::shared_ptr<type> ptr{ new type { std::forward<Args>(args)... } }; \
        return ptr; \
    }
#   define ALIAS(expression) expression

#endif // CSCONTRACTDEFINESBEGIN_MC_H
