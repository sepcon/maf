#include <maf/messaging/client-server/CSContentError.h>

namespace maf {
namespace messaging {

CSContentError::CSContentError(Description desc, int32_t code) :
    _description(std::move(desc)),
    _code(std::move(code))
{
    setType(Type::Error);
}


bool CSContentError::equal(const CSMessageContentBase *other) const
{
    if(!other && (other->type() == Type::Error))
    {
        auto otherError = static_cast<const CSContentError*>(other);
        return (_code == otherError->_code) &&
               (_description == otherError->_description);
    }

    return false;
}

CSMessageContentBase *CSContentError::clone() const
{
    return new CSContentError{description(), code()};
}

const CSContentError::Description &CSContentError::description() const
{
    return _description;
}

void CSContentError::setDescription(Description description)
{
    _description = std::move(description);
}

CSContentError::ErrorCode CSContentError::code() const
{
    return _code;
}

void CSContentError::setCode(ErrorCode code)
{
    _code = std::move(code);
}


} // messaging
} // maf
