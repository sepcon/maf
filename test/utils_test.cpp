#include <maf/threading/AtomicObject.h>
#include <maf/threading/MutexRef.h>
#include <maf/utils/cppextension/AggregateCompare.h>
#include <maf/utils/cppextension/TypeTraits.h>
#include <maf/utils/serialization/AggregateDump.h>
#include <maf/utils/serialization/Dumper.h>

#include <mutex>

#define CATCH_CONFIG_MAIN

#include "catch/catch_amalgamated.hpp"
namespace maf {
using namespace nstl;

struct A {
  int a = 0;
  int b = 0;
};

struct B {
  A a;
  std::string str;
  std::vector<int> vec;
};

template <class Ostream, class T>
Ostream& operator<<(Ostream& os, T&& b) {
  maf::srz::dump(os, b, -1);
  return os;
}

MC_MAF_MAKE_COMPARABLE(A)
MC_MAF_MAKE_COMPARABLE(B)

TEST_CASE("aggregation_compare") {
  B first, second;
  REQUIRE(first == second);
  first.a.a = 100;
  REQUIRE(first != second);
}

TEST_CASE("typetraits_test") {
  typedef void (*function_ptr)(int, int);
  int* pointer;

  REQUIRE(is_fnc_ptr_v<function_ptr>);
  REQUIRE(is_fnc_ptr_v<decltype(pointer)> == false);
}

TEST_CASE("AtomicObject_test") {
  using namespace maf::threading;
  struct MutexFake {
    bool locked = false;
    void lock() { locked = true; }
    void unlock() { locked = false; }
    void try_lock() {

    }
  };

  using AtomicString = AtomicObject<std::string, MutexReference<std::mutex>>;
  using AtomicStringPtr =
      AtomicObject<std::unique_ptr<std::string>, MutexReference<std::mutex>>;
  std::mutex m;

  AtomicString aString{"hello", m};
  AtomicString aString1{"hello", m};
  aString->append("world");
  aString1->append("world");
  REQUIRE(aString.lockee() == aString1.lockee());

  AtomicStringPtr sptr(m);

  sptr = std::make_unique<std::string>();
  sptr->append("hello world");
}

}  // namespace maf
