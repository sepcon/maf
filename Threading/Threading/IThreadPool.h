#ifndef ITHREADPOOL_H
#define ITHREADPOOL_H

namespace Threading
{

class Runable
{
public:
    virtual void run() = 0;
    virtual void stop() {}
    bool autoDelete() const { return _autoDeleted; }
    void setAutoDeleted(bool value) { _autoDeleted = value; }
    virtual ~Runable() {}
private:
    bool _autoDeleted;
};

class IThreadPool
{
public:
    virtual void run(Runable* pRuner, unsigned int priority = 0) = 0;
    virtual void setMaxThreadCount(unsigned int nThreadCount) = 0;
    virtual unsigned int activeThreadCount() = 0;
    virtual void shutdown() = 0;
    virtual ~IThreadPool() {}
};

}


#endif // ITHREADPOOL_H
