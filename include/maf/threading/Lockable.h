#pragma once

#include <mutex>
#include <maf/patterns/Patterns.h>

namespace maf {
namespace threading {

template<class Protected, class Mutex = std::mutex>
class Lockable : public pattern::Unasignable
{
    template<class Lockable_, class Protected_>
    struct AtomicRef : public pattern::Unasignable
    {
    public:
        AtomicRef(Lockable_* p) : _lockable{p}
        {
            _lockable->lock();
        }

        ~AtomicRef()
        {
            _lockable->unlock();
        }

        Protected_* operator->()
        {
            return _lockable->operator->();
        }
        Protected_& operator*()
        {
            return _lockable->operator*();
        }

    private:
        Lockable_* _lockable;
    };

public:
    using DataType = Protected;
    using AtomicSession = AtomicRef<Lockable<Protected, Mutex>, Protected>;
    using CAtomicSession = AtomicRef<const Lockable<Protected, Mutex>, const Protected>;

    Lockable() = default;
    template<std::enable_if_t<std::is_copy_assignable_v<DataType>, bool> = true>
    Lockable(const DataType& p) :_protected(p) { }
    template<std::enable_if_t<std::is_move_assignable_v<DataType>, bool> = true>
    Lockable(DataType&& p) :_protected(std::move(p)) {}

    void lock() const { _mutex.lock(); }
    void lock()  { _mutex.lock(); }
    void unlock() const { _mutex.unlock(); }
    void unlock()  { _mutex.unlock(); }
    bool try_lock() const { return _mutex.try_lock(); }
    bool try_lock() { return _mutex.try_lock(); }

    DataType* operator->() { return &_protected; }
    DataType& operator*() { return _protected; }
    const DataType* operator->() const { return &_protected; }
    const DataType& operator*() const { return _protected; }

    CAtomicSession atomic() const { return CAtomicSession{this}; }
    AtomicSession atomic() { return AtomicSession { this }; }

private:
    mutable Mutex _mutex;
    DataType _protected;
};

} // nstl
} // maf


