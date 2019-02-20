#ifndef SIGNAL_H
#define SIGNAL_H


class Signal
{
public:
    Signal();
    ~Signal();
    void wait();
    void notify();
    void notifyAll();
    void stop();
    void lock();
    void unlock();
    bool isStopped() const;
    class SignalImpl* _pImpl;
};

#endif // SIGNAL_H
