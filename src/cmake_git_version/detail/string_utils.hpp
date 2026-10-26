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
            for(auto const v : viewsToAdd) { sum += v.size(); }
            return sum;
        }();

        std::array<char, arraySize> ret{};
        auto                        it = begin(ret);

        auto const addString = [&it](std::string_view s) {
            for(auto const c : s) {
                *it = c;
                std::advance(it, 1);
            }
        };

        for(auto const& v : viewsToAdd) { addString(v); }

        return ret;
    }

    static constexpr auto             buffer = gen();
    static constexpr std::string_view stringView{buffer.data(), buffer.size()};
};

template<typename F>
constexpr auto makeGetter(F) {
    struct X {
        static constexpr auto get() { return F{}(); }
    };

    return X{};
}

template<std::uint32_t... DigitCount>
struct ToChars {
    static constexpr std::array<char, sizeof...(DigitCount)> value{('0' + DigitCount)...};
};

template<std::uint32_t Rem, std::uint32_t... DigitCount>
struct Explode : Explode<Rem / 10, Rem % 10, DigitCount...> {};

template<std::uint32_t... DigitCount>
struct Explode<0, DigitCount...> : ToChars<DigitCount...> {};

template<std::uint32_t Num>
struct NumToString : Explode<Num> {};

template<>
struct NumToString<0> {
    static constexpr std::array<char, 1> value{'0'};
};

template<std::uint32_t Number>
static constexpr std::string_view toStringView{NumToString<Number>::value.data(),
                                               NumToString<Number>::value.size()};

template<Version V>
static constexpr std::string_view versionToStringView{StringViewMerge<decltype(makeGetter([]() {
    return std::array{
      toStringView<V.major>,
      std::string_view{"."},
      toStringView<V.minor>,
      std::string_view{"."},
      toStringView<V.patch>,
    };
}))>::stringView};

}   // namespace CMakeGitVersion::detail
