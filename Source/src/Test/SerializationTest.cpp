#include "headers/Libs/Utils/Serialization/SerializableObject.h"
#include <chrono>
#include <iostream>
#include <fstream>

using namespace thaf::srz;
using namespace std::chrono;

enum class FunctionID : uint32_t
{
    SayHello = 1,
    SayGoodBye = 100
};

SB_OBJECT_S(TheObject)
    SB_OBJECT_S(SubOject)
        SB_PROPERTIES
        (
            (std::string, Properties),
            (uint32_t, Index)
        )
    SB_OBJECT_E(SubOject)
    SB_PROPERTIES
    (
        ( std::string, Name ),
        ( FunctionID, FuncID ),
        ( std::shared_ptr<std::string>, Action ),
        ( float, Address ),
        ( SubOject, Sub ),
        ( std::vector<uint32_t>, ListOfIndex )
    )
SB_OBJECT_E(TheObject)

SB_OBJECT_S(SA)
SB_PROPERTIES
(
        ( std::string, Name ),
        ( FunctionID, FuncID ),
        ( std::shared_ptr<std::string>, Action ),
        ( float, Address ),
        ( std::vector<uint32_t>, ListOfIndex ),
        ( std::shared_ptr<int>, SharedPtr )
)
SB_OBJECT_E(SA)

void thaf_srz_runTest()
{
    const std::string dest = "/home/sepcon/Desktop/binary.dat"; //"D:\\binary.dat"; //
    auto startTime = system_clock::now();
    try
    {

        std::ofstream writer(dest, std::ios::binary);
        StreamSerializer sr(writer);
        for(int i = 0; i < 100000; ++i)
        {
            TheObject::SubOject sub{"Helo", 10};
            TheObject obj; //{ "sdfsdf", FunctionID::SayHello, nullptr, "sdfafqer", SubOject{"Helo", 10} , std::vector<uint32_t>{} };
            obj.setName("Hello");
            obj.setListOfIndex({1, 2, 3});
            obj.setAddress(1000);
            std::string nestring = "Nguyen van con from ha noi";
            sr << nestring << obj;
        }

        sr.flush();

        std::ifstream reader(dest, std::ios_base::binary);
        StreamDeserializer dsrz(reader, 500);
        int i = 0;

        while(!dsrz.exhaust())
        {
            ++i;
            TheObject to1;
            std::string nestring1;
            dsrz >> nestring1 >> to1;
            std::cout << i << ". name = " << to1.getName()
                      << " type = " << static_cast<uint32_t>(to1.getFuncID())
                      << " action = " << (to1.getAction() ? *to1.getAction() : "nul")
                      << " address = " << to1.getAddress()
                      << " sub = " << to1.getSub().getProperties();
            for(const auto& i : to1.getListOfIndex())
            {
                std::cout << ", " << i ;
            }
            std::cout << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "Dammaged Serializtion data with exception: " << e.what() << std::endl;
    }

    std::cout << "total time: " << duration_cast<milliseconds>(system_clock::now() - startTime).count() << std::endl;
}
