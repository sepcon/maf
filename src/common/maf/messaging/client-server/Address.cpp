#include <maf/messaging/client-server/Address.h>
#include <maf/utils/serialization/DumpHelper.h>

namespace maf {
namespace messaging {

using namespace srz;

Address::Address(Address::Name name, Address::Port port)
    : name_(std::move(name)), port_(port) {}

bool Address::operator==(const Address &other) const {
  return name_ == other.name_ && port_ == other.port_;
}

bool Address::operator!=(const Address &other) const {
  return !(*this == other);
}

bool Address::operator<(const Address &other) const {
  return name_ < other.name_ || port_ < other.port_;
}

bool Address::valid() const {
  return (get_port() != INVALID_PORT) || (get_name() != INVALID_NAME);
}

const Address::Port &Address::get_port() const { return port_; }
Address::Port &Address::get_port() { return port_; }

const Address::Name &Address::get_name() const { return name_; }
Address::Name &Address::get_name() { return name_; }

void Address::dump(int indent, std::string &out) const {
  out += getIndent(indent) + R"({ "name": )";
  DumpHelper<Name>::dump(name_, indent, out);
  out += R"(, "port": )";
  DumpHelper<Port>::dump(port_, indent, out);
  out += "}";
}

std::string Address::dump(int indent) const {
  std::string out;
  dump(indent, out);
  return out;
}

}  // namespace messaging
}  // namespace maf
