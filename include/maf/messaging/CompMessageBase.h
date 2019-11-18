#ifndef MESSAGE_H
#define MESSAGE_H

#include <typeindex>
#include <memory>

namespace maf {
namespace messaging {

class CompMessageBase;
template <typename Msg> using MessagePtr    = std::shared_ptr<Msg>;
template <typename Msg> using CMessagePtr   = const std::shared_ptr<Msg>;
using MessageBasePtr                        = MessagePtr<CompMessageBase>;
using CMessageBasePtr                       = CMessagePtr<CompMessageBase>;


template<class Msg, typename... Args>
MessagePtr<Msg> makeCompMessage(Args&&... args) {
    return std::make_shared<Msg>(std::forward<Args>(args)...);
}


class CompMessageBase
{
public:
    using Type = std::type_index;
    virtual ~CompMessageBase();
    virtual Type id() const;
    int priority() const;
    void setPriority(int p);

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


template<typename T> static CompMessageBase::Type msgID() { return typeid (T); }
template<typename T> static CompMessageBase::Type msgID(const T& obj) { return typeid (obj); }

}
}
#endif // MESSAGE_H
