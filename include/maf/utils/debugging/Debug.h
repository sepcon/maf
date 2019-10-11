#ifndef HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H
#define HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H

//#define ENABLE_MAF_DEBUG_LOG 1
#ifdef ENABLE_MAF_DEBUG_LOG
#    if defined(__clang__) || defined (__GNUC__)
#        define maf_FUNC __FUNCTION__
#    elif defined(_MSC_VER)
#        define maf_FUNC __FUNCTION__
#    else
#        define maf_FUNC ""
#    endif

#    if ENABLE_MAF_DEBUG_LOG == 2
#        define maf_CODE_INFO() "\t\t: " << __FILE__ << ":" << __LINE__ << ":("<< maf_FUNC << ")" << ":"
#    else
#        define maf_CODE_INFO() ""
#    endif

#    define mafInfo(messageChain)  sendToMyLoggingDevice("INFO: " << messageChain, std::cout, maf_CODE_INFO())
#    define mafWarn(messageChain)  sendToMyLoggingDevice("WARN: " << messageChain, std::cerr, maf_CODE_INFO())
#    define mafErr(messageChain)   sendToMyLoggingDevice("ERROR: " << messageChain, std::cerr, maf_CODE_INFO())
#    define mafFatal(messageChain) sendToMyLoggingDevice("FATAL: " << messageChain, std::cerr, maf_CODE_INFO())

#else
#    define mafInfo(messageChain) sendToMyLoggingDevice(messageChain, maf::debugging::DumpMan(), "")
#    define mafWarn(messageChain) sendToMyLoggingDevice(messageChain, maf::debugging::DumpMan(), "")
#    define mafErr(messageChain)  sendToMyLoggingDevice(messageChain, maf::debugging::DumpMan(), "")
#endif

#    include <iostream>
#    include <sstream>
namespace maf {
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


#    define mafMsg(messageChain)   sendToMyLoggingDevice(messageChain, std::cout, "")
#    define sendToMyLoggingDevice(messageChain, loggingDevice, debugInfo)  { \
                std::ostringstream oss; \
                oss << messageChain << debugInfo << "\n"; \
                loggingDevice << oss.str(); \
                loggingDevice.flush(); \
            } void(0)

#endif // HEADERS_LIBS_UTILS_DEBUGGING_DEBUG_H
