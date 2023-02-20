#pragma once

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <string_view>

namespace CMakeGitVersion {
namespace detail {
    template<std::size_t N>
    consteval void compile_time_assert(char const (&str)[N], bool predicat) {
        [[maybe_unused]] auto const x = str[N - 1 + static_cast<std::size_t>(!predicat)];
    }
}   // namespace detail

struct DateTime {
    std::array<char, 19>       buffer{};
    constexpr std::string_view sv() const { return std::string_view{buffer.data(), buffer.size()}; }

    template<typename T>
    consteval DateTime(T s) {
        std::string_view input{s};
        detail::compile_time_assert("bad DateTime", input.size() == 19 || input.size() == 20);
        if(input.size() == 19) {
            //"0000-01-01T00:00:00"
            detail::compile_time_assert("bad DateTime", input.size() == 19);
            detail::compile_time_assert("bad DateTime", input[4] == '-');
            detail::compile_time_assert("bad DateTime", input[7] == '-');
            detail::compile_time_assert("bad DateTime", input[10] == 'T');
            detail::compile_time_assert("bad DateTime", input[13] == ':');
            detail::compile_time_assert("bad DateTime", input[16] == ':');
            //TODO more checks...

            std::copy(input.begin(), input.end(), buffer.begin());
            buffer[10] = ' ';
        } else if(input.size() == 20) {
            //"Jan  1 0000 00:00:00"
            detail::compile_time_assert("bad DateTime", input.size() == 20);
            detail::compile_time_assert("bad DateTime", input[3] == ' ');
            detail::compile_time_assert("bad DateTime", input[6] == ' ');
            detail::compile_time_assert("bad DateTime", input[11] == ' ');
            detail::compile_time_assert("bad DateTime", input[14] == ':');
            detail::compile_time_assert("bad DateTime", input[17] == ':');

            constexpr std::array MonthNames{
              std::string_view{"Jan 01"},
              std::string_view{"Feb 02"},
              std::string_view{"Mar 03"},
              std::string_view{"Apr 04"},
              std::string_view{"May 05"},
              std::string_view{"Jun 06"},
              std::string_view{"Jul 07"},
              std::string_view{"Aug 08"},
              std::string_view{"Sep 09"},
              std::string_view{"Oct 10"},
              std::string_view{"Nov 11"},
              std::string_view{"Dec 12"}};

            std::string_view month{};

            for(auto const& m : MonthNames) {
                if(input.starts_with(m.substr(0, 3))) {
                    month = m.substr(4, 2);
                    break;
                }
            }

            auto const day1 = [&]() {
                if(input[4] == ' ') {
                    return std::string_view("0");
                } else {
                    return input.substr(4, 1);
                }
            }();
            auto const day2 = input.substr(5, 1);
            auto const year = input.substr(7, 4);
            auto const time = input.substr(12, 8);

            auto out = buffer.begin();
            auto add = [&](auto ss) { out = std::copy(std::begin(ss), std::end(ss), out); };

            add(year);
            *(out++) = '-';
            add(month);
            *(out++) = '-';
            add(day1);
            add(day2);
            *(out++) = ' ';
            add(time);
        }
    }
};

struct Version {
    std::uint32_t major{};
    std::uint32_t minor{};
    std::uint32_t patch{};
};

namespace detail {

    template<typename ViewsToAddGetter>
    struct StringViewMerge {
        static constexpr auto gen() {
            constexpr auto        viewsToAdd = ViewsToAddGetter::get();
            constexpr std::size_t arraySize  = [&]() {
                std::size_t sum = 0;
                for(auto v : viewsToAdd) {
                    sum += v.size();
                }
                return sum;
            }();

            std::array<char, arraySize> ret{};
            auto                        it = begin(ret);

            auto addString = [&it](std::string_view s) {
                for(auto c : s) {
                    *it = c;
                    ++it;
                }
            };

            for(auto const& v : viewsToAdd) {
                addString(v);
            }

            return ret;
        }

        static constexpr auto             buffer = gen();
        static constexpr std::string_view string_view{buffer.data(), buffer.size()};
    };

    template<typename F>
    constexpr auto makeGetter(F) {
        struct X {
            static constexpr auto get() { return F{}(); }
        };
        return X{};
    }

    template<std::uint32_t... digits>
    struct to_chars {
        inline static constexpr std::array<char, sizeof...(digits)> value{('0' + digits)...};
    };

    template<std::uint32_t rem, std::uint32_t... digits>
    struct explode : explode<rem / 10, rem % 10, digits...> {};

    template<std::uint32_t... digits>
    struct explode<0, digits...> : to_chars<digits...> {};

    template<std::uint32_t num>
    struct num_to_string : explode<num> {};

    template<>
    struct num_to_string<0> {
        inline static constexpr std::array<char, 1> value{'0'};
    };

    template<std::uint32_t Number>
    static constexpr std::string_view toStringView{
      std::begin(num_to_string<Number>::value),
      std::size(num_to_string<Number>::value) - 1};

    template<Version version>
    static constexpr std::string_view VersionToStringView{StringViewMerge<decltype(makeGetter([]() {
        return std::array{
          toStringView<version.major>,
          std::string_view{"."},
          toStringView<version.minor>,
          std::string_view{"."},
          toStringView<version.patch>,
        };
    }))>::string_view};

}   // namespace detail
}   // namespace CMakeGitVersion

#include "cmake_git_version/git_version_generated.hpp"
#include "cmake_git_version/project_version_generated.hpp"

namespace CMakeGitVersion { namespace Compiler {
}}   // namespace CMakeGitVersion::Compiler

namespace CMakeGitVersion {
namespace Git {
    static constexpr std::string_view CommitTime{detail::CommitTime.sv()};
}   // namespace Git
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
        //be carefull possible odr violation...
        static constexpr DateTime BuildTime{__DATE__ " " __TIME__};
#ifdef __clang__
    #pragma clang diagnostic pop
#endif
    }   // namespace detail
    static constexpr std::string_view CMakeTime{detail::CMakeTime.sv()};
    static constexpr std::string_view BuildTime{detail::BuildTime.sv()};
}   // namespace Target
namespace Compiler {
#ifdef __clang__
    static constexpr std::string_view Name{"Clang"};
    namespace detail {
        static constexpr Version Version{__clang_major__, __clang_minor__, __clang_patchlevel__};
    }
#elif defined(_MSC_VER)
    static constexpr std::string_view Name{"MSVC"};
    namespace detail {
        static constexpr Version Version{_MSC_VER - (_MSC_VER % 100), _MSC_VER % 100, 0};
    }
#else
    static constexpr std::string_view Name{"GCC"};
    namespace detail {
        static constexpr Version Version{__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__};
    }
#endif

    static constexpr std::string_view Version{
      ::CMakeGitVersion::detail::VersionToStringView<detail::Version>};
}   // namespace Compiler

static constexpr std::string_view FullVersion{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{
        Project::Name,
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
        Compiler::Optimizeation,
        std::string_view{" "},
        Compiler::Name,
        std::string_view{"-"},
        Compiler::Version,
        std::string_view{")"}};
  }))>::string_view};

static constexpr std::string_view VersionWithGit{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{Project::Version, std::string_view{"+"}, Git::Hash, Git::Dirty};
  }))>::string_view};

static constexpr std::string_view NameTargetVersion{
  detail::StringViewMerge<decltype(detail::makeGetter([]() {
      return std::array{
        Project::Name,
        std::string_view{" "},
        Target::Name,
        std::string_view{" "},
        Project::Version};
  }))>::string_view};

}   // namespace CMakeGitVersion
