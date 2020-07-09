#include <maf/utils/Process.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace maf {
namespace util {
namespace process {

ProcessID pid() { return GetCurrentProcessId(); }

} // namespace process
} // namespace util
} // namespace maf
