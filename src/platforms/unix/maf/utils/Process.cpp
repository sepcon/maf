#include <maf/utils/Process.h>
#include <unistd.h>

namespace maf {
namespace util {
namespace process {

ProcessID pid() { return getpid(); }

} // namespace process
} // namespace util
} // namespace maf
