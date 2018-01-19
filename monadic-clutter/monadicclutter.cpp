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
    return [=](Ratio r) -> std::optional<Voltage> {
        if (r.value < .0 || r.value > 1.0) return {};
        else return { { range.low.value + r.value * (range.high.value - range.low.value) } };
    };
}
const VoltageRange range{ { 1.0 },{ 10.0 } };
const auto toVoltage = inRange(range);

template<typename T>
auto flatten(const T &arg) {
    return arg;
}

template<typename T>
auto flatten(const std::optional<std::optional<T>> &arg) -> std::optional<T> {
    if (arg) return *arg;
    else return {};
}

template<typename T, typename Ft>
auto transform(const std::optional<T> &arg, Ft f) -> std::optional< decltype(f(*arg)) >
{
    if(arg) return {f(*arg)};
    else return {};
}

template<typename T, typename Ft>
auto flat_transform(const std::optional<T> &arg, Ft f) {
    return flatten(transform(arg, f));
}

template<typename F>
auto m(F f) {
    return [=](const auto &arg) {
        return flat_transform(arg, f);
    };
}

auto m_fromIndex = m(fromIndex);
auto m_toVoltage = m(toVoltage);

std::optional<Voltage> stringToVoltage(const std::string_view arg)
{
    const auto input = FormInput{ arg };
    const auto index = fromForm(input);
    const auto ratio = m_fromIndex(index);
    const auto voltage = m_toVoltage(ratio);
    return voltage;
}

std::string voltageToString(const Voltage &voltage) {
    return std::to_string(voltage.value).substr(0, 3) + "V";
}

int main(const int argc, const char** args)
{
    std::optional<std::string_view> arg;
    if(argc > 1) arg = args[1];
    const auto voltage = m(stringToVoltage)(arg);
    std::cout << m(voltageToString)(voltage).value_or("?") << std::endl;
    return 0;
}

