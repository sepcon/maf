#ifndef LOGGERBASE_H
#define LOGGERBASE_H

#include "LoggerInterface.h"
#include "LoggingComponent.h"
#include "thaf/Patterns/Patterns.h"

namespace thaf {
namespace logging {

template <typename CharT>
struct LoggingMessage : messaging::MessageBase
{
    using Content = std::basic_string<CharT>;
    Content msgContent;
    LogLevel level;
    LoggingMessage(const Content& msgContent_, LogLevel level_ = LogLevel::INFO) :
        msgContent(msgContent_), level(level_){}
};

template<typename WStrLogRequestMsg, typename StrLogRequestMsg>
class LoggerBase : public ILogger
{
public:
    void log(const std::string& message, LogLevel level = LogLevel::INFO) override
    {
        LoggingComponent::instance().postMessage<StrLogRequestMsg>(message, level);
    }
    void log(const std::wstring& message, LogLevel level = LogLevel::INFO) override
    {
        LoggingComponent::instance().postMessage<WStrLogRequestMsg>(message, level);
    }
protected:
    virtual std::string format(const std::string& message, LogLevel) const
    {
        return message;
    }
    virtual std::wstring format(const std::wstring& message, LogLevel) const
    {
        return message;
    }
    virtual void write(const std::string& , LogLevel = LogLevel::INFO) {}
    virtual void write(const std::wstring& , LogLevel = LogLevel::INFO) {}

    LoggerBase()
    {
        mapMessageHandler<WStrLogRequestMsg>();
        mapMessageHandler<StrLogRequestMsg>();
    }
    template <typename Msg>
    void mapMessageHandler() {
        LoggingComponent::instance().onMessage<Msg>([this](messaging::CMessagePtr<Msg> msg){
            if(msg)
            {
                write(format(msg->msgContent, msg->level), msg->level);
            }
        });
    }
};


class NullLogger : public ILogger, public pattern::SingletonObject<NullLogger>
{
public:
    NullLogger(Invisible){}
    void log(const std::string&, LogLevel = LogLevel::INFO) override {}
    void log(const std::wstring&, LogLevel = LogLevel::INFO) override {}
};
} //app
} //thaf

#define DCL_LOGGING_MSG(MsgName) template <typename C> \
    struct MsgName : public thaf::logging::LoggingMessage<C> { \
    using LoggingMessage<C>::LoggingMessage; \
    }
#define SINGLETON_LOGGER_CLASS_START(LoggerClass, UniqueMessageName) \
    DCL_LOGGING_MSG(UniqueMessageName); \
    class LoggerClass : \
            public LoggerBase<UniqueMessageName<wchar_t>, UniqueMessageName<char>>, \
            public pattern::SingletonObject \
    { \
    public: \
        static LoggerClass* instance(); \
    private: \
        LoggerClass();
#define SINGLETON_LOGGER_CLASS_END(LoggerClass) };

#endif // LOGGERBASE_H
