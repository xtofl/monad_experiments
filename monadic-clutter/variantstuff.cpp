// monadicclutter.cpp : Defines the entry point for the console application.
//

#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include <iostream>
#include <chrono>
#include <tuple>
#include <iterator>
#include <ctime>

struct Foo { std::string bar; };
std::ostream &operator<<(std::ostream& out, const Foo &){ return out << "a Foo";}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

auto now() { return std::chrono::system_clock::now();}
using TimeStamp = decltype(now());
const TimeStamp programStart = now();
struct Interval { TimeStamp start; TimeStamp stop; };

auto time_string(const auto t){
    const auto timet = std::chrono::system_clock::to_time_t(t);
    std::string s(std::ctime(&timet));
    return s.substr(0, s.size() - 1);
}

namespace std {
std::ostream &operator<<(std::ostream &out, const Interval &i){
    return out << "interval[ "
        << time_string(i.start) << " - " << time_string(i.stop) << " ]";
}}

int runFSM(){
    struct WaitForOpening{ std::vector<Interval> intervals; };
    struct OpenInterval{ std::vector<Interval> intervals; TimeStamp started; };
    struct Ready{ std::vector<Interval> intervals; };

    using State = std::variant<WaitForOpening, OpenInterval, Ready>;
    struct FSM {
        State state = WaitForOpening{{}};
        bool running() const { return std::get_if<Ready>(&state) == nullptr; }
        void process(std::string token){
            state = std::visit(overloaded{
                [=](WaitForOpening w) -> State {
                    if (token == "stop") { return Ready{w.intervals}; }
                    if (token == "open") { return OpenInterval{w.intervals, now()}; }
                    return w;
                },
                [=](OpenInterval o) -> State  {
                    if (token == "close") {
                        o.intervals.push_back({o.started, now()});
                        return WaitForOpening{o.intervals}; }
                    return o;
                },
                [=](Ready r) -> State { return r;}
            }, state);
        }
    } fsm;

    while(fsm.running()){
        std::string token;
        std::cin>>token;
        fsm.process(token);
    }
    const auto &intervals = std::get<Ready>(fsm.state).intervals;
    std::cout << intervals.size() << " intervals\n";
    std::copy(begin(intervals), end(intervals), std::ostream_iterator<Interval>(std::cout, "\n"));
}

int main(){
    using Var = std::variant<int, double, Foo>;
    Var u;
    u = 5;


    if(0 == u.index()) { std::cout << std::get<0>(u) << " int\n"; }
    std::get<int>(u);
    if (auto p = std::get_if<int>(&u)){ std::cout << *p << " *int\n"; }

    std::visit([](auto x){ std::cout << x << "\n";}, u);
    struct V {
        void operator() (int){ std::cout << "int\n"; };
        void operator() (double){ std::cout << "double\n"; };
        void operator() (Foo){ std::cout << "Foo\n"; };
    } visitor;

    std::visit(visitor, u);

    std::visit([](auto u){
        if constexpr(std::is_same<int, decltype(u)>::value){ std::cout << "int!\n";}
    }, u);

    // the overload trick...
    std::visit(overloaded{
        [](int i){ std::cout << "INT!!!\n"; },
        [](double d){ std::cout << "DOUBLE!!!\n";},
        [](Foo foo){ std::cout << "FOO!!!\n";}
        }, u);

    runFSM();
}