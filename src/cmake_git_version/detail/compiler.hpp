#pragma once

#include "../core.hpp"
#include "string_utils.hpp"

#include <string_view>

namespace CMakeGitVersion::Compiler {

#ifdef __clang__
static constexpr std::string_view name{"Clang"};

namespace detail {
    static constexpr Version version{.major = __clang_major__,
                                     .minor = __clang_minor__,
                                     .patch = __clang_patchlevel__};
}

// #elifdef is C++23; this header also serves C++20 users (MSVC).
// NOLINTNEXTLINE(readability-use-concise-preprocessor-directives) // C++20 users
#elif defined(_MSC_VER)
static constexpr std::string_view name{"MSVC"};

namespace detail {
    static constexpr Version version{.major = (_MSC_VER - (_MSC_VER % 100)) / 100,
                                     .minor = _MSC_VER % 100,
                                     .patch = 0};
}
#else
static constexpr std::string_view name{"GCC"};

namespace detail {
    static constexpr Version version{.major = __GNUC__,
                                     .minor = __GNUC_MINOR__,
                                     .patch = __GNUC_PATCHLEVEL__};
}
#endif

static constexpr std::string_view version{
  ::CMakeGitVersion::detail::versionToStringView<detail::version>};

}   // namespace CMakeGitVersion::Compiler
