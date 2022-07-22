#include <maf/logging/Logger.h>

#include <atomic>

namespace maf {
namespace logging {

namespace {

struct Statics {
  LoggingFunctionType out = [](const std::string &) {};
  LoggingFunctionType err = [](const std::string &) {};
  std::atomic<LogLevels> allowedLevels = LOG_LEVEL_SILENCE;
};

static Statics &statics() {
  static auto s = new Statics;
  return *s;
}

}  // namespace

void init(LogLevels allowedLevels, LoggingFunctionType outLogFunc,
          LoggingFunctionType errLogFunc) {
  if (outLogFunc) {
    statics().out = std::move(outLogFunc);
  }
  if (errLogFunc) {
    statics().err = std::move(errLogFunc);
  } else if (statics().out) {
    statics().err = statics().out;
  }

  changeLogLevels(allowedLevels);
}
void stopLogging() { statics().allowedLevels = LOG_LEVEL_SILENCE; }

void changeLogLevels(LogLevels allowedLevels) {
  statics().allowedLevels = allowedLevels;
}

bool allowed(LogLevel level) {
  return statics().allowedLevels.load(std::memory_order_relaxed) & level;
}

void enable(LogLevel level) { statics().allowedLevels &= level; }

void disable(LogLevel level) { statics().allowedLevels &= ~level; }

void logImpl(LogLevel filteredLevel, const std::string &msg) {
  switch (filteredLevel) {
    case LOG_LEVEL_INFO:
    case LOG_LEVEL_DEBUG:
    case LOG_LEVEL_VERBOSE:
      statics().out(msg);
      break;
    default:
      statics().err(msg);
      break;
  }
}

}  // namespace logging
}  // namespace maf
