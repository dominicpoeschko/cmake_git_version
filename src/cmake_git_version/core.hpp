#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>

namespace CMakeGitVersion {

namespace detail {
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif
    template<std::size_t N>
    consteval void compile_time_assert(char const (&str)[N],
                                       bool predicate) {
        [[maybe_unused]] auto const x = str[N - 1 + static_cast<std::size_t>(!predicate)];
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif
}   // namespace detail

struct DateTime {
    std::array<char, 19> buffer{};

    constexpr std::string_view sv() const { return std::string_view{buffer.data(), buffer.size()}; }

    template<typename T>
    consteval DateTime(T s) {
        std::string_view input{s};
        detail::compile_time_assert("bad DateTime", input.size() == 19 || input.size() == 20);

        if(input.size() == 19) {
            detail::compile_time_assert("bad DateTime", input.size() == 19);
            detail::compile_time_assert("bad DateTime", input[4] == '-');
            detail::compile_time_assert("bad DateTime", input[7] == '-');
            detail::compile_time_assert("bad DateTime", input[10] == 'T');
            detail::compile_time_assert("bad DateTime", input[13] == ':');
            detail::compile_time_assert("bad DateTime", input[16] == ':');

            std::ranges::copy(input, buffer.begin());
            buffer[10] = ' ';
        } else if(input.size() == 20) {
            detail::compile_time_assert("bad DateTime", input.size() == 20);
            detail::compile_time_assert("bad DateTime", input[3] == ' ');
            detail::compile_time_assert("bad DateTime", input[6] == ' ');
            detail::compile_time_assert("bad DateTime", input[11] == ' ');
            detail::compile_time_assert("bad DateTime", input[14] == ':');
            detail::compile_time_assert("bad DateTime", input[17] == ':');

            constexpr std::array MonthNames{std::string_view{"Jan 01"},
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
                }
                return input.substr(4, 1);
            }();
            auto const day2 = input.substr(5, 1);
            auto const year = input.substr(7, 4);
            auto const time = input.substr(12, 8);

            auto out = buffer.begin();
            auto add = [&](auto ss) { out = std::copy(std::begin(ss), std::end(ss), out); };

            add(year);
            add(std::string_view{"-"});
            add(month);
            add(std::string_view{"-"});
            add(day1);
            add(day2);
            add(std::string_view{" "});
            add(time);
        }
    }
};

struct Version {
    std::uint32_t major{};
    std::uint32_t minor{};
    std::uint32_t patch{};
};

}   // namespace CMakeGitVersion
