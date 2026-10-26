#pragma once

#include "core.hpp"
#include "detail/compiler.hpp"
#include "detail/string_utils.hpp"
#include "git.hpp"
#include "project.hpp"

namespace CMakeGitVersion {

static constexpr std::string_view fullVersion{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{Project::Name,
                        std::string_view{"("},
                        Project::version,
                        std::string_view{"+"},
                        Git::Hash,
                        Git::Dirty,
                        std::string_view{" "},
                        Target::buildTime,
                        std::string_view{" "},
                        Target::Name,
                        std::string_view{"/"},
                        Compiler::Optimization,
                        std::string_view{" "},
                        Compiler::name,
                        std::string_view{"-"},
                        Compiler::version,
                        std::string_view{")"}};
  }))>::stringView};

static constexpr std::string_view nameTargetVersion{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{Project::Name,
                        std::string_view{" "},
                        Target::Name,
                        std::string_view{" "},
                        Project::version};
  }))>::stringView};

static constexpr std::string_view shortVersion{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{Project::version, std::string_view{"+"}, Git::Hash, Git::Dirty};
  }))>::stringView};

static constexpr std::string_view gitInfo{detail::StringViewMerge<decltype(detail::makeGetter([]() {
    return std::array{std::string_view{"branch:"},
                      Git::Branch,
                      std::string_view{" "},
                      Git::Hash,
                      Git::Dirty,
                      std::string_view{" (author: "},
                      Git::Author,
                      std::string_view{")"}};
}))>::stringView};

static constexpr std::string_view debugString{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{std::string_view{"Project: "},
                        Project::Name,
                        std::string_view{" "},
                        Project::version,
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
                        Git::commitTime,
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
                        Target::buildTime,
                        std::string_view{" (CMake: "},
                        Target::cmakeTime,
                        std::string_view{")\n"},
                        std::string_view{"Compiler: "},
                        Compiler::name,
                        std::string_view{" "},
                        Compiler::version};
  }))>::stringView};

}   // namespace CMakeGitVersion
