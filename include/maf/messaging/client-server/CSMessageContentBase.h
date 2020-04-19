#ifndef MAF_MESSAGING_CLIENT_SERVER_CSMESSAGECONTENTBASE_H
#define MAF_MESSAGING_CLIENT_SERVER_CSMESSAGECONTENTBASE_H

namespace maf {
namespace messaging {

class CSMessageContentBase {
public:
  enum class Type : char { Data, Error, NA };

  CSMessageContentBase(Type type) : type_{type} {}
  virtual ~CSMessageContentBase() = default;
  Type type() const { return type_; }
  void setType(Type t) { type_ = t; }
  virtual bool equal(const CSMessageContentBase *other) const = 0;
  virtual CSMessageContentBase *clone() const = 0;

private:
  Type type_ = Type::NA;
};

} // namespace messaging
} // namespace maf

#endif // MAF_MESSAGING_CLIENT_SERVER_CSMESSAGECONTENTBASE_H
