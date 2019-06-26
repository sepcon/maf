//#define SERIALIZATION_NUMBER_USING_STRING
#include "headers/Libs/Utils/Serialization/SerializableObject.h"
#include <chrono>
#include <iostream>
#include <fstream>

using namespace thaf::srz;
using namespace std::chrono;

enum FunctionID
{
    SayHello = 1,
    SayGoodBye = 100
};

tfmc_serializable_object(SubOject)
tfmc_properties
    (
        (std::wstring, Properties),
        (uint32_t, Index)
    )
tfmc_serializable_object_end(SubOject)

tfmc_serializable_object(TheObject)
using TEntryMap = std::map<int, std::string>;
    tfmc_properties
    (
        ( std::wstring, Name ),
        ( FunctionID, FuncID ),
        ( std::shared_ptr<std::wstring>, Action ),
        ( double, Address ),
        ( SubOject, Sub ),
        ( std::vector<TEntryMap>, ListOfIndex ),
        ( TEntryMap, EntryMap )
    )
tfmc_serializable_object_end(TheObject)


void thaf_srz_runTest()
{
    const std::string dest = "D:\\binary.dat"; //"/home/sepcon/Desktop/binary.dat"; //
    auto startTime = system_clock::now();
    try
    {
//        std::ofstream writer(dest, std::ios::binary);
//        StreamSerializer sr(writer);
//        for(int i = 0; i < 20000; ++i)
//        {
//            TheObject obj = {
//                L"Hello world",
//                SayHello,
//                std::make_shared<std::wstring>(L"shared pointer"),
//                1000.0,
//                SubOject {
//                    L"This is sub object",
//                    0
//                },
//                std::vector<TheObject::TEntryMap>{
//                    TheObject::TEntryMap {
//                        { 1, "Hello"},
//                        { 2, "Hallu"},
//                        { 3, "World"}
//                    },
//                    TheObject::TEntryMap {
//                        { 1, "Hello"},
//                        { 2, "Hallu"},
//                        { 3, "World"}
//                    },
//                    TheObject::TEntryMap {
//                        { 1, "Hello"},
//                        { 2, "Hallu"},
//                        { 3, "World"}
//                    }
//                },
//                TheObject::TEntryMap {
//                    { 1, "Hello"},
//                    { 2, "Hallu"},
//                    { 3, "World"}
//                }
//            };
//            std::cout  << " Written " << i + 1 << std::endl;
//            sr << obj;
//        }

//        sr.flush();

        std::ifstream reader(dest, std::ios_base::binary);
        StreamDeserializer dsrz(reader, 500);
        int i = 0;

        while(!dsrz.exhausted())
        {
            ++i;
            TheObject to1;
            dsrz >> to1;
            std::cout << i << ".   " <<  to1.name() << to1.dump(2) << std::endl;
        }
        std::cout << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "Dammaged Serializtion data with exception: " << e.what() << std::endl;
    }

    std::cout << "total time: " << duration_cast<milliseconds>(system_clock::now() - startTime).count() << std::endl;
}
