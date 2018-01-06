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

struct Foo { std::string bar; };
std::ostream &operator<<(std::ostream& out, const Foo &){ return out << "a Foo";}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

int runFSM(){
    using Interval = std::tuple<std::chrono::seconds, std::chrono::seconds>;
    struct Idle{};
    struct WaitForOpening{ std::vector<Interval> intervals; };
    struct OpenInterval{ std::vector<Interval> intervals; std::chrono::seconds started; };
    struct Ready{ std::vector<Interval> intervals; };

    using State = std::variant<Idle, WaitForOpening, OpenInterval, Ready>;

    struct FSM {
        State state = Idle{};
        bool running() const { return std::get_if<Ready>(&state) == nullptr; }
        void process(std::string token){
            if (token == "stop") state = Ready{{}};
        }
    } fsm;

    while(fsm.running()){
        std::string token;
        std::cin>>token;
        fsm.process(token);
    }
    std::cout << std::get<Ready>(fsm.state).intervals.size() << " intervals\n";
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