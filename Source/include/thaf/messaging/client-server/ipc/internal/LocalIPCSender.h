#pragma once

#if defined (WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)
#include "Platforms/windows/LocalIPCSender.h"
#elif defined (LINUX)

#elif defined (MAC)
#endif
