#include <maf/messaging/client-server/Address.h>
#include <maf/utils/serialization/Dumper.h>

namespace maf {
namespace messaging {

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

std::string Address::dump(int indent) const {
  using namespace srz;
  std::ostringstream os;
  writeIndent(os, indent);
  os << R"({ "name": )";
  srz::dump(os, get_name(), indent);
  os << R"(, "port": )";
  srz::dump(os, get_port(), indent);
  os << "}";
  return os.str();
}

}  // namespace messaging
}  // namespace maf
