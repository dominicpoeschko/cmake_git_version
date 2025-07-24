#pragma once

#include "cmake_git_version/project_version_generated.hpp"
#include "core.hpp"
#include "detail/string_utils.hpp"

#include <string_view>

namespace CMakeGitVersion {

namespace Project {
    static constexpr std::string_view Version{
      ::CMakeGitVersion::detail::VersionToStringView<detail::Version>};
}

namespace Target {
    namespace detail {
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdate-time"
#endif
        static constexpr DateTime BuildTime{__DATE__ " " __TIME__};
#ifdef __clang__
    #pragma clang diagnostic pop
#endif
    }   // namespace detail

    static constexpr std::string_view CMakeTime{detail::CMakeTime.sv()};
    static constexpr std::string_view BuildTime{detail::BuildTime.sv()};
}   // namespace Target

}   // namespace CMakeGitVersion
