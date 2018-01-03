// monadicclutter.cpp : Defines the entry point for the console application.
//

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>


struct FormInput { std::string_view value; }; 
struct Index { int value; };
struct Ratio { double value; };
struct Voltage { double value; };
struct VoltageRange { Voltage low; Voltage high; };

std::optional<Index> fromForm(FormInput input) {
    try {
        return {{ std::stoi(input.value.data()) }};
    } catch(const std::invalid_argument &) {
        return {};
    }
};
std::optional<Ratio> fromIndex(Index i) {
    return {Ratio{static_cast<double>(i.value) / 100.0}};
}
auto inRange(VoltageRange range) {
    return [=](Ratio r) {
        if (r.value < 0.0 || 1.0 < r.value) return std::optional<Voltage>{};
        return std::make_optional(Voltage{ range.low.value + r.value * (range.high.value - range.low.value) });
    };
}
const VoltageRange range{ { 1.0 },{ 10.0 } };
const auto toVoltage = inRange(range);

template<typename F, typename...Fs>
auto compose(F f, Fs ...fs);

template<typename F>
auto compose(F f) { return f; }

template<typename F, typename...Fs>
auto compose(F f, Fs ...fs) {
        auto rest = compose(fs...);
        return [=](auto x){
            auto fx = f(x);
            using T = decltype(rest(*fx));
            return fx ? rest(*fx) : T{};
        };
}

auto voltageToString(Voltage v) { return std::to_string(v.value).substr(0, 3) + "V"; };
std::optional<FormInput> fromArgument(std::optional<std::string_view> s) {
    if(s)
        return {FormInput{*s}};
    else
        return {};
}

// auto toVoltageString = compose(
//     fromArgument,
//     fromForm,
//     fromIndex,
//     toVoltage,
//     voltageToString);

int main(const int argc, const char** args)
{
    std::optional<std::string_view> arg;
    if (argc > 1) arg = {args[1]};
    compose(fromArgument, fromForm)(arg);
    // std::cout << toVoltageString(arg) << std::endl;
    // assert(toVoltageString({}) == "?");
    // assert(toVoltageString({ "ZORK" }) == "?");
    // assert(toVoltageString({ "110" }) == "?");
    // assert(toVoltageString({ "90" }) == "9.1V");
    return 0;
}

