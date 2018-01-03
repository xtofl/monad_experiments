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

Index fromForm(FormInput input) { return { std::stoi(input.value.data()) }; }
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
    try {
        auto index = fromForm(input);
        auto v = toVoltage(fromIndex(index));
        return std::to_string(v.value).substr(0, 3) + "V";
    } catch (const std::invalid_argument &) {
        return std::string{"?"};
    }
}

int main(const int argc, const char** args)
{
    if (argc > 1)
        std::cout << toVoltageString({args[1]}) << std::endl;
    else
        std::cout << "?" << std::endl;
    assert(toVoltageString({"ZORK"}) == "?");
    assert(toVoltageString({ "90" }) == "9.1V");
    return 0;
}

