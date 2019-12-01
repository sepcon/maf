#pragma once

#include "RegID.h"
#include "CSContentError.h"
#include <variant>

namespace maf {
namespace messaging {

class RegStatus
{
public:

private:
    std::variant<RegID, CSContentError> _d;
};
} // messaging
} // maf

