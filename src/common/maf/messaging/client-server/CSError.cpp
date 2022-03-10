#include <maf/messaging/client-server/CSError.h>
#include <maf/utils/serialization/Dumper.h>

#include <sstream>

namespace maf {
namespace messaging {

struct CSErrorDataPrv {
  CSError::Description description;
  CSError::ErrorCode code;
};

CSError::CSError(Description desc, ErrorCode code)
    : d_{new CSErrorDataPrv{std::move(desc), code}} {}

CSError::~CSError() { delete d_; }

bool CSError::equal(const CSMsgPayloadIF *other) const {
  if (other && (other->type() == CSPayloadType::Error)) {
    auto otherError = static_cast<const CSError *>(other);
    return (otherError->d_->code == d_->code) &&
           (otherError->d_->description == d_->description);
  }
  return false;
}

CSMsgPayloadIF *CSError::clone() const {
  return new CSError{description(), code()};
}

const CSError::Description &CSError::description() const {
  return d_->description;
}

void CSError::setDescription(Description description) {
  d_->description = std::move(description);
}

CSError::ErrorCode CSError::code() const { return d_->code; }

void CSError::setCode(ErrorCode code) { d_->code = (std::move(code)); }

std::string CSError::dump(int /*level*/) const {
  std::ostringstream oss;
  dump(oss);
  return oss.str();
}

CSPayloadType CSError::type() const { return CSPayloadType::Error; }

void CSError::dump(std::ostream &os) const {
  using namespace maf::srz;
  auto ds = dumpstream(os);
  ds << "{\n\tcode: " << d_->code << "\n\tdescription: " << d_->description
     << "\n}";
}

}  // namespace messaging
}  // namespace maf
