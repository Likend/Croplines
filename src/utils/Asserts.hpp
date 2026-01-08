#pragma once

#include <cstddef>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string_view>

#define ANNOTATE_LOCATION __FUNCTION__, __FILE__, __LINE__

#ifndef NDEBUG

namespace assert_detail {
template <typename... Arg>
[[noreturn]] static inline void assert_throw(std::string_view func_name, std::string_view file_name,
                                             size_t file_line, const Arg&... args) {
    std::stringstream ss;
    ((ss << args << ' '), ...);
    ss << '(' << func_name << " @ " << file_name << ':' << file_line << ')';
    throw std::runtime_error(ss.str());
}
}  // namespace assert_detail

#define ASSERT_WITH(x, ...)                                                \
    do {                                                                   \
        if (x) {                                                           \
        } else {                                                           \
            ::assert_detail::assert_throw(ANNOTATE_LOCATION, __VA_ARGS__); \
        }                                                                  \
    } while (0);

#define ASSERT(x) ASSERT_WITH(x, "Assert failed!")
#define UNREACHABLE() ASSERT_WITH(false, "Unreachable!");

#else

#define UNREACHABLE() __builtin_unreachable()
#define ASSERT_WITH(x, ...) \
    do {                    \
        if (x) {            \
        } else {            \
            UNREACHABLE();  \
        }                   \
    }
#define ASSERT(x) ASSERT_WITH(x)

#endif
