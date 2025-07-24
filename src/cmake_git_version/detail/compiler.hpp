#pragma once

#include "../core.hpp"
#include "string_utils.hpp"

#include <string_view>

namespace CMakeGitVersion::Compiler {

#ifdef __clang__
static constexpr std::string_view Name{"Clang"};

namespace detail {
    static constexpr Version Version{__clang_major__, __clang_minor__, __clang_patchlevel__};
}
#elif defined(_MSC_VER)
static constexpr std::string_view Name{"MSVC"};

namespace detail {
    static constexpr Version Version{(_MSC_VER - (_MSC_VER % 100)) / 100, _MSC_VER % 100, 0};
}
#else
static constexpr std::string_view Name{"GCC"};

namespace detail {
    static constexpr Version Version{__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__};
}
#endif

static constexpr std::string_view Version{
  ::CMakeGitVersion::detail::VersionToStringView<detail::Version>};

}   // namespace CMakeGitVersion::Compiler
