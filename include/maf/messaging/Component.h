#pragma once

#include <maf/export/MafExport_global.h>
#include <maf/patterns/Patterns.h>
#include <typeindex>
#include <functional>
#include <future>
#include <any>



namespace maf {
namespace messaging {

class TimerManager;
class Component;

using ComponentRef    = std::weak_ptr<Component>;
using ComponentPtr    = std::shared_ptr<Component>;
using TimerMgrPtr     = std::shared_ptr<TimerManager>;

enum class LaunchMode
{
    Async,
    AttachToCurrentThread
};

using ComponentMessage                  = std::any;
using ComponentMessageID                = std::type_index;
using GenericMsgHandlerFunction         = std::function<void(ComponentMessage)>;
template <class Msg>
using ComponentMessageHandlerFunction   = std::function<void(Msg)>;
class ComponentMessageHandler;


class Component final : pattern::Unasignable, public std::enable_shared_from_this<Component>
{
    std::unique_ptr<struct ComponentDataPrv> d_;
    MAF_EXPORT Component();
public:

    MAF_EXPORT static std::shared_ptr<Component> create();
    MAF_EXPORT const std::string& name() const;
    MAF_EXPORT void setName(std::string name);

    MAF_EXPORT void run(
            std::function<void()> onEntry = {},
            std::function<void()> onExit = {}
            );

    MAF_EXPORT std::future<void> runAsync(
            std::function<void()> onEntry = {},
            std::function<void()> onExit = {}
            );

    MAF_EXPORT void stop();

    MAF_EXPORT bool post(ComponentMessage&& msg);

    MAF_EXPORT void registerMessageHandler(
            ComponentMessageID msgid,
            std::weak_ptr<ComponentMessageHandler> handler
            );

    MAF_EXPORT void registerMessageHandler(
            ComponentMessageID msgid,
            GenericMsgHandlerFunction onMessageFunc
            );

    MAF_EXPORT bool unregisterHandler( ComponentMessageID msgid );

    template<class Msg>
    Component* onMessage(ComponentMessageHandlerFunction<Msg> f)
    {
        auto translatorCallback = [callback = std::move(f), this](ComponentMessage genericMsg) {
            try
            {
                callback(std::any_cast<Msg>(std::move(genericMsg)));
            }
            catch(const std::exception& e)
            {
                logError(std::string{"Failed to proccess message: "} + e.what());
            }
        };
        registerMessageHandler(typeid(Msg), std::move(translatorCallback));
        return this;
    }

    template<class Msg,
             typename... Args,
             std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true
             >
    bool post(Args&&... args)
    {
        return post(Msg{std::forward<Args>(args)...});
    }

    template<class Msg>
    bool ignoreMessage()
    {
        return this->unregisterHandler(typeid (Msg));
    }


private:
    MAF_EXPORT ~Component();
    void logError(const std::string& info);
    friend struct ComponentImpl;
    friend class CompThread;
};


class ComponentMessageHandler
{
public:
    virtual void onMessage(ComponentMessage msg) = 0;
    virtual ~ComponentMessageHandler() = default;
};

struct RunningComponent
{
    MAF_EXPORT static std::shared_ptr<Component> shared();
    MAF_EXPORT static std::weak_ptr<Component> weak();
    MAF_EXPORT static bool stop();
    MAF_EXPORT static bool post(ComponentMessage&& msg);

    template<class Msg,
             typename ...Args,
             std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool>
             >
    static bool post(Args&&... args)
    {
        return post(Msg{std::forward<Args>(args)...});
    }

    template<class Msg>
    static bool onMessage(ComponentMessageHandlerFunction<Msg> f)
    {
        if(auto comp = shared())
        {
            comp->onMessage<Msg>(std::move(f));
            return true;
        }
        return false;
    }
    template<class Msg>
    static bool ignoreMessage()
    {
        if(auto comp = shared())
        {
            return comp->ignoreMessage<Msg>();
        }
        return false;
    }
};

}
}

