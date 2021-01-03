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
 *  maf::srz::Buffer ba = readFromFile/FromPipe();
 *  maf::srz::BADeserializer dsr(ba);
 *  TheObject object;
 *  std::vector<std::string> vec;
 *  dsr >> object >> vec; //The order must be kept same as serialization
 *
 *  std::cout << object.name() << object.type() << object.action();
 *  printVector(vec) ...;
 *
 * */

#ifndef MAF_UTILS_SERIALIZATION_MAFOBJECTBEGIN_MC_H
#define MAF_UTILS_SERIALIZATION_MAFOBJECTBEGIN_MC_H
#   include "Serializer.h"
#   include "DumpHelper.h"
#endif // MAF_UTILS_SERIALIZATION_MAFOBJECTBEGIN_MC_H

// The rest of this file must be putted outside include guard
// Make it to be use with multiple files
#include "Internal/SerializableObjectImpl.mc.h"

# ifdef OBJECT
#     pragma push_macro("OBJECT")
#     define maf_restore_macro_OBJECT
# endif
# ifdef OBJECT_EX
#     pragma push_macro("OBJECT_EX")
#     define maf_restore_macro_OBJECT_EX
# endif
# ifdef MEMBERS
#     pragma push_macro("MEMBERS")
#     define maf_restore_macro_MEMBERS
# endif
# ifdef ENDOBJECT
#     pragma push_macro("ENDOBJECT")
#     define maf_restore_macro_END_OBJECT
# endif

#define OBJECT mc_maf_sb_object
#define OBJECT_EX mc_maf_sb_object_ex
#define ENDOBJECT mc_maf_sb_endobject
#define MEMBERS mc_maf_sb_members
