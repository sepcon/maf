#ifndef ATOMICCONTAINER_H
#define ATOMICCONTAINER_H

#include <mutex>


namespace thaf {
namespace stl {


template<class Container, class Mutex>
class AtomicContainer
{
public:
    Container* operator->()
    {
        return &_c;
    }

    Container& operator*()
    {
        return _c;
    }

    operator Container()
    {
        return _c;
    }

    /**
     * @brief a_lock: auto lock, the caller must call it like this: auto lock = TheAtomicContainer.a_lock();
     * to keep the lock_guard object alive until it leave the specified scope
     * @return lock_guard<Mutex> object, that will auto free the lock when it leaves its scope
     */
    std::lock_guard<Mutex> a_lock()
    {
        return std::lock_guard<Mutex>(_m);
    }

    /**
     * @brief m_lock: manual lock, the caller must call m_unlock after that
     */
    void m_lock()
    {
        _m.lock();
    }

    /**
     * @brief m_unlock: manual unlock, the caller must call this function after call m_lock
     */
    void m_unlock()
    {
        _m.unlock();
    }
private:
    Container _c;
    Mutex _m;
};

template <typename Container>
using MutexContainer = AtomicContainer<Container, std::mutex>;

} // stl
} // thaf


#endif // ATOMICCONTAINER_H
