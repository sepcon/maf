#include <memory>
#include <chrono>
#include <iostream>
#include <string>
#include <functional>

#include <map>
#include <set>

#define MAF_ENABLE_DUMP
#include <maf/utils/serialization/MafObjectBegin.mc.h>
OBJECT(Header)
PROPERTIES
    (
        (std::string, index),
        (std::string, name)
    )
ENDOBJECT(Header)

using HeaderMap = std::map<std::string, Header>;
using SpecialMap =  std::map<std::string, std::map<std::string, std::vector<std::set<int64_t>>>>;
using IntIntMap = std::map<int, int>;

OBJECT(SecurityScanRequestMsg)
    PROPERTIES
    (
        (int64_t, index, 100),
        (std::string, name),
        (HeaderMap, headers),
        (std::vector<std::string>, custom_list),
        (double, double_value),
        (SpecialMap, special_map)
    )
ENDOBJECT(SecurityScanRequestMsg)
#include <maf/utils/serialization/MafObjectEnd.mc.h>

#include <maf/utils/serialization/BASerializer.h>

int main()
{
    SecurityScanRequestMsg s;
    s.special_map()["hello"]["bello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["hello"]["cello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});

    std::cout << "s before cleared is: " << std::endl;
    std::cout << (s.dump()) << std::endl;
    maf::srz::BASerializer srz;
    srz << s;
    std::cout << srz.bytes() << std::endl;

    //clear s
    s = {};

    std::cout << "After cleared: " << s.dump() << std::endl;
    maf::srz::BADeserializer dsrz(srz.mutableBytes());

    dsrz >> s;

    std::cout << "After deserialization: " << std::endl;
    std::cout << s.dump() << std::endl;
    std::cout << s.dump(-1) << std::endl;

    std::map<std::string, std::vector<int>> mv = {
        {"1", {1, 2, 3}},
        {"2", {3, 2, 3}}
    };
    {
        std::string dumped;
        maf::srz::DumpHelper<decltype (mv)>::dump(mv, -1, dumped);
        std::cout << dumped << std::endl;

    }
    {
        std::string dumped;
        maf::srz::DumpHelper<decltype (mv)>::dump(mv, 0, dumped);
        std::cout << dumped << std::endl;

    }

    return 0;
}
