#ifndef MESSAGEVALIDATOR_H
#define MESSAGEVALIDATOR_H

namespace maf {
namespace messaging {
class CSMessage;

namespace ipc {

class MessageValidatorIF {
public:
  virtual bool isValid(const CSMessage *) { return true; }
};

} // namespace ipc
} // namespace messaging
} // namespace maf
#endif // MESSAGEVALIDATOR_H
