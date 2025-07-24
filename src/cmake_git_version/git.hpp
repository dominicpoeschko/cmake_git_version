#pragma once

#include "cmake_git_version/git_version_generated.hpp"
#include "core.hpp"

#include <string_view>

namespace CMakeGitVersion::Git {

static constexpr std::string_view CommitTime{detail::CommitTime.sv()};

}   // namespace CMakeGitVersion::Git
