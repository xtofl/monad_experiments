// monadicclutter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>

#define assert(x) if(!(x)) { std::cerr << "fail: " << #x << "\n"; return 1; } \
else { std::cout << "OK: " << #x << "\n"; }


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

std::string toVoltageString(const std::vector<std::string_view> &args)
{
    if (args.size() < 1) return "?";
    const auto input = FormInput{ args.at(0) };
    const auto index = fromForm(input);
    if (!index) return "?";
    auto v = safe::toVoltage(fromIndex(*index));
    if (!v) return "?";
    return std::to_string(v->value).substr(0, 3) + "V";
}

int main(const char** args, const int argc)
{
    assert(toVoltageString({ "90" }) == "1.9V");
    assert(toVoltageString({ }) == "?");
    assert(toVoltageString({ "not a number" }) == "?");
    assert(toVoltageString({ "200" }) == "?"); // out of bounds
    return 0;
}

