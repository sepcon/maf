#include <maf/messaging/client-server/CSError.h>
#include <maf/utils/serialization/MafObjectBegin.mc.h>
#include <sstream>

namespace maf {
namespace messaging {

struct CSErrorDataPrv {
  CSError::Description description;
  CSError::ErrorCode code;
};

CSError::CSError(Description desc, ErrorCode code)
    : d_{new CSErrorDataPrv{std::move(desc), code}} {
  setType(Type::Error);
}

CSError::~CSError() { delete d_; }

bool CSError::equal(const CSMessageContentBase *other) const {
  if (!other && (other->type() == Type::Error)) {
    auto otherError = static_cast<const CSError *>(other);
    return (otherError->d_->code == d_->code) &&
           (otherError->d_->description == d_->description);
  }
  return false;
}

CSMessageContentBase *CSError::clone() const {
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

std::string CSError::dump(int level) const {
  std::ostringstream oss;
  oss << "{\n\tcode: " << d_->code << "\n\tdescription: " << d_->description
      << "\n}";
  return oss.str();
}

} // namespace messaging
} // namespace maf
