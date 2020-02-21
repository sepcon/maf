#pragma once

#include <mutex>
#include <maf/patterns/Patterns.h>

namespace maf {
namespace threading {

template<class Protected, class Mutex = std::mutex>
class AtomicObject : public pattern::Unasignable
{
    template<class AtomicObject_, class Protected_>
    friend struct AtomicRef;

    template<class AtomicObject_, class Protected_>
    struct AtomicRef : public pattern::Unasignable
    {
    public:
        AtomicRef(AtomicObject_* p) : _AtomicObject{p}
        {
            _AtomicObject->_mutex.lock();
        }

        AtomicRef(const Protected_& p)
        {
            operator*() = p;
        }

        AtomicRef(Protected_&& p)
        {
            operator*() = std::move(p);
        }

        AtomicRef& operator=(const Protected_& p)
        {
            operator*() = p;
            return *this;
        }

        AtomicRef& operator=(Protected_&& p)
        {
            operator*() = std::move(p);
            return *this;
        }

        ~AtomicRef()
        {
            _AtomicObject->_mutex.unlock();
        }

        Protected_* operator->()
        {
            return &(_AtomicObject->_protected);
        }
        Protected_& operator*()
        {
            return _AtomicObject->_protected;
        }

        const Protected_* operator->() const
        {
            return &(_AtomicObject->_protected);
        }

        const Protected_& operator*() const
        {
            return _AtomicObject->_protected;
        }

        operator Protected_&() { return _AtomicObject->_protected; }
        operator const Protected_&() const { return _AtomicObject->_protected; }

    private:
        AtomicObject_* _AtomicObject;
    };

public:

    using DataType = Protected;
    using AtomicRefType = AtomicRef<AtomicObject<Protected, Mutex>, Protected>;
    using CAtomicRefType = AtomicRef<const AtomicObject<Protected, Mutex>, const Protected>;

    AtomicObject() = default;
    template<std::enable_if_t<std::is_copy_assignable_v<DataType>, bool> = true>
    AtomicObject(const DataType& p) :_protected(p) { }
    template<std::enable_if_t<std::is_move_assignable_v<DataType>, bool> = true>
    AtomicObject(DataType&& p) :_protected(std::move(p)) {}

    Mutex& getMutex() { return _mutex; }
    const Mutex& getMutex() const { return _mutex; }

    DataType& lockee() { return _protected; }
    const DataType& lockee() const { return _protected; }

    AtomicRefType operator->() { return AtomicRefType{this}; }
    AtomicRefType operator*() { return AtomicRefType{this}; }

    CAtomicRefType operator->() const { return CAtomicRefType{this}; }
    CAtomicRefType operator*() const  { return CAtomicRefType{this}; }

private:
    mutable Mutex _mutex;
    DataType _protected;
};

} //threading
} //maf
