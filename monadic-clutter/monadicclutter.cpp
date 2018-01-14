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

auto toVoltageString(const std::vector<std::string_view> &args)
{
    const auto input = FormInput{ args.at(0) };
    const auto index = fromForm(input);
    const auto ratio = fromIndex(index);
    const auto voltage = toVoltage(ratio);
    return std::to_string(voltage.value).substr(0, 3) + "V";
}

int main(const int argc, const char** args)
{
    std::cout << toVoltageString({args[1]}) << std::endl;
    assert(toVoltageString({ "90" }) == "9.1V");
    return 0;
}

