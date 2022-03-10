#pragma once

#include <maf/utils/cppextension/TupleManip.h>

namespace maf {
namespace srz {

MC_MAF_DEFINE_HAS_METHOD_CHECK(as_tuple)
MC_MAF_DEFINE_HAS_METHOD_CHECK(cas_tuple)

#define MC_MAF_GENERATE_AS_TUPLE_METHOD(...)                  \
 public:                                                      \
  decltype(auto) as_tuple() { return std::tie(__VA_ARGS__); } \
  decltype(auto) cas_tuple() const { return std::tie(__VA_ARGS__); }

#define MC_MAF_GENERATE_AS_TUPLE_METHOD_WITH_BASE_CLASS(Base, ...)   \
 public:                                                             \
  decltype(auto) as_tuple() {                                        \
    return std::tuple_cat(Base::as_tuple(), std::tie(__VA_ARGS__));  \
  }                                                                  \
  decltype(auto) cas_tuple() const {                                 \
    return std::tuple_cat(Base::cas_tuple(), std::tie(__VA_ARGS__)); \
  }

}  // namespace srz
}  // namespace maf
