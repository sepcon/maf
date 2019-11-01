#pragma once
#include <maf/utils/IDManager.h>
#include <maf/patterns/Patterns.h>
#include <functional>

namespace maf {
namespace messaging {

class TimerManager : public pattern::Unasignable
{
public:
    using JobID = util::IDManager::IDType;
    typedef long long Duration;
    typedef std::function<void()> TimeOutCallback;
    TimerManager();
    ~TimerManager();
    JobID start(Duration milliseconds, TimeOutCallback callback, bool cyclic = false);
    void restart(JobID jid);
    void stop(JobID jid);
	void stop();
    bool isRunning(JobID jid);
    void setCyclic(JobID jid, bool cyclic = true);
    static bool isValid(JobID jid);
    static JobID invalidJobID() { return util::IDManager::INVALID_ID; }

private:
    struct TimerManagerImpl* _pImpl;
    util::IDManager _idManager;
};

}
}
