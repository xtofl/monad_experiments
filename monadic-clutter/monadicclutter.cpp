// monadicclutter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <type_traits>

#include "optional_monad/optional_monad.h"

#define assert(x) if(!(x)) { std::cerr << "fail: " << #x << "\n"; return 1; } \
else { std::cout << "OK: " << #x << "\n"; }

#define exit_if_not_equal(a, b) {if(a != b) { std::cerr << "fail: " << #a << "=" << (a) << " != " << #b << "\n"; return 1; } \
else { std::cout << "OK: " << #a << " == " << #b << "\n"; }}

struct FormInput { std::string_view value; }; 
struct Index { int value; };
struct Ratio { double value; };

struct Voltage { double value; };
struct VoltageRange { Voltage high; Voltage low; };

std::optional<Index> fromForm(FormInput input) {
    char *end;
    auto i = strtol(input.value.data(), &end, 10);
    if (end == input.value.data()) return {};
    else return { { i } };
}
Ratio fromIndex(Index i) {
    return { static_cast<double>(i.value) / 100.0 };
}
auto inRange(VoltageRange range) {
    return [=](Ratio r) { return Voltage{ range.low.value + r.value * (range.high.value - range.low.value) }; };
}
const VoltageRange range{ { 1.0 },{ 10.0 } };
const auto toVoltage = inRange(range);
namespace safe {
    auto inRange(VoltageRange range) {
        auto isSafe = [=](Ratio r) { return 0 <= r.value && r.value <= 1; };
        auto unChecked = ::inRange(range);
        return [=](Ratio r) -> std::optional<Voltage>{
            if (!isSafe(r)) return {};
            else return { unChecked(r) };
        };
    }
    const auto toVoltage = safe::inRange(range);
}

auto to_string(const Voltage &v) { return std::to_string(v.value).substr(0, 3) + "V"; }

std::string toVoltageString(const std::vector<std::string_view> &args)
{
    if (args.size() >= 1) {
        const auto input = FormInput{ args.at(0) };
        const auto index = fromForm(input);
        if (index) {
            auto v = safe::toVoltage(fromIndex(*index));
            if (v) {
                return to_string(*v);
            }
        }
    }
    return "?";
}

namespace fp {
    namespace optional_monad {
        auto make_optional = [](auto f) {
            return [=](auto x) {
                auto r = f(x);
                return std::make_optional(r);
            };
        };
        template<typename F, typename ...Fs>
        struct Composed {
            F f;
            using Rest = Composed<Fs...>;
            Rest rest;

            template<typename T>
            auto apply(T &&arg) const {
                auto first = f(arg);
                if(first) return rest.apply(*first);
                else return decltype(rest.apply(*first)){};
            }
        };
        template<typename F>
        struct Composed<F> {
            F f;
            template<typename T>
            auto apply(T &&arg) const { return f(arg); }
        };
        auto compose = [](auto ...fns) {
            const auto c = Composed<decltype(fns)...>{ fns... };
            return [=](auto &&x) {
                return c.apply(x);
            };
        };
        //auto compose = [](auto fn, auto ...fns) {
        //    constexpr if (sizeof(fns) == 0) {
        //        return fn;
        //    }
        //    else {
        //        auto rest = compose(fns...);
        //        return [=](auto x) { return rest(fn(x)); };
        //    }
        //};
    }
}


int main(const char** args, const int argc)
{
    assert(toVoltageString({ "90" }) == "1.9V");
    assert(toVoltageString({ }) == "?");
    assert(toVoltageString({ "not a number" }) == "?");
    assert(toVoltageString({ "200" }) == "?"); // out of bounds

    const auto composed = fp::optional_monad::compose(
        fromForm,
        fp::optional_monad::make_optional(fromIndex),
        safe::toVoltage,
        to_string);
    exit_if_not_equal(composed(FormInput{ "90" }), "1.9V");
    return 0;
}

