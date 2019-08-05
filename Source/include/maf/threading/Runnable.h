#ifndef RUNNABLE_H
#define RUNNABLE_H

namespace maf {
namespace threading {

class Runnable
{
public:
    virtual void run() = 0;
    virtual void stop() { }
    bool autoDelete() const { return _autoDeleted; }
    void setAutoDeleted(bool value) { _autoDeleted = value; }
    virtual ~Runnable() {}
private:
    bool _autoDeleted;
};

inline void run(Runnable* runner)
{
    if(runner)
    {
        runner->run();
    }
}

inline void stop(Runnable* runner)
{
    if(runner)
    {
        runner->stop();
    }
}

inline void done(Runnable* runner)
{
    if(runner && runner->autoDelete())
    {
        delete runner;
    }
}

}
}
#endif // RUNNABLE_H
