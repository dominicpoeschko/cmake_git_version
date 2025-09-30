#pragma once

#include "../core.hpp"
#include "string_utils.hpp"

#include <string_view>

namespace CMakeGitVersion::Compiler {

#ifdef __clang__
static constexpr std::string_view Name{"Clang"};

namespace detail {
    static constexpr Version Version{.major = __clang_major__,
                                     .minor = __clang_minor__,
                                     .patch = __clang_patchlevel__};
}
#elif defined(_MSC_VER)
static constexpr std::string_view Name{"MSVC"};

namespace detail {
    static constexpr Version Version{.major = (_MSC_VER - (_MSC_VER % 100)) / 100,
                                     .minor = _MSC_VER % 100,
                                     .patch = 0};
}
#else
static constexpr std::string_view Name{"GCC"};

namespace detail {
    static constexpr Version Version{.major = __GNUC__,
                                     .minor = __GNUC_MINOR__,
                                     .patch = __GNUC_PATCHLEVEL__};
}
#endif

static constexpr std::string_view Version{
  ::CMakeGitVersion::detail::VersionToStringView<detail::Version>};

}   // namespace CMakeGitVersion::Compiler
