/**
 * @brief Use below MACROS to define a serializable object like this:
 *
 * OBJECT(TheObject) // or OBJECT(TheObject, BaseObject)
 *     MEMBERS
 *     (
 *          (std::string, Name),
 *          (int, Type),
 *          (std::string, Action),
 *          (Type, Name) //( <Type, Name> must be enclosed by parentheses)
 *     )
 * ENDOBJECT(TheObject)
 *
 * After that, you can use it for serialization like this:
 *
 *  int type = 10;
 *  TheObject object("name", type, "Pull request action");
 *  std::vector<std::string> vec = {....};
 *
 *  maf::srz::BASerializer sr;
 *  sr << object << vec; // done serialize
 *
 *  //... write sr.c_str() to file, share it between programs
 *  //... now deserialize it
 *
 *  maf::srz::ByteArray ba = readFromFile/FromPipe();
 *  maf::srz::BADeserializer dsr(ba);
 *  TheObject object;
 *  std::vector<std::string> vec;
 *  dsr >> object >> vec; //The order must be kept same as serialization
 *
 *  std::cout << object.name() << object.type() << object.action();
 *  printVector(vec) ...;
 *
 * */

#ifndef MAFOBJECTBEGIN_MC_H
#define MAFOBJECTBEGIN_MC_H
#   include "Serializer.h"
#   include "DumpHelper.h"
#endif // MAFOBJECTBEGIN_MC_H

// The rest of this file must be putted outside include guard
// Make it to be use with multiple files

#include "Internal/TplkdefBegin.mc.h"

#    ifdef OBJECT
#        pragma push_macro("OBJECT")
#        define maf_restore_macro_OBJECT
#    endif
#    ifdef MEMBERS
#        pragma push_macro("MEMBERS")
#        define maf_restore_macro_PROPERTIES
#    endif
#    ifdef ENDOBJECT
#        pragma push_macro("ENDOBJECT")
#        define maf_restore_macro_END_OBJECT
#    endif

#define OBJECT mc_maf_tuple_like_object

#define ENDOBJECT mc_maf_tuple_like_object_end

#define MEMBERS mc_maf_properties_map

