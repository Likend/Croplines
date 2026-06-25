#pragma once

#include <cassert>

#ifndef NDEBUG
#define UNREACHABLE() assert(false && "Unreachable!");
#else

#define UNREACHABLE() __builtin_unreachable()

#endif
