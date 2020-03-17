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
    MAF_EXPORT static ComponentRef getActiveWeakPtr();
    MAF_EXPORT static std::shared_ptr<Component> getActiveSharedPtr();

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

    MAF_EXPORT void post(ComponentMessage msg);

    MAF_EXPORT void registerMessageHandler(
            ComponentMessageID msgid,
            std::weak_ptr<ComponentMessageHandler> handler
            );

    MAF_EXPORT void registerMessageHandler(
            ComponentMessageID msgid,
            GenericMsgHandlerFunction onMessageFunc
            );

    template<class Msg,
             typename... Args,
             std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true
             >
    void post(Args&&... args);

    template<class Msg>
    Component& onMessage(ComponentMessageHandlerFunction<Msg> f);

    MAF_EXPORT ~Component();

private:
    static void setTLRef(ComponentRef ref);
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

template<class Msg>
Component &Component::onMessage(ComponentMessageHandlerFunction<Msg> f)
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
    return *this;
}

template<class Msg,
         typename ...Args,
         std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool>
         >
void Component::post(Args&&... args)
{
    post(Msg{std::forward<Args>(args)...});
}

#define mc_maf_tlcomp_invoke(method, ...)      \
if(auto comp = Component::getActiveSharedPtr())\
{                                              \
    comp->method(__VA_ARGS__);                 \
    return true;                               \
}                                              \
else                                           \
{                                              \
    return false;                              \
}


template<class Msg >
bool tlcompOnMessage(ComponentMessageHandlerFunction<Msg> f)
{
    mc_maf_tlcomp_invoke(onMessage<Msg>, std::move(f))
}

template<class Msg,
         typename... Args,
         std::enable_if_t<std::is_constructible_v<Msg, Args...>, bool> = true
         >
bool tlcomppost(Args&&... args)
{
    mc_maf_tlcomp_invoke(post<Msg>, std::forward<Args>(args)...)
}

#undef mc_maf_tlcomp_invoke

}
}

