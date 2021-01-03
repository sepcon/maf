#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "test.h"

// clang-format off
#include <maf/utils/serialization/SerializableObjectBegin.mc.h>

OBJECT(Header)
  MEMBERS
  (
    (std::string, index),
    (std::string, name)
  )
ENDOBJECT(Header)

using HeaderMap = std::map<std::string, Header>;
using SpecialMap =
    std::map<std::string,
             std::map<std::string, std::vector<std::set<int64_t>>>>;
using IntIntMap = std::map<int, int>;

OBJECT(ComplexObject)
  MEMBERS
  (
    (int64_t, index, 100),
    (std::string, name),
    (HeaderMap, headers),
    (std::vector<std::string>, custom_list),
    (double, double_value),
    (SpecialMap, special_map)
  )
ENDOBJECT(ComplexObject)

#include <maf/utils/serialization/SerializableObjectEnd.mc.h>
// clang-format on

enum class TheEnum : char { Val0, Val1, Val2, ValN, DefauLtVal };
class AbstractClass {
  virtual void abstractFunction() = 0;
  virtual ~AbstractClass() = default;
};

static ComplexObject ocomplex = {
    1001001,
    "hell world",
    HeaderMap{{"header", Header{"index", "name"}}},
    std::vector<std::string>{"one", "two", "three", "..."},
    100.00,
    SpecialMap{{"map", {{"vector_of_set", {{1, 2, 3, 4}, {2, 3, 4, 4}}}}}}};

static AbstractClass *abstract = nullptr;
static int oi = 1000;
static long ol = 10000;
static float of = 200;
static double od = 3000;
static std::string os = "hello";
static std::vector<int> ovi = {1, 2, 3};
static std::set<int> oseti = {1, 2, 3};
static std::map<std::string, std::vector<int>> omsvi = {{os, ovi}};
static std::map<std::set<int>, std::set<int>> omsseti = {{oseti, oseti}};
static std::shared_ptr<std::string> osptr =
    std::make_shared<std::string>("Hello world");
static std::string *orptr = new std::string{"hello world"};
static std::unique_ptr<std::string> ouptr{new std::string{"hello world"}};
static TheEnum oenum = TheEnum::Val1;

template <class OStream, class IStream, typename IOConnector>
static void serializationTest(OStream &theostream, IStream &theistream,
                              IOConnector ioconnect) {
  maf::srz::SR srz{theostream};
  maf::srz::DSR dsrz{theistream};

  srz << ocomplex << oi << ol << of << od << os << ovi << oseti << omsvi
      << osptr << orptr << ouptr << oenum;

  // flush the ostream and get fetch the data to istream
  ioconnect(theostream, theistream);

  ComplexObject dcomplex;
  int di;
  long dl;
  float df;
  double dd;
  std::string ds;
  std::vector<int> dvi;
  std::set<int> dseti;
  std::map<std::string, std::vector<int>> dmsvi;
  std::map<std::set<int>, std::set<int>> dmsseti;
  std::shared_ptr<std::string> dsptr;
  std::string *drptr = nullptr;
  std::unique_ptr<std::string> duptr;
  TheEnum denum = TheEnum::DefauLtVal;

  dsrz >> dcomplex >> di >> dl >> df >> dd >> ds >> dvi >> dseti >> dmsvi >>
      dsptr >> drptr >> duptr >> denum;

  // clang-format off
  TEST_CASE_B(stream_serialization_all_types)
  {
    EXPECT(ocomplex  == dcomplex );
    EXPECT(oi  == di );
    EXPECT(ol  == dl );
    EXPECT(of  == df );
    EXPECT(od  == dd );
    EXPECT(os  == ds );
    EXPECT(ovi  == dvi );
    EXPECT(oseti  == dseti );
    EXPECT(omsvi  == dmsvi );
    EXPECT(*osptr  == *dsptr );
    EXPECT(*orptr == *drptr);
    EXPECT(*ouptr == *duptr);
    EXPECT(oenum == denum);

    EXPECT( maf::srz::dump(ocomplex) == maf::srz::dump(dcomplex) );
    EXPECT( maf::srz::dump(oi) == maf::srz::dump(di) );
    EXPECT( maf::srz::dump(ol) == maf::srz::dump(dl) );
    EXPECT( maf::srz::dump(of) == maf::srz::dump(df) );
    EXPECT( maf::srz::dump(od) == maf::srz::dump(dd) );
    EXPECT( maf::srz::dump(os) == maf::srz::dump(ds) );
    EXPECT( maf::srz::dump(ovi) == maf::srz::dump(dvi) );
    EXPECT( maf::srz::dump(oseti) == maf::srz::dump(dseti) );
    EXPECT( maf::srz::dump(omsvi) == maf::srz::dump(dmsvi) );
    EXPECT( maf::srz::dump(*osptr) == maf::srz::dump(*dsptr) );
    EXPECT( maf::srz::dump(*orptr) == maf::srz::dump(*drptr) );
    EXPECT( maf::srz::dump(oenum) == maf::srz::dump(denum) );
  } TEST_CASE_E()
  // clang-format on
}

static void fileStreamSerializationTest() {
  std::string path = "helloworld.dat";
  std::ofstream ofs{path, std::ios_base::binary};
  std::ifstream ifs;
  auto ioconnect = [&path](std::ofstream &ofs, std::ifstream &ifs) {
    ofs.close();
    ifs.open(path, std::ios_base::in | std::ios_base::binary);
  };
  serializationTest(ofs, ifs, ioconnect);
}

static void stringStreamSerializationTest() {
  std::istringstream iss;
  std::ostringstream oss;
  auto ioconnect = [](std::ostringstream &oss, std::istringstream &iss) {
    oss.flush();
    iss.str(oss.str());
  };
  serializationTest(oss, iss, ioconnect);
}

void dumpheperTest() {
  // clang-format off
  TEST_CASE_B(dump_helper_all_types)
  {
    EXPECT(maf::srz::dump(oi)  == "1000");
    EXPECT(maf::srz::dump(ol)  == "10000");
    EXPECT(maf::srz::dump(of)  == "200.000000");
    EXPECT(maf::srz::dump(od)  == "3000.000000");
    EXPECT(maf::srz::dump(os)  == R"("hello")");
    EXPECT(maf::srz::dump(ovi)  == "[\n  1,\n  2,\n  3\n]");
    EXPECT(maf::srz::dump(oseti)  == "[\n  1,\n  2,\n  3\n]");
    EXPECT(maf::srz::dump(omsvi)  == "{\n  \"hello\": [\n    1,\n    2,\n    3\n  ]\n}");
    EXPECT(maf::srz::dump(omsseti)  == "{\n  [\n    1,\n    2,\n    3\n  ]: [\n    1,\n    2,\n    3\n  ]\n}");
    EXPECT(maf::srz::dump(osptr)  == R"("Hello world")");
    EXPECT(maf::srz::dump(orptr)  == R"("hello world")");
    EXPECT(maf::srz::dump(ouptr)  == R"("hello world")");
    EXPECT(maf::srz::dump(oenum)  == "1");
    EXPECT(maf::srz::dump(abstract)  == maf::srz::dump(std::string{typeid(AbstractClass*).name()}));

  }
TEST_CASE_E(dump_helper_all_types)
  // clang-format on
}

#include <thread>
int main() {
  maf::test::init_test_cases();

  fileStreamSerializationTest();
  stringStreamSerializationTest();
  dumpheperTest();

  return 0;
}
