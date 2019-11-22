#include <maf/messaging/CompMessageBase.h>

namespace maf {
namespace messaging {

CompMessageBase::~CompMessageBase() = default;

CompMessageBase::Type CompMessageBase::id() const { return typeid (*this); }

int CompMessageBase::priority() const { return _priority; }

void CompMessageBase::setPriority(int p) { _priority = p; }

}
}
