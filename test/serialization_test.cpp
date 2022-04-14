#include <maf/utils/cppextension/AggregateCompare.h>
#include <maf/utils/serialization/AggregateDump.h>
#include <maf/utils/serialization/VariantSerializer.h>

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#define CATCH_CONFIG_MAIN
#include "catch/catch_amalgamated.hpp"

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

struct Address {
  std::string house_no;
  std::string street;
  std::string dist;
};

MC_MAF_MAKE_COMPARABLE(Address);
OBJECT(StudentInfo)
  MEMBERS
  (
    (Address, members),
    (int, age, 0)
  )
ENDOBJECT(StudentInfo)

#include <maf/utils/serialization/SerializableObjectEnd.mc.h>
// clang-format on

enum class TheEnum : char { Val0, Val1, Val2, ValN, DefauLtVal };

static ComplexObject ocomplex = {
    1001001,
    "hell world",
    HeaderMap{{"header", Header{/*Temp{},*/ "index", "name"}}},
    std::vector<std::string>{"one", "two", "three", "..."},
    100.00,
    SpecialMap{{"map", {{"vector_of_set", {{1, 2, 3, 4}, {2, 3, 4, 4}}}}}}};

static int oi = 1000;
static long ol = 10000;
static float of = 200.101f;
static double od = 3000.202;
static std::string os = "hello";
static std::vector<int> ovi = {1, 2, 3};
static std::set<int> oseti = {1, 2, 3};
static std::map<std::string, std::vector<int>> omsvi = {{os, ovi}};
static std::map<std::set<int>, std::set<int>> omsseti = {{oseti, oseti}};
static std::shared_ptr<std::string> osptr =
    std::make_shared<std::string>("Hello world");
static std::string* orptr = new std::string{"hello world"};
static std::unique_ptr<std::string> ouptr{new std::string{"hello world"}};
static TheEnum oenum = TheEnum::Val1;

template <class OStream, class IStream, typename IOConnector>
static void serializationTest(OStream& theostream, IStream& theistream,
                              IOConnector ioconnect) {
  maf::srz::SR srz{theostream};
  maf::srz::DSR dsrz{theistream};

  srz << ocomplex << oi << ol << of << od << os << ovi << oseti << omsvi
      << osptr << orptr << ouptr << oenum;

  // flush the ostream and get fetch the data to istream
  ioconnect(theostream, theistream);

  ComplexObject dcomplex;
  int di = -1;
  long dl;
  float df;
  double dd;
  std::string ds;
  std::vector<int> dvi;
  std::set<int> dseti;
  std::map<std::string, std::vector<int>> dmsvi;
  std::map<std::set<int>, std::set<int>> dmsseti;
  std::shared_ptr<std::string> dsptr;
  std::string* drptr = nullptr;
  std::unique_ptr<std::string> duptr;
  TheEnum denum = TheEnum::DefauLtVal;

  dsrz >> dcomplex >> di >> dl >> df >> dd >> ds >> dvi >> dseti >> dmsvi >>
      dsptr >> drptr >> duptr >> denum;

  // clang-format off
  SECTION("stream_serialization_all_types")
  {
    REQUIRE(ocomplex  == dcomplex );
    REQUIRE(oi  == di );
    REQUIRE(ol  == dl );
    REQUIRE(of  == df );
    REQUIRE(od  == dd );
    REQUIRE(os  == ds );
    REQUIRE(ovi  == dvi );
    REQUIRE(oseti  == dseti );
    REQUIRE(omsvi  == dmsvi );
    REQUIRE(*osptr  == *dsptr );
    REQUIRE(*orptr == *drptr);
    REQUIRE(*ouptr == *duptr);
    REQUIRE(oenum == denum);

    REQUIRE( maf::srz::toString(ocomplex) == maf::srz::toString(dcomplex) );
    REQUIRE( maf::srz::toString(oi) == maf::srz::toString(di) );
    REQUIRE( maf::srz::toString(ol) == maf::srz::toString(dl) );
    REQUIRE( maf::srz::toString(of) == maf::srz::toString(df) );
    REQUIRE( maf::srz::toString(od) == maf::srz::toString(dd) );
    REQUIRE( maf::srz::toString(os) == maf::srz::toString(ds) );
    REQUIRE( maf::srz::toString(ovi) == maf::srz::toString(dvi) );
    REQUIRE( maf::srz::toString(oseti) == maf::srz::toString(dseti) );
    REQUIRE( maf::srz::toString(omsvi) == maf::srz::toString(dmsvi) );
    REQUIRE( maf::srz::toString(*osptr) == maf::srz::toString(*dsptr) );
    REQUIRE( maf::srz::toString(*orptr) == maf::srz::toString(*drptr) );
    REQUIRE( maf::srz::toString(oenum) == maf::srz::toString(denum) );
  }
  // clang-format on
}

TEST_CASE("file stream serialization test", "[FileSerialization]") {
  std::string path = "helloworld.dat";
  std::ofstream ofs{path, std::ios_base::binary};
  std::ifstream ifs;
  auto ioconnect = [&path](std::ofstream& ofs, std::ifstream& ifs) {
    ofs.close();
    ifs.open(path, std::ios_base::in | std::ios_base::binary);
  };
  serializationTest(ofs, ifs, ioconnect);
}

TEST_CASE("string stream serialization test", "[InmemorySerialization]") {
  std::istringstream iss;
  std::ostringstream oss;
  auto ioconnect = [](std::ostringstream& oss, std::istringstream& iss) {
    oss.flush();
    iss.str(oss.str());
  };
  serializationTest(oss, iss, ioconnect);
}

TEST_CASE("Dumper", "[Dumper]") {
  // clang-format off
  SECTION("dump_helper_all_types")
  {
    REQUIRE(maf::srz::toString(oi)  == "1000");
    REQUIRE(maf::srz::toString(ol)  == "10000");
    REQUIRE(maf::srz::toString(of)  == "200.101");
    REQUIRE(maf::srz::toString(od)  == "3000.2");
    REQUIRE(maf::srz::toString(os)  == "hello");
    REQUIRE(maf::srz::toString(ovi)  == "[\n  1,\n  2,\n  3\n]");
    REQUIRE(maf::srz::toString(oseti)  == "[\n  1,\n  2,\n  3\n]");
    REQUIRE(maf::srz::toString(omsvi)  == "{\n  hello: [\n    1,\n    2,\n    3\n  ]\n}");
    REQUIRE(maf::srz::toString(omsseti)  == "{\n  [\n    1,\n    2,\n    3\n  ]: [\n    1,\n    2,\n    3\n  ]\n}");
    REQUIRE(maf::srz::toString(osptr)  == R"(Hello world)");
    REQUIRE(maf::srz::toString(orptr)  == R"(hello world)");
    REQUIRE(maf::srz::toString(ouptr)  == R"(hello world)");
    REQUIRE(maf::srz::toString(oenum)  == "1");
  }
  // clang-format on
}

namespace ns {

struct CustomType {
  std::string val;
};

MC_MAF_DEFINE_DUMP_FUNCTION(CustomType, val) {
  maf::srz::dump(os, val.val, indentLevel);
}

MC_MAF_DEFINE_SERIALIZE_FUNCTION(CustomType, val) {
  maf::srz::serialize(os, val.val);
}

MC_MAF_DEFINE_DESERIALIZE_FUNCTION(CustomType, val) {
  return maf::srz::deserialize(is, val.val);
}

}  // namespace ns

TEST_CASE("custom_object_test", "[CustomObject]") {
  std::string str = "hello world";
  REQUIRE(maf::srz::toString(ns::CustomType{str}, -1) ==
          maf::srz::toString(str));

  ns::CustomType before{"hello world"};
  ns::CustomType after{"hello world"};

  std::stringstream ss;
  maf::srz::SR sr(ss);
  maf::srz::DSR dsr(ss);

  sr << before;
  dsr >> after;

  REQUIRE(before.val == after.val);
}

TEST_CASE("aggregate_serialization_test") {
  struct Aggregate {
    int int_ = 0;
    std::string string_ = "string";
    std::set<std::string> stringSet_ = {"set_of_string", "set_of_string1"};
    struct SubAggregate {
      std::vector<int> vec_ = {1, 2, 3};
      bool bool_ = true;
    } subAggregate_;
  };

  Aggregate a;

  REQUIRE(maf::srz::toString(a, -1) ==
          "[0,string,[set_of_string,set_of_string1],[[1,2,3],true]]");

  std::stringstream ss;
  maf::srz::SR sr(ss);
  maf::srz::DSR dsr(ss);

  Aggregate sbefore, safter;
  sbefore.subAggregate_.vec_ = {2, 3, 4, 5, 6, 7};
  safter.subAggregate_.vec_.clear();
  sr << sbefore;
  dsr >> safter;

  REQUIRE(maf::srz::toString(sbefore) == maf::srz::toString(safter));
  StudentInfo inf{Address{"house.no", "street", "dist"}, 10}, ninf;

  ss.str("");
  sr << inf;
  dsr >> ninf;
  REQUIRE(inf == ninf);
}

TEST_CASE("variant_serialization") {
  using Var =
      std::variant<std::string, int, bool, double, std::vector<int>, Address>;
  Var varInt = 1;
  Var varStr = "helloworld";
  Var varBool = true;
  Var varVec = std::vector<int>{1, 2, 3, 4};
  Var varComplex = Address{"1", "2", "3"};
  std::stringstream ss;
  maf::srz::serializeBatch(ss, varInt, varStr, varBool, varVec, varComplex);
  Var varInt1;
  Var varStr1;
  Var varBool1;
  Var varVec1;
  Var varComplex1;
  maf::srz::deserializeBatch(ss, varInt1, varStr1, varBool1, varVec1,
                             varComplex1);

  REQUIRE(varInt == varInt1);
  REQUIRE(varStr == varStr1);
  REQUIRE(varBool == varBool1);
  REQUIRE(varVec == varVec1);
  REQUIRE(varComplex1 == varComplex);
}
