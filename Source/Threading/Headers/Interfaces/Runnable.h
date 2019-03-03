#ifndef RUNNABLE_H
#define RUNNABLE_H

namespace Threading
{

class Runnable
{
public:
    virtual void run() = 0;
    virtual void stop() {}
    bool autoDelete() const { return _autoDeleted; }
    void setAutoDeleted(bool value) { _autoDeleted = value; }
    virtual ~Runnable() {}
private:
    bool _autoDeleted;
};

}

#endif // RUNNABLE_H
