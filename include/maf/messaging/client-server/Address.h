#pragma once

#include <maf/export/MafExport_global.h>
#include <maf/utils/serialization/Tuplizable.h>

#include <string>

namespace maf {
namespace messaging {

class Address : public srz::Tuplizable {
 public:
  using Port = int;
  using Name = std::string;
  using NameConstant = const char*;

  static constexpr Port INVALID_PORT = static_cast<Port>(-1);
  static constexpr NameConstant INVALID_NAME = "";

  Address() = default;
  Address(const Address&) = default;
  Address(Address&&) = default;
  Address& operator=(const Address&) = default;
  Address& operator=(Address&&) = default;

  MAF_DECL_EXPORT Address(Name name, Port port = INVALID_PORT);
  MAF_DECL_EXPORT bool operator==(const Address& other) const;
  MAF_DECL_EXPORT bool operator!=(const Address& other) const;
  MAF_DECL_EXPORT bool operator<(const Address& other) const;

  MAF_DECL_EXPORT bool valid() const;

  MAF_DECL_EXPORT const Port& get_port() const;
  MAF_DECL_EXPORT Port& get_port();
  MAF_DECL_EXPORT const Name& get_name() const;
  MAF_DECL_EXPORT Name& get_name();

  decltype(auto) as_tuple() { return std::tie(name_, port_); }
  decltype(auto) as_tuple() const { return std::tie(name_, port_); }

  MAF_DECL_EXPORT void dump(int indent, std::string& out) const;
  MAF_DECL_EXPORT std::string dump(int indent = -1) const;

 private:
  Name name_ = INVALID_NAME;
  Port port_ = INVALID_PORT;
};

}  // namespace messaging
}  // namespace maf
