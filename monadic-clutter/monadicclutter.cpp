// monadicclutter.cpp : Defines the entry point for the console application.
//

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <type_traits>
#include <optional>

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

namespace monads {
    struct Id {
        template<typename T> using M = T;

        template<typename T>
        static T mGet(T t) { return t; }

        template<typename FAR, typename MA>
        static auto mMap(FAR f, MA ma) { return f(mGet(ma)); }

        template<typename T>
        static T mReturn(T t) { return t; }

        template<typename T>
        static T mBind(T t) { return t; }
    };
    struct Optional
    {
        template<typename T> using M = std::optional<T>;

        template<typename T>
        static auto mGet(std::optional<T> mt) { return *mt; }

        template<typename FAR, typename A>
        static auto mMap(FAR f, std::optional<A> ma) {
            return mReturn(f(mGet(ma)));
        }

        template<typename T>
        static auto mReturn(T t) { return mBind(std::optional<T>{t}); }

        template<typename T>
        static auto mBind(std::optional<std::optional<T>> t) { return *t; }

        template<typename T>
        static auto mBind(std::optional<T> t) { return t; }
    };
}

template<typename M = monads::Id>
auto stringToVoltage(typename M::template M<FormInput> input)
{
    const auto index = M::mMap(fromForm, input);
    const auto ratio = M::mMap(fromIndex, index);
    const auto voltage = M::mMap(toVoltage, ratio);
    return voltage;
};

std::string voltageToString(const Voltage &voltage) {
    return std::to_string(voltage.value).substr(0, 3) + "V";
}

int main(const int argc, const char** args)
{
    {
        using M = monads::Id;
        using namespace std;
        static_assert(is_same_v<Voltage, decltype(M::mReturn(declval<Voltage>()))>);
        static_assert(is_same_v<Voltage, decltype(M::mMap(toVoltage, declval<Ratio>()))>);
        static_assert(is_same_v<Ratio, decltype(M::mMap(fromIndex, declval<Index>()))>);
        static_assert(is_same_v<Index, decltype(M::mMap(fromForm, declval<FormInput>()))>);
        static_assert(is_same_v<Voltage, decltype(M::mMap(stringToVoltage<M>, declval<FormInput>()))>);

        const auto voltage = stringToVoltage(FormInput{std::string_view("1.1")});
        std::cout << voltageToString(voltage) << std::endl;
    }
    {
        using M = monads::Optional;
        std::optional<FormInput> arg;
        if (argc > 1) arg = {std::string_view(args[1])};
        using namespace std;
        static_assert(is_same_v<optional<Voltage>, decltype(M::mReturn(declval<Voltage>()))>);
        static_assert(is_same_v<optional<Voltage>, decltype(M::mMap(toVoltage, declval<optional<Ratio>>()))>);
        static_assert(is_same_v<optional<Ratio>, decltype(M::mMap(fromIndex, declval<optional<Index>>()))>);
        static_assert(is_same_v<optional<Index>, decltype(M::mMap(fromForm, declval<optional<FormInput>>()))>);
        static_assert(is_same_v<optional<Voltage>, decltype(M::mMap(stringToVoltage<M>, declval<optional<FormInput>>()))>);

        const auto voltage = stringToVoltage<M>(arg);
        std::cout << voltageToString(*voltage) << std::endl;
    }
    return 0;
}

