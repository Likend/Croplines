#pragma once

#include <cassert>

#ifdef NDEBUG
#define UNREACHABLE() __builtin_unreachable()
#else
#define UNREACHABLE() assert(false && "Unreachable!");
#endif
