/// This section is for backing up the macros if they are already defined in some third party libs
/// Try to isolate this file and do not include this file with other third party headers
// Backup macros starts !------------------------------------------------------->
#   ifdef contract_begin
#       pragma message("contract_begin is already defined")
#       pragma push_macro("contract_begin")
#       define maf_restore_macro_contract_begin
#   endif
#   ifdef contract_end
#       pragma push_macro("contract_end")
#       define maf_restore_macro_contract_end
#   endif
#   ifdef result_object_s
#       pragma push_macro("result_object_s")
#       define maf_restore_macro_result_class_s
#   endif
#   ifdef result_object_e
#       pragma push_macro("result_object_e")
#       define maf_restore_macro_result_class_e
#   endif
#   ifdef request_object_s
#       pragma push_macro("request_object_s")
#       define maf_restore_macro_request_object_s
#   endif
#   ifdef request_object_e
#       pragma push_macro("request_object_e")
#       define maf_restore_macro_request_object_e
#   endif
#   ifdef properties
#       pragma push_macro("properties")
#       define maf_restore_macro_properties
#   endif
#   ifdef client_server_contract_object_s
#       pragma push_macro("client_server_contract_object_s")
#       define maf_restore_macro_client_server_contract_object_s
#   endif
#   ifdef client_server_contract_object_e
#       pragma push_macro("client_server_contract_object_e")
#       define maf_restore_macro_client_server_contract_object_e
#   endif
// Backup macros ends <!-------------------------------------------------------
