#include <maf/messaging/MessageBase.h>

namespace maf {
namespace messaging {

MessageBase::~MessageBase() = default;

MessageBase::Type MessageBase::id() const { return typeid (*this); }

int MessageBase::priority() const { return _priority; }

void MessageBase::setPriority(int p) { _priority = p; }

}
}
