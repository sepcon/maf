#ifndef MESSAGE_H
#define MESSAGE_H

#include <typeindex>
#include <memory>

namespace thaf
{

namespace messaging
{
class MessageBase;
template <typename Msg> using MessagePtr = std::shared_ptr<Msg>;
template <typename Msg> using CMessagePtr = const std::shared_ptr<Msg>;
using MessageBasePtr = MessagePtr<MessageBase>;
using CMessageBasePtr = CMessagePtr<MessageBase>;

class MessageBase
{
public:
    using Type = std::type_index;
    virtual Type id() const { return typeid (*this); }
    template<typename T>
    static Type idof() { return typeid (T); }
    template<typename T>
    static Type idof(const T& obj) { return typeid (obj); }
    virtual ~MessageBase(){}

#ifdef MESSAGIN_BY_PRIORITY
    struct PriorityComp
    {
        bool operator()(CMessageBasePtr m1, CMessageBasePtr m2)
        {
            return m1->priority() < m2->priority();
        }
    };
    int priority() const { return _priority; }
    void setPriority(int p) { _priority = p; }
private:
    int _priority = 0;
#endif
};


}
}
#endif // MESSAGE_H
