#pragma once

#include "Serializer.h"
#include "DumpHelper.h"
#include "Tplkdef.mc.h"

/**
 * @brief Use below MACROS to define a serializable object like this:
 *
 * mcsb_class(TheObject)
 *     mc_tpl_properties
 *     (
 *          (std::string, Name),
 *          (int, Type),
 *          (std::string, Action),
 *          (Type, Name) //( <Type, Name> must be enclosed by parentheses)
 *     )
 * mc_tpl_class_end(TheObject)
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
 *  std::cout << object.getName() << object.getType() << object.getAction();
 *  printVector(vec) ...;
 *
 * */


#define mc_tpl_class(ClassName) mc_tuple_like_object_(ClassName)

#define mc_tpl_class_end(ClassName) mc_tuple_like_object_end_(ClassName)

#define mc_tpl_class_hasbase(ClassName, BaseClassName) mc_tuple_like_object_has_base_(ClassName, BaseClassName)

#define mc_tpl_class_hasbase_end(ClassName) mc_tuple_like_object_has_base_end_(ClassName)

#define mc_tpl_properties(...) mc_properties_map_(__VA_ARGS__)

