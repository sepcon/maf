#ifndef CSCONTRACTDEFINESBEGIN_MC_H
#define CSCONTRACTDEFINESBEGIN_MC_H

#include <maf/messaging/client-server/CSTypes.h>
#include <maf/messaging/client-server/SerializableMessageTrait.h>
#include <maf/messaging/client-server/internal/cs_param.h>

#endif // CSCONTRACTDEFINESBEGIN_MC_H

// The rest of this file must be putted outside include guard
// Make it to be use with multiple files
#  ifdef ALIAS
#  pragma push_macro("ALIAS")
#  define maf_restore_macro_ALIAS
#  endif
#  ifdef ENDPROPERTY
#  pragma push_macro("ENDPROPERTY")
#  define maf_restore_macro_ENDPROPERTY
#  endif
#  ifdef ENDREQUEST
#  pragma push_macro("ENDREQUEST")
#  define maf_restore_macro_ENDREQUEST
#  endif
#  ifdef ENDSERVICE
#  pragma push_macro("ENDSERVICE")
#  define maf_restore_macro_ENDSERVICE
#  endif
#  ifdef INPUT
#  pragma push_macro("INPUT")
#  define maf_restore_macro_INPUT
#  endif
#  ifdef mc_maf_csc_declare_feature
#  pragma push_macro("mc_maf_csc_declare_feature")
#  define maf_restore_macro_mc_maf_csc_declare_feature
#  endif
#  ifdef mc_maf_csc_function_params
#  pragma push_macro("mc_maf_csc_function_params")
#  define maf_restore_macro_mc_maf_csc_function_params
#  endif
#  ifdef mc_maf_csc_function_params_empty
#  pragma push_macro("mc_maf_csc_function_params_empty")
#  define maf_restore_macro_mc_maf_csc_function_params_empty
#  endif
#  ifdef OUTPUT
#  pragma push_macro("OUTPUT")
#  define maf_restore_macro_OUTPUT
#  endif
#  ifdef PROPERTY
#  pragma push_macro("PROPERTY")
#  define maf_restore_macro_PROPERTY
#  endif
#  ifdef REQUEST
#  pragma push_macro("REQUEST")
#  define maf_restore_macro_REQUEST
#  endif
#  ifdef SERVICE
#  pragma push_macro("SERVICE")
#  define maf_restore_macro_SERVICE
#  endif
#  ifdef STATUS
#  pragma push_macro("STATUS")
#  define maf_restore_macro_STATUS
#  endif
#  ifdef VOID_INPUT
#  pragma push_macro("VOID_INPUT")
#  define maf_restore_macro_VOID_INPUT
#  endif
#  ifdef VOID_OUTPUT
#  pragma push_macro("VOID_OUTPUT")
#  define maf_restore_macro_VOID_OUTPUT
#  endif
#  ifdef VOID_STATUS
#  pragma push_macro("VOID_STATUS")
#  define maf_restore_macro_VOID_STATUS
#  endif


#include <maf/utils/serialization/MafObjectBegin.mc.h>


// Service declarations
#  define SERVICE(service) \
  namespace service##_service { \
  using namespace maf::messaging;

#  define ENDSERVICE(service)  };


// Request declarations
#  define REQUEST(name)  mc_maf_csc_declare_feature(request, name)
#  define INPUT(...)  mc_maf_csc_function_params(input, __VA_ARGS__)
#  define OUTPUT(...)  mc_maf_csc_function_params(output, __VA_ARGS__)
#  define ENDREQUEST(...)  };

#  define VOID_REQUEST(name) REQUEST(name) ENDREQUEST()


// Property declarations
#  define PROPERTY(name)  mc_maf_csc_declare_feature(property, name)
#  define STATUS(...)  mc_maf_csc_function_params(status, __VA_ARGS__)
#  define ENDPROPERTY(...)  };

#  define SIGNAL(name)  mc_maf_csc_declare_feature(signal, name)
#  define ATTRIBUTES(...)  mc_maf_csc_function_params(attributes, __VA_ARGS__)
#  define ENDSIGNAL(...)  };
#  define VOID_SIGNAL(name) SIGNAL(name) ENDSIGNAL()


// Implementations
#  define mc_maf_csc_declare_feature(type, name) \
  struct name : public cs_##type{ \
  static constexpr OpIDConstant ID =  "OpID_"#name "_" #type ; \
  static constexpr OpIDConstant operationID() { return ID; }

#  define mc_maf_csc_function_params(type, ...) \
  private: \
  template <class sb_param_type> \
  using my_sb_param##type = serializable_cs_param_t<sb_param_type, cs_##type>; \
  public: \
  OBJECT(type, my_sb_param##type<type>) \
  static constexpr OpIDConstant operationID() { return ID; } \
  MEMBERS(__VA_ARGS__) }; \
  using type##_ptr = std::shared_ptr<type>; \
  template<typename... Args> \
  static type##_ptr make_##type(Args&&... args) { \
    type##_ptr ptr{ new type { std::forward<Args>(args)... } }; \
    return ptr; \
  }

//#  define mc_maf_csc_function_params_empty(type) \
//  struct type : public serializable_cs_param_base<cs_##type> { \
//  static constexpr OpIDConstant operationID() { return ID; } \
//  std::string dump(int = 1) const { return "{}"; } \
//  bool operator<(const type&) { return false; } \
//  bool operator==(const type&) { return true; } \
//  bool operator!=(const type&) { return false; } \
//  }; \
//  template<typename ...Args> \
//  static std::shared_ptr<type> make##type(Args&&... args) { \
//  std::shared_ptr<type> ptr{ new type { std::forward<Args>(args)... } }; \
//  return ptr; \
//  }
#  define ALIAS(expression) expression

