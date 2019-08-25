#include <memory>
#include <chrono>
#include <iostream>
#include <string>
#include <functional>


#define MAF_ENABLE_DUMP
#define MAF_USING_JSON11
#include "json11.hpp"
#include <maf/utils/serialization/3rdparty/Json11Trait.h>
#include <maf/messaging/MsgDefHelper.mc.h>



mc_sbClass(Header)
mc_sbProperties((std::string, index), (std::string, name))
mc_sbClass_end(Header)

using HeaderMap = std::map<std::string, Header>;
using SpecialMap =  std::map<std::string, std::map<std::string, std::vector<std::set<int64_t>>>>;
using IntIntMap = std::map<int, int>;
mc_dcl_msg_start(SecurityScanRequestMsg6)
    mc_dcl_msg_props
    (
        (int64_t, index, 100),
        (std::string, name),
        (json11::Json, myjsonlike),
        (HeaderMap, headers),
        (std::vector<std::string>, custom_list),
        (double, double_value),
        (SpecialMap, special_map)
    )
mc_dcl_msg_end(SecurityScanRequestMsg6)

#include <fstream>
static const std::string datapath = "/home/nocpes/Desktop/data.dat";
#include <maf/utils/TimeMeasurement.h>
#include <bitset>

int main()
{
    maf::util::TimeMeasurement tm{[](auto totalTime){ std::cout << "total time = " << totalTime << std::endl;}};

    using JTrait = maf::srz::JsonTrait<json11::Json>;
    using Json = JTrait::Json;
    using JObject = Json::object;
    using JArray = Json::array;

    SecurityScanRequestMsg6 s;
    s.special_map()["hello"]["bello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["hello"]["cello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["hello"]["dello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["kello"]["bello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["kello"]["cello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["kello"]["dello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["hello"]["bello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["hello"]["cello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["hello"]["dello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["kello"]["bello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["kello"]["cello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["kello"]["dello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["hello"]["bello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["hello"]["cello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["hello"]["dello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["kello"]["bello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["kello"]["cello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    s.special_map()["kello"]["dello"].push_back({1, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3, 4});
    const std::string dump = s.dump();
    s = {};
    std::cout << "After clean: " << s.dump() << std::endl;
    s.load_from_json<Json>(dump);
    std::cout << "After load: " << s.dump() << std::endl;
    std::string strOut;
    maf::srz::DumpHelper<std::vector<std::set<int64_t>>>::dump(s.special_map()["kello"]["bello"], 0, strOut);
    std::cout << strOut << std::endl;
    return 0;
} // UB: double-delete
