#ifndef SERIALIZABLEOBJECT_H
#define SERIALIZABLEOBJECT_H

#include "Serializer.h"
#include "SerializableObjMacros.h"

/**
 * @brief Use below MACROS to define a serializable object like this:
 *
 * tfmc_serializable_object(TheObject)
 *     tfmc_properties
 *     (
 *          (std::string, Name),
 *          (int, Type),
 *          (std::string, Action),
 *          (Type, Name) //( <Type, Name> must be enclosed by parentheses)
 *     )
 * tfmc_serializable_object_end(TheObject)
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


#define SB_OBJECT_S(ClassName) SERIALIZABLE_OBJECT_PRV_(ClassName)

#define SB_OBJECT_E(ClassName) SERIALIZABLE_OBJECT_END_PRV_(ClassName)

#define SB_PROPERTIES(...) PROPERTIES_MAP_(__VA_ARGS__)


#endif // SERIALIZABLEOBJECT_H
