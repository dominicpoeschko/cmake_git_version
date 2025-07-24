#pragma once

#include "../core.hpp"

#include <array>
#include <string_view>

namespace CMakeGitVersion::detail {

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
                std::advance(it, 1);
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
static constexpr std::string_view toStringView{num_to_string<Number>::value.data(),
                                               num_to_string<Number>::value.size()};

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

}   // namespace CMakeGitVersion::detail
