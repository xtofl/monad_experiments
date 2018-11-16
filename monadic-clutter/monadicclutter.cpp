// monadicclutter.cpp : Defines the entry point for the console application.
//

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <type_traits>

struct FormInput { std::string_view value; }; 
struct Index { int value; };
struct Ratio { double value; };

struct Voltage { double value; };
struct VoltageRange { Voltage low; Voltage high; };

Index fromForm(FormInput input) {
    return { std::stoi(input.value.data()) };
}
Ratio fromIndex(Index i) {
    return { static_cast<double>(i.value) / 100.0 };
}
auto inRange(VoltageRange range) {
    return [=](Ratio r) { return Voltage{ range.low.value + r.value * (range.high.value - range.low.value) }; };
}
const VoltageRange range{ { 1.0 },{ 10.0 } };
const auto toVoltage = inRange(range);

auto formInput = [](auto arg)
{
    return FormInput{ arg };
};

auto stringToVoltage = [](auto arg)
{
    const auto input = formInput(arg);
    const auto index = fromForm(input);
    const auto ratio = fromIndex(index);
    const auto voltage = toVoltage(ratio);
    return voltage;
};

std::string voltageToString(const Voltage &voltage) {
    return std::to_string(voltage.value).substr(0, 3) + "V";
}

int main(const int argc, const char** args)
{
    {
        const auto voltage = stringToVoltage(std::string_view("1.1"));
        static_assert(std::is_same_v<decltype(voltage), Voltage)
        std::cout << voltageToString(voltage) << std::endl;
    }
    {
        std::optional<string_view> arg;
        if (argc > 1) arg = args[1];
        const auto voltage = stringToVoltage(arg);
        static_assert(std::is_same_v<decltype(voltage), std::optional<Voltage>>)
        std::cout << voltageToString(voltage) << std::endl;
    }
    {
        const auto voltage = stringToVoltage(std::vector<std::string_view>{"1.1", "1.2", "1.4"});
        static_assert(std::is_same_v<decltype(voltage), std::vector<Voltage>)
        std::cout << voltageToString(voltage) << std::endl;
    }
    return 0;
}

