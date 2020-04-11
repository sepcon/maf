#include <fstream>
#include <iostream>
#include <maf/utils/serialization/BASerializer.h>
#include <maf/utils/serialization/StreamSerializer.h>
#include <map>
#include <set>
#include <string>

#include <maf/utils/serialization/MafObjectBegin.mc.h>

OBJECT(Header)
    MEMBERS((std::string, index), (std::string, name))
ENDOBJECT(Header)

using HeaderMap = std::map<std::string, Header>;
using SpecialMap =
    std::map<std::string,
             std::map<std::string, std::vector<std::set<int64_t>>>>;
using IntIntMap = std::map<int, int>;

OBJECT(SecurityScanRequestMsg)
    MEMBERS
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

static void serializeTest(maf::srz::Serializer &srz) {
  SecurityScanRequestMsg s;
  s.get_special_map()["hello"]["bello"].push_back(
      {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4});
  s.get_special_map()["hello"]["cello"].push_back(
      {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4});

  std::cout << "s before cleared is: " << std::endl;
  std::cout << (s.dump()) << std::endl;
  srz << s;
  std::cout << "After cleared: " << s.dump() << std::endl;
}

static void deserializeTest(maf::srz::Deserializer &dsrz) {

  SecurityScanRequestMsg s;
  dsrz >> s;

  std::cout << "After deserialization: " << std::endl;
  std::cout << s.dump() << std::endl;
  std::cout << s.dump(-1) << std::endl;

  std::map<std::string, std::vector<int>> mv = {{"1", {1, 2, 3}},
                                                {"2", {3, 2, 3}}};
  {
    std::string dumped;
    maf::srz::DumpHelper<decltype(mv)>::dump(mv, -1, dumped);
    std::cout << dumped << std::endl;
  }
  {
    std::string dumped;
    maf::srz::DumpHelper<decltype(mv)>::dump(mv, 0, dumped);
    std::cout << dumped << std::endl;
  }
}

static void testBASerializer() {
  maf::srz::BASerializer srz;
  serializeTest(srz);
  maf::srz::BADeserializer dszr{srz.mutableBytes()};
  deserializeTest(dszr);
}

static void testStreamSerializer() {
  std::string path = "helloworld.dat";
  std::ofstream ofs(path);
  maf::srz::StreamSerializer srz(ofs);
  serializeTest(srz);
  srz.flush();

  std::ifstream ifs(path);
  maf::srz::StreamDeserializer dszr{ifs};
  deserializeTest(dszr);
}

int main() {
  testBASerializer();
  testStreamSerializer();
  return 0;
}
