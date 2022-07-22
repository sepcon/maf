#ifndef CSCONTRACTDEFINESBEGIN_MC_H
#define CSCONTRACTDEFINESBEGIN_MC_H

#include <maf/messaging/client-server/CSTypes.h>
#include <maf/messaging/client-server/cs_param.h>
#include <maf/utils/cppextension/AggregateCompare.h>

#endif  // CSCONTRACTDEFINESBEGIN_MC_H

// clang-format off
#	ifdef ATTRIBUTES
#		pragma push_macro("ATTRIBUTES")
#		define maf_restore_macro_ATTRIBUTES
#		undef ATTRIBUTES
#	endif
#	ifdef ENDPROPERTY
#		pragma push_macro("ENDPROPERTY")
#		define maf_restore_macro_ENDPROPERTY
#		undef ENDPROPERTY
#	endif
#	ifdef ENDREQUEST
#		pragma push_macro("ENDREQUEST")
#		define maf_restore_macro_ENDREQUEST
#		undef ENDREQUEST
#	endif
#	ifdef ENDSERVICE
#		pragma push_macro("ENDSERVICE")
#		define maf_restore_macro_ENDSERVICE
#		undef ENDSERVICE
#	endif
#	ifdef ENDSIGNAL
#		pragma push_macro("ENDSIGNAL")
#		define maf_restore_macro_ENDSIGNAL
#		undef ENDSIGNAL
#	endif
#	ifdef INPUT
#		pragma push_macro("INPUT")
#		define maf_restore_macro_INPUT
#		undef INPUT
#	endif
#	ifdef mc_maf_csc_declare_feature
#		pragma push_macro("mc_maf_csc_declare_feature")
#		define maf_restore_macro_mc_maf_csc_declare_feature
#		undef mc_maf_csc_declare_feature
#	endif
#	ifdef mc_maf_csc_function_params
#		pragma push_macro("mc_maf_csc_function_params")
#		define maf_restore_macro_mc_maf_csc_function_params
#		undef mc_maf_csc_function_params
#	endif
#	ifdef OUTPUT
#		pragma push_macro("OUTPUT")
#		define maf_restore_macro_OUTPUT
#		undef OUTPUT
#	endif
#	ifdef ERROR
#		pragma push_macro("ERROR")
#		define maf_restore_macro_ERROR
#		undef ERROR
#	endif
#	ifdef PROPERTY
#		pragma push_macro("PROPERTY")
#		define maf_restore_macro_PROPERTY
#		undef PROPERTY
#	endif
#	ifdef REQUEST
#		pragma push_macro("REQUEST")
#		define maf_restore_macro_REQUEST
#		undef REQUEST
#	endif
#	ifdef SERVICE
#		pragma push_macro("SERVICE")
#		define maf_restore_macro_SERVICE
#		undef SERVICE
#	endif
#	ifdef SIGNAL
#		pragma push_macro("SIGNAL")
#		define maf_restore_macro_SIGNAL
#		undef SIGNAL
#	endif
#	ifdef STATUS
#		pragma push_macro("STATUS")
#		define maf_restore_macro_STATUS
#		undef STATUS
#	endif
#	ifdef VOID_REQUEST
#		pragma push_macro("VOID_REQUEST")
#		define maf_restore_macro_VOID_REQUEST
#		undef VOID_REQUEST
#	endif
#	ifdef VOID_SIGNAL
#		pragma push_macro("VOID_SIGNAL")
#		define maf_restore_macro_VOID_SIGNAL
#		undef VOID_SIGNAL
#	endif

// User can provide custom prefix to reduce conflict between operationID
#   ifndef MAF_CS_CONTRACT_PREFIX
#       define MAF_CS_CONTRACT_PREFIX
#   endif

// clang-format on

#include <maf/utils/serialization/SerializableObjectBegin.mc.h>

// Service declarations
#define SERVICE(service)        \
  namespace service##_service { \
    using namespace maf::messaging;

#define ENDSERVICE(service) \
  }                         \
  ;

// Request declarations
#define REQUEST(name) mc_maf_csc_declare_feature(request, name)
#define INPUT(...) mc_maf_csc_function_params(input, __VA_ARGS__)
#define OUTPUT(...) mc_maf_csc_function_params(output, __VA_ARGS__)
#define ERROR(...) mc_maf_csc_function_params(error, __VA_ARGS__)
#define ENDREQUEST(...) \
  }                     \
  ;

#define VOID_REQUEST(name) REQUEST(name) ENDREQUEST(name)

// Property declarations
#define PROPERTY(name) mc_maf_csc_declare_feature(property, name)
#define STATUS(...) mc_maf_csc_function_params(status, __VA_ARGS__)
#define ENDPROPERTY(...) \
  }                      \
  ;

#define SIGNAL(name) mc_maf_csc_declare_feature(signal, name)
#define ATTRIBUTES(...) mc_maf_csc_function_params(attributes, __VA_ARGS__)
#define ENDSIGNAL(...) \
  }                    \
  ;
#define VOID_SIGNAL(name) SIGNAL(name) ENDSIGNAL()

// Implementations
#define mc_maf_csc_declare_feature(type, name)                          \
  struct name##_##type : public maf::messaging::cs_##type {             \
    static constexpr maf::messaging::OpIDConst ID =                     \
        MAF_CS_CONTRACT_PREFIX #name "." #type;                         \
    static constexpr maf::messaging::OpIDConst operationID() noexcept { \
      return ID;                                                        \
    }

#define mc_maf_csc_function_params(type, ...)                             \
 private:                                                                 \
  template <class sb_param_type>                                          \
  using my_sb_param##type =                                               \
      maf::messaging::serializable_cs_param_t<sb_param_type,              \
                                              maf::messaging::cs_##type>; \
                                                                          \
 public:                                                                  \
  OBJECT_EX(type, my_sb_param##type<type>)                                \
  static constexpr maf::messaging::OpIDConst operationID() noexcept {     \
    return ID;                                                            \
  }                                                                       \
  MEMBERS(__VA_ARGS__)                                                    \
  ENDOBJECT(type)                                                         \
  using type##_ptr = std::shared_ptr<type>;                               \
  using type##_cptr = std::shared_ptr<const type>;                        \
  template <typename... Args>                                             \
  static type##_ptr make_##type(Args &&... args) {                        \
    type##_ptr ptr{new type(std::forward<Args>(args)...)};                \
    return ptr;                                                           \
  }
