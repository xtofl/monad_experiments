// monadicclutter.cpp : Defines the entry point for the console application.
//

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>


struct FormInput { std::string_view value; }; 
struct Index { int value; };
struct Ratio { double value; };

struct Voltage { double value; };
struct VoltageRange { Voltage high; Voltage low; };

Index fromForm(FormInput input) { return { atoi(input.value.data()) }; }
Ratio fromIndex(Index i) {
    return { static_cast<double>(i.value) / 100.0 };
}
auto inRange(VoltageRange range) {
    return [=](Ratio r) { return Voltage{ range.low.value + r.value * (range.high.value - range.low.value) }; };
}
const VoltageRange range{ { 1.0 },{ 10.0 } };
const auto toVoltage = inRange(range);

auto precision(int p, double d) {
    std::ostringstream ss;
    ss.precision(2);
    ss << d;
    return ss.str();
}

auto toVoltageString(const std::vector<std::string_view> &args)
{
    const auto input = FormInput{ args.at(1) };
    auto v = toVoltage(fromIndex(fromForm(input)));
    return precision(2, v.value) + "V";
}

int main(const int argc, const char** arg)
{
    std::vector<std::string_view> args;
    std::copy_n(arg, argc, std::back_inserter(args));
    puts(toVoltageString(args).c_str());
    assert(toVoltageString({ "90" }) == "1.9V");
    assert(toVoltageString({ }) == "?");
    return 0;
}

