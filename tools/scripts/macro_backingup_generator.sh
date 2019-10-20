
# /media/data/CURRENT-WORKING/Git/MessagingAppFramework/include/maf/utils/serialization/Tplkdef.mc.h

mafMacroBackUpRestoreGenerate() 
{
    local header=$1 
    local destination=$2
    [[ -z $destination ]] && destination=$(dirname $header)
    
    local backup=`basename ${header}`
    local restore=${backup}
       
    if ! [[ "${backup}" == *".mc"* ]]; then
        backup=${destination}/"${backup/.h/Begin.mc.h}"
    else
        backup=${destination}/"${backup/.mc/Begin.mc}"
    fi
    if ! [[ "${restore}" == *"."* ]]; then
        restore=${destination}/"${restore/.h/End.mc.h}"
    else
        restore=${destination}/"${restore/.mc/End.mc}"
    fi
    
    local pp="`grep -oE '#define [^\(]*' $header | sort | uniq`"
    
    # pp will look like this:
    #   #define macro_name_xxx
    #   #define macro_name_yyy...
    
    echo "Generate backup file at ${backup} ... "
    
    echo "#pragma once" > ${backup}
    echo "${pp}" | awk ' { 
        print "#\tifdef " $2
        print "#\t\tpragma push_macro(\"" $2 "\")"
        print "#\t\tdefine maf_restore_macro_" $2
        print "#\tendif" 
        }' >> ${backup}
      
    cat $header >> ${backup}
#     echo "#include \"$(basename ${header})\"" >> ${backup}
        
    echo "Generate restore file at ${restore}..."
    
    echo "#pragma once" > ${restore}
    echo "${pp}" | awk ' {
        print "#\tifdef " $2
        print "#\t\tundef " $2
        print "#\tendif" 
        print "#\tifdef maf_restore_macro_" $2
        print "#\t\tpragma pop_macro(\"" $2 "\")"
        print "#\t\tundef maf_restore_macro_" $2
        print "#\tendif" 
        }' >> ${restore}
}
