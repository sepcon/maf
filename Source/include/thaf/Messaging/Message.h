#ifndef MESSAGE_H
#define MESSAGE_H

#include <typeindex>
#include <memory>

namespace thaf {
namespace messaging {

class MessageBase;
template <typename Msg> using MessagePtr = std::shared_ptr<Msg>;
template <typename Msg> using CMessagePtr = const std::shared_ptr<Msg>;
using MessageBasePtr = MessagePtr<MessageBase>;
using CMessageBasePtr = CMessagePtr<MessageBase>;
template<class Msg, typename... Args>
MessagePtr<Msg> createMessage(Args&&... args) { return std::make_shared<Msg>(std::forward<Args>(args)...); }

class MessageBase
{
public:
    using Type = std::type_index;
    virtual Type id() const { return typeid (*this); }
    template<typename T>
    static Type idof() { return typeid (T); }
    template<typename T>
    static Type idof(const T& obj) { return typeid (obj); }
    virtual ~MessageBase() = default;
    int priority() const { return _priority; }
    void setPriority(int p) { _priority = p; }

    struct PriorityComp
    {
        bool operator()(CMessageBasePtr m1, CMessageBasePtr m2)
        {
            return m1->priority() < m2->priority();
        }
    };

private:
    int _priority = 0;
};


}
}
#endif // MESSAGE_H
