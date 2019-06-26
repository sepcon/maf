#ifndef SERIALIZABLEOBJECT_H
#define SERIALIZABLEOBJECT_H

#include "Serializer.h"
#include "DumpHelper.h"
#include "SerializableObjMacros.h"

/**
 * @brief Use below MACROS to define a serializable object like this:
 *
 * mcsb_class(TheObject)
 *     mc_sbProperties
 *     (
 *          (std::string, Name),
 *          (int, Type),
 *          (std::string, Action),
 *          (Type, Name) //( <Type, Name> must be enclosed by parentheses)
 *     )
 * mc_sbClass_end(TheObject)
 *
 * After that, you can use it for serialization like this:
 *
 *  int type = 10;
 *  TheObject object("name", type, "Pull request action");
 *  std::vector<std::string> vec = {....};
 *
 *  thaf::srz::BASerializer sr;
 *  sr << object << vec; // done serialize
 *
 *  //... write sr.c_str() to file, share it between programs
 *  //... now deserialize it
 *
 *  thaf::srz::ByteArray ba = readFromFile/FromPipe();
 *  thaf::srz::BADeserializer dsr(ba);
 *  TheObject object;
 *  std::vector<std::string> vec;
 *  dsr >> object >> vec; //The order must be kept same as serialization
 *
 *  std::cout << object.getName() << object.getType() << object.getAction();
 *  printVector(vec) ...;
 *
 * */


#define mc_sbClass(ClassName) mc_serializable_object_(ClassName)

#define mc_sbClass_end(ClassName) mc_serializable_object_end_(ClassName)

#define mc_sbProperties(...) mc_properties_map_(__VA_ARGS__)


#endif // SERIALIZABLEOBJECT_H
