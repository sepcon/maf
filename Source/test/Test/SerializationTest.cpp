//#define SERIALIZATION_NUMBER_USING_STRING
#include "thaf/Utils/Serialization/SerializableObject.h"
#include "thaf/Utils/Debugging/Debug.h"
#include <chrono>
#include <fstream>

using namespace thaf::srz;
using namespace std::chrono;

enum FunctionID
{
    SayHello = 1,
    SayGoodBye = 100
};

mc_sbClass(SubOject)
mc_sbProperties
    (
        (std::string, Properties),
        (uint32_t, Index)
    )
mc_sbClass_end(SubOject)



mc_sbClass(TheObject)
    using JsonObject = std::map<std::string, std::string>;
    enum class PhoneNumberType : char
    {
        Home,
        Work,
        Mobile
    };

    mc_sbProperties
    (
            (bool, Enabled, false),
            (std::string, Name, "sdfsdfsdf"),
            (uint32_t, Index, 0),
            (JsonObject, Json),
            (PhoneNumberType, NumberType, PhoneNumberType::Work),
            (SubOject, Sub)
    )

mc_sbClass_end(TheObject)

///
///

void serializeTest(Serializer& sr)
{
    SubOject sub{ "hallu", 100 };
    for(int i = 0; i < 100; ++i)
    {

        TheObject obj = {
                true,
                "Hello world",
                i,
                TheObject::JsonObject {
                    { "Hello", "Hello"},
                    { "Hallu", "Hallu"},
                    { "World", "World"}
                },
                TheObject::PhoneNumberType::Home,
                SubOject{}
            };
        obj.set_Index(100);
        sr << obj;
    }
    sr.flush();
}

void deserializeTest(Deserializer& dsrz)
{
    int i = 0;
    while(!dsrz.exhausted())
    {
        ++i;
        TheObject to1;
        dsrz >> to1;
        thafInfo( i << ".   " <<  to1.class_name() << to1.dump(2) );
    }
    thafInfo("");
}

void thaf_srz_runTest()
{
#ifdef _WIN32
    const std::string dest = "D:\\binary.dat";
#elif linux
    const std::string dest = "/home/sepcon/Desktop/binary.dat";
#endif

    auto startTime = system_clock::now();
    try
    {
        std::ofstream writer(dest, std::ios::binary);
        StreamSerializer sr(writer);
        serializeTest(sr);

        std::ifstream reader(dest, std::ios_base::binary);
        StreamDeserializer dsrz(reader, 500);

        deserializeTest(dsrz);

    }
    catch (const std::exception& e)
    {
        thafErr( "Dammaged Serializtion data with exception: " << e.what() );
    }
    catch(std::ios::iostate state)
    {
        thafErr( "The stream is not in good state: " << state );
    }

    thafWarn( "total time: " << duration_cast<milliseconds>(system_clock::now() - startTime).count() );
}
