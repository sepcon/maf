#include <fstream>
#include <iostream>
#include <maf/utils/serialization/SerializableObjectBegin.mc.h>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

OBJECT(Header)
MEMBERS((std::string, index), (std::string, name))
ENDOBJECT(Header)

using HeaderMap = std::map<std::string, Header>;
using SpecialMap =
    std::map<std::string,
             std::map<std::string, std::vector<std::set<int64_t>>>>;
using IntIntMap = std::map<int, int>;

OBJECT(SecurityScanRequestMsg)
MEMBERS((int64_t, index, 100), (std::string, name), (HeaderMap, headers),
        (std::vector<std::string>, custom_list), (double, double_value),
        (SpecialMap, special_map))
ENDOBJECT(SecurityScanRequestMsg)

#include <maf/utils/serialization/SerializableObjectEnd.mc.h>

template <class OStream> static void serializeTest(OStream &os) {
  maf::srz::SR<OStream> srz{os};
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

template <class IStream> static void deserializeTest(IStream &is) {
  maf::srz::DSR<IStream> dsrz{is};
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

static void testStringStreamSerializer() {
  std::ostringstream oss;
  serializeTest(oss);
  std::istringstream is{oss.str()};
  deserializeTest(is);
}

static void testFileStreamSerializer() {
  std::string path = "helloworld.dat";
  std::ofstream ofs(path);
  serializeTest(ofs);
  ofs.close();

  std::ifstream ifs(path);
  deserializeTest(ifs);
}

#include <maf/utils/serialization/OByteStream.h>
#include <maf/utils/serialization/IByteStream.h>
#include <maf/utils/TimeMeasurement.h>

int main() {
//  testStringStreamSerializer();
//  testFileStreamSerializer();

    using namespace maf::srz;
    using namespace maf::util;
    std::vector<std::string> vec = {"hello world", "hello world", "hello world", "hello world", "hello world"};
    for(int i = 0; i < 100000; ++i)
        vec.push_back("hallu world");
    int x = 100;
    double d = 10000;
    std::string s = "hallu";
    std::wstring ws = L"hallu";

    OByteStream obs;
    {
        TimeMeasurement tm{[](auto elapsed) {
                std::cout << "time to serialize = " << elapsed.count() << std::endl;
            }};
        SR<OByteStream> sr{obs};
        for(int i = 0; i < 100; ++i)
        {
            sr.serializeBatch(vec, d, x, s, ws);
        }
    }

    {
        TimeMeasurement tm{[](auto elapsed) {
                std::cout << "time to serialize = " << elapsed.count() << std::endl;
            }};

        IByteStream is{obs.bytes()};
        DSR<IByteStream> sr{is};
        for(int i = 0; i < 100; ++i)
        {
            sr >> vec >> d >> x >> s >> ws;
        }
    }

    {
        TimeMeasurement tm{[](auto elapsed) {
                std::cout << "time to serialize = " << elapsed.count() << std::endl;
            }};
        IByteStream is{obs.bytes()};
        DSR<IByteStream> ds{is};

        for(int i = 0; i < 100; ++i)
        {
            ds.deserializeBatch(vec, d, x, s, ws);
        }
    }

  return 0;
}
