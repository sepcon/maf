#pragma once

#include <mutex>
#include <memory>

namespace thaf {
namespace stl {


template<class ProtectedObject, class Mutex = std::mutex>
class SyncObject
{
public:
    using DataType = ProtectedObject;
    using MutexType = Mutex;

    SyncObject(ProtectedObject obj = {}) : _o(std::move(obj)){}

    Mutex& getMutex() { return _m; }

    const ProtectedObject* operator->() const
    {
        return &_o;
    }

    ProtectedObject* operator->()
    {
        return &_o;
    }

    const ProtectedObject& operator*() const
    {
        return _o;
    }

    ProtectedObject& operator*()
    {
        return _o;
    }

    operator ProtectedObject()
    {
        return _o;
    }

    void reset(ProtectedObject obj = {})
    {
        _o = std::move(obj);
    }

    ProtectedObject& get()
    {
        return _o;
    }

    const ProtectedObject& get() const
    {
        return _o;
    }

    /**
     * @brief a_lock: auto lock, the caller must call it like this: auto lock = TheAtomicProtectedObject.a_lock();
     * to keep the lock_guard object alive until it leave the specified scope
     * @return lock_guard<Mutex> object, that will auto free the lock when it leaves its scope
     */

    std::lock_guard<Mutex> a_lock() const
    {
        return std::lock_guard<Mutex>(_m);
    }

    std::unique_ptr<std::lock_guard<std::mutex>> pa_lock() const
    {
        return std::make_unique<std::lock_guard<std::mutex>>(_m);
    }

    /**
     * @brief lock: manual lock, the caller must call unlock after that
     */
    void lock() const
    {
        _m.lock();
    }

    bool try_lock() const
    {
        return _m.try_lock();
    }
    /**
     * @brief unlock: manual unlock, the caller must call this function after call lock
     */
    void unlock() const
    {
        _m.unlock();
    }

private:
    ProtectedObject _o;
    mutable Mutex _m;
};

template <typename ProtectedObject>
using SyncObjectM = SyncObject<ProtectedObject, std::mutex>;

} // stl
} // thaf


