#ifndef MESSAGE_H
#define MESSAGE_H

#include <typeindex>
#include <memory>

namespace thaf
{

namespace Messaging
{
class Message;
using MessagePtr = std::shared_ptr<Message>;
using CMessagePtr = const std::shared_ptr<Message>;

class Message
{
public:
    using Type = std::type_index;
    virtual Type id() const { return typeid (*this); }
    template<typename T>
    static Type idof() { return typeid (T); }
    template<typename T>
    static Type idof(const T& obj) { return typeid (obj); }
    virtual ~Message(){}
};

}
}
#endif // MESSAGE_H
