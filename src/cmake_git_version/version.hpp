#pragma once

#include "core.hpp"
#include "detail/compiler.hpp"
#include "detail/string_utils.hpp"
#include "git.hpp"
#include "project.hpp"

namespace CMakeGitVersion {

static constexpr std::string_view FullVersion{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{Project::Name,
                        std::string_view{"("},
                        Project::Version,
                        std::string_view{"+"},
                        Git::Hash,
                        Git::Dirty,
                        std::string_view{" "},
                        Target::BuildTime,
                        std::string_view{" "},
                        Target::Name,
                        std::string_view{"/"},
                        Compiler::Optimization,
                        std::string_view{" "},
                        Compiler::Name,
                        std::string_view{"-"},
                        Compiler::Version,
                        std::string_view{")"}};
  }))>::string_view};

static constexpr std::string_view NameTargetVersion{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{Project::Name,
                        std::string_view{" "},
                        Target::Name,
                        std::string_view{" "},
                        Project::Version};
  }))>::string_view};

static constexpr std::string_view ShortVersion{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{Project::Version, std::string_view{"+"}, Git::Hash, Git::Dirty};
  }))>::string_view};

static constexpr std::string_view GitInfo{detail::StringViewMerge<decltype(detail::makeGetter([]() {
    return std::array{std::string_view{"branch:"},
                      Git::Branch,
                      std::string_view{" "},
                      Git::Hash,
                      Git::Dirty,
                      std::string_view{" (author: "},
                      Git::Author,
                      std::string_view{")"}};
}))>::string_view};

static constexpr std::string_view DebugString{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{std::string_view{"Project: "},
                        Project::Name,
                        std::string_view{" "},
                        Project::Version,
                        std::string_view{"\n"},
                        std::string_view{"Git: "},
                        Git::Branch,
                        std::string_view{" "},
                        Git::Hash,
                        Git::Dirty,
                        std::string_view{" by "},
                        Git::Author,
                        std::string_view{"\n"},
                        std::string_view{"Commit: "},
                        Git::Subject,
                        std::string_view{" ("},
                        Git::CommitTime,
                        std::string_view{")\n"},
                        std::string_view{"Tag: "},
                        Git::Tag,
                        std::string_view{"\n"},
                        std::string_view{"Target: "},
                        Target::Name,
                        std::string_view{" ("},
                        Compiler::Optimization,
                        std::string_view{")\n"},
                        std::string_view{"Built: "},
                        Target::BuildTime,
                        std::string_view{" (CMake: "},
                        Target::CMakeTime,
                        std::string_view{")\n"},
                        std::string_view{"Compiler: "},
                        Compiler::Name,
                        std::string_view{" "},
                        Compiler::Version};
  }))>::string_view};

}   // namespace CMakeGitVersion
