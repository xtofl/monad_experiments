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
        return  {{ std::stoi(input.value.data()) }};
    } catch( const std::invalid_argument &) {
        return {};
    }
}
Ratio fromIndex(Index i) {
    return { static_cast<double>(i.value) / 100.0 };
}
auto inRange(VoltageRange range) {
    return [=](Ratio r) { return Voltage{ range.low.value + r.value * (range.high.value - range.low.value) }; };
}
const VoltageRange range{ { 1.0 },{ 10.0 } };
const auto toVoltage = inRange(range);

template<typename T, typename Ft>
auto transform(const std::optional<T> &arg, Ft f) -> std::optional< decltype(f(*arg)) >
{
    if(arg) return {f(*arg)};
    else return {};
}
template<typename T>
auto flatten(const std::optional<std::optional<T>> &arg) -> std::optional<T> {
    if(arg) return *arg;
    else return {};
}

std::optional<Voltage> stringToVoltage(const std::string_view arg)
{
    const auto input = FormInput{ arg };
    const auto index = fromForm(input);
    const auto ratio = transform(index, fromIndex);
    const auto voltage = transform(ratio, toVoltage);
    return voltage;
}

std::string voltageToString(const Voltage &voltage) {
    return std::to_string(voltage.value).substr(0, 3) + "V";
}

int main(const int argc, const char** args)
{
    std::optional<std::string_view> arg;
    if(argc > 1) arg = args[1];
    const auto voltage = flatten(transform(arg, stringToVoltage));
    std::cout << transform(voltage, voltageToString).value_or("?") << std::endl;
    return 0;
}

