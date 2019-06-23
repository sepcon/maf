#ifndef SERIALIZABLEOBJECT_H
#define SERIALIZABLEOBJECT_H

#include "Serializer.h"
#include "SerializableObjMacros.h"

/**
 * @brief Use below MACROS to define a serializable object like this:
 *
 * SERIALIZABLE_OBJECT(TheObject)
 *     HAS_PROPERTIES(Name, Type, Action)
 *     WITH_RESPECTIVE_TYPES(std::string, int, std::string)
 *     DEFINE_GET_SET_METHODS(std::string, Name)
 *     DEFINE_GET_SET_METHODS(int, Type)
 *     DEFINE_GET_SET_METHODS(std::string, Action)
 * SERIALIZABLE_OBJECT_END(TheObject)
 *
 * After that, you can use it for serialization like this:
 *
 *  int type = 10;
 *  TheObject object("name", type, "Pull request action");
 *  std::vector<std::string> vec = {....};
 *
 *  thaf::srz::Serializer sr;
 *  sr << object << vec; // done serialize
 *
 *  //... write sr.c_str() to file, share it between programs
 *  //... now deserialize it
 *
 *  thaf::srz::ByteArray ba = readFromFile/FromPipe();
 *  thaf::srz::Deserializer dsr(ba);
 *  TheObject object;
 *  std::vector<std::string> vec;
 *  dsr >> object >> vec; //The order must be kept same as serialization
 *
 *  std::cout << object.getName() << object.getType() << object.getAction();
 *  printVector(vec) ...;
 * */


#define SERIALIZABLE_OBJECT(ClassName) SERIALIZABLE_OBJECT_PRV_(ClassName)

#define SERIALIZABLE_OBJECT_END(ClassName) SERIALIZABLE_OBJECT_END_PRV_(ClassName)

#define PROPERTIES_MAP(...) PROPERTIES_MAP_(__VA_ARGS__)


#endif // SERIALIZABLEOBJECT_H
