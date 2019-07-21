#ifndef HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H
#define HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H


#ifdef ENABLE_IPC_DEBUG
#    if defined(__clang__) || defined (__GNUC__)
#        define THAF_FUNC __FUNCTION__
#    elif defined(_MSC_VER)
#        define THAF_FUNC __FUNCTION__
#    else
#        define THAF_FUNC ""
#    endif

#    if ENABLE_IPC_DEBUG == 2
#        define THAF_CODE_INFO() "\t\t: " << __FILE__ << ":" << __LINE__ << ":("<< THAF_FUNC << ")" << ":"
#    else
#        define THAF_CODE_INFO() ""
#    endif

#    define thafInfo(messageChain)  sendToMyLoggingDevice("INFO: " << messageChain, std::cout, THAF_CODE_INFO())
#    define thafWarn(messageChain)  sendToMyLoggingDevice("WARN: " << messageChain, std::cerr, THAF_CODE_INFO())
#    define thafErr(messageChain)   sendToMyLoggingDevice("ERROR: " << messageChain, std::cerr, THAF_CODE_INFO())
#    define thafFatal(messageChain) sendToMyLoggingDevice("FATAL: " << messageChain, std::cerr, THAF_CODE_INFO())

#else
#    define thafInfo(messageChain) sendToMyLoggingDevice(messageChain, thaf::debugging::DumpMan(), "")
#    define thafWarn(messageChain) sendToMyLoggingDevice(messageChain, thaf::debugging::DumpMan(), "")
#    define thafErr(messageChain)  sendToMyLoggingDevice(messageChain, thaf::debugging::DumpMan(), "")
#endif

#    include <iostream>
#    include <sstream>
namespace thaf {
namespace debugging {

class DumpMan
{
public:
    template<typename T>
    DumpMan& operator<<(const T& /*anything*/) { return *this; }
	void flush() {}
};

}
}


#    define thafMsg(messageChain)   sendToMyLoggingDevice(messageChain, std::cout, "")
#    define sendToMyLoggingDevice(messageChain, loggingDevice, debugInfo)  { \
                std::ostringstream oss; \
                oss << messageChain << debugInfo << "\n"; \
                loggingDevice << oss.str(); \
                loggingDevice.flush(); \
            } void(0)

#endif // HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H
