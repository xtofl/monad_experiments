// All copyrights and other intellectual property rights to software and any
// documents with which this notice is distributed or installed, shall, as
// between the parties, remain with TOMRA ASA, Tomra Sorting AS, Tomra Collection
// AS or any other Tomra subsidiary distributing the software. Use of such software
// and documents may only take place in accordance with the applicable licenses, and
// for the purpose of using the relevant Tomra product. The software and documents
// may not be duplicated, altered or made accessible to any third party.
/*
#include <gtest/gtest.h>

#include "csp/test/gtest/OutputSink.h"

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    csp::test::gtest::installBestMatchingOutputSink(*testing::UnitTest::GetInstance(), argc, argv);

    return RUN_ALL_TESTS();
}


#include <gtest/gtest.h>

*/


/* Monadic lifting.
 *
 * Copyright 2014 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <numeric>
#include <utility>
#include <functional>
#include <tuple>
#include <type_traits>

// http://bannalia.blogspot.be/2014/03/monadic-lifting-in-c.html
 
//////////////////////////////////////////////////////////////////////////
// make_index_sequence from http://stackoverflow.com/questions/19783205/
 
template <std::size_t ...>
struct index_sequence
{
    using type=index_sequence;
};

template <typename T>
using invoke = typename T::type;

template <typename T, typename U>
struct concate;

template <std::size_t ...I, std::size_t ...J>
struct concate<index_sequence<I...>, index_sequence<J...>>
    : index_sequence<I..., (J + sizeof ...(I))...>
{
};

template <std::size_t N>
struct make_index_sequence_help
    : concate<
          invoke<make_index_sequence_help<N/2>>,
          invoke<make_index_sequence_help<N-N/2>>>
{
};

template <>
struct make_index_sequence_help<0>
    : index_sequence<>
{
};

template <>
struct make_index_sequence_help<1>
    : index_sequence<0>
{
};

template <int N>
using make_index_sequence = invoke<make_index_sequence_help<N>>;
 

template <typename F, typename Tuple, size_t... I>
auto apply_pointer_tuple_impl(F &&f, Tuple &&t, index_sequence<I...>)
{
    return std::forward<F>(f)(*std::get<I>(std::forward<Tuple>(t))...);
}

template <typename F, typename Tuple>
auto apply_pointer_tuple(F &&f, Tuple &&t)
{
    constexpr auto tupleSize = std::tuple_size<typename std::decay<Tuple>::type>::value;
    using index_sequence = typename make_index_sequence<tupleSize>::type;
    
    return apply_pointer_tuple_impl(std::forward<F>(f), std::forward<Tuple>(t), index_sequence());
}
 
struct mvoid {};

template <template <typename> class M, class T>
M<T> mreturn(T &&t)
{
    return M<T>(std::forward<T>(t));
}
 
template <template <typename> class M, class T>
struct mwrapped
{
    using type = T;
};

template <template <typename> class M, class T>
struct mwrapped<M,M<T>>
{
    using type = T;
};

template <template <typename> class M, class T>
struct mwrapped<M, const M<T>>
{
    using type = const T;
};

template <template <typename> class M, class T>
using mwrapped_t = typename mwrapped<M,T>::type;
 
template <template <typename> class M, typename Context, std::size_t N>
struct mlifted_call_stage
{
    static const auto I = Context::num_args - N;

    mlifted_call_stage(Context &c)
        : c(c)
    {
    }

    auto operator()()
    {
        return operator()(*std::get<I>(c.args));
    }
   
    template <typename T>
    auto operator()(T &&t)
    {
        std::get<I>(c.postargs) = &t;
        mlifted_call_stage<M, Context,N-1> next(c);
        return next();
    }

    template <typename T>
    auto operator()(const M<T> &m)
    {
        return m >>= *this;
    }

    template <typename T>
    auto operator()(M<T> &m)
    {
        return m >>= *this;
    }

    Context &c;
};

template <template <typename> class M, typename Context>
struct mlifted_call_stage_end_base
{
    mlifted_call_stage_end_base(Context &c)
        : c(c)
    {
    }

    using return_type = decltype(std::declval<mlifted_call_stage_end_base>());
    static const bool is_void_return = std::is_void<return_type>::value;
 
    auto operator()()
    {
        return apply_pointer_tuple(c.f, c.postargs);
    }
 
    Context &c;
};

template <
    template <typename> class M,
    typename Context,
    bool isVoidReturn = mlifted_call_stage_end_base<M, Context>::is_void_return>
struct mlifted_call_stage_end;

template <template <typename> class M, typename Context>
struct mlifted_call_stage_end<M, Context, false>
    : mlifted_call_stage_end_base<M, Context>
{
    using super = mlifted_call_stage_end_base<M, Context>;
   
    mlifted_call_stage_end(Context &c)
        : super(c)
    {
    }

    auto operator()()
    {
        auto v = super::operator()();
        return mreturn<M>(std::move(v));
    }
};

template <template <typename> class M, typename Context>
struct mlifted_call_stage_end<M, Context, true>
    : mlifted_call_stage_end_base<M, Context>
{
    using super = mlifted_call_stage_end_base<M, Context>;

    mlifted_call_stage_end(Context &c)
        : super(c)
    {
    }

    auto operator()()
    {
        super::operator()();
        return mreturn<M>(mvoid());
    }
};

template <template <typename> class M, typename Context>
struct mlifted_call_stage<M, Context, 0>
    : mlifted_call_stage_end<M, Context>
{
    using super = mlifted_call_stage_end<M, Context>;

    mlifted_call_stage(Context &c)
        : super(c)
    {
    }
};
 
template <
    template <typename> class M,
    typename F,
    typename ...Args>
struct mlifted_call_context
{
    static const auto num_args = sizeof...(Args);

    mlifted_call_context(F& f,Args&... args)
        : f(f)
        ,args(std::make_tuple(&args...))
    {
    }
   
    auto operator()()
    {
        auto stage = mlifted_call_stage<M,mlifted_call_context,num_args>(*this);
        return stage();
    }
   
    F &f;
    std::tuple<typename std::remove_reference<Args>::type*...>               args;
    std::tuple<mwrapped_t<M, typename std::remove_reference<Args>::type>*...> postargs;
};
 
template <template <typename> class M, typename F>
struct mlifted
{
    mlifted(const F &f)
        :f(f)
    {
    }

    mlifted(F &&f)
        : f(std::move(f))
    {
    }
   
    template <typename ...Args>
    auto operator()(Args &&...args)
    {
        auto context = mlifted_call_context<M, F, Args...>(f, args...);
        return context();
    }

private:
    F f;
};
 
template <template <typename> class M, typename F>
mlifted<M,F> mlift(F &&f)
{
    return std::forward<F>(f);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


#include <algorithm>
#include <boost/optional.hpp>
#include <boost/none.hpp>
#include <iostream>
#include <vector>
 
template <typename T>
struct maybe:boost::optional<T>
{
    using super=boost::optional<T>;
    using super::super;
};

template <typename T, typename F>
auto operator>>=(const maybe<T>& m,F&& f)
{
    return m
        ? f(m.get())
        : boost::none;
}

template <typename T, typename F>
auto operator>>=(maybe<T>& m,F&& f)
{
    return m
        ? f(m.get())
        : boost::none;
}

std::ostream& operator<<(std::ostream& os,mvoid)
{
    return os<<"void";
}
 
template <typename T>
void dump(const maybe<T>& m)
{
    if (m)
        std::cout<<m.get()<<"\n";
    else
        std::cout<<"none\n";
}
 
template <typename T>
struct logged
{
    logged(const T& t):t(t){}
   
    T t;
};
 
template <typename T, typename F>
auto operator>>=(const logged<T>& m,F&& f)
{
    auto ret=f(m.t);
    std::cout<<m.t<<"->"<<ret.t<<"\n";
    return ret;
}
 
template <typename T, typename F>
auto operator>>=(logged<T>& m,F&& f)
{
    auto ret=f(m.t);
    std::cout<<m.t<<"->"<<ret.t<<"\n";
    return ret;
}
 
int foo(int x,int y){return x*y;}
void bar(int& x){x=190;}
 
int testLifting()
{
    auto mfoo=mlift<maybe>(&foo);
    dump(mfoo(maybe<int>(),maybe<int>(2)));
/*    const maybe<int> m=2;
    dump(mfoo(m,m));
    dump(mfoo(2,maybe<int>(2)));
    dump(mfoo(2,2));
    dump(mfoo(2,maybe<int>()));
    
    auto mbar=mlift<maybe>(&bar);
    maybe<int> mm=2;
    //mbar(mm);
    dump(mm);
    //dump(mbar(mm));
 
    auto lfoo=mlift<logged>(&foo);
    lfoo(logged<int>(2),logged<int>(3));

    std::vector<logged<int> > v;
    for (int i=0;i<10;++i)
        v.push_back(i);
    std::accumulate(v.begin(),v.end(),logged<int>(0),mlift<logged>(std::plus<int>()));*/

    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <boost/optional.hpp>
#include <boost/range.hpp>
#include <iterator>

namespace boost
{

    template <typename T>
    struct range_mutable_iterator<optional<T>>
    {
        using type = T*;
    };

    template <typename T>
    struct range_const_iterator<optional<T>>
    {
	    using type = const T*;
    };

}

/*
namespace boost
{
    template <typename T>
    T* begin(optional<T> &v)
    {
        return v ? &v.value() : nullptr;
    }

    template <typename T>
    const T* begin(const optional<T> &v)
    {
        return v ? &v.value() : nullptr;
    }

    template <typename T>
    T* end(optional<T> &v)
    {
        return v ? &v.value() + 1 : nullptr;
    }

    template <typename T>
    const T* end(const optional<T> &v)
    {
        return v ? &v.value() + 1 : nullptr;
    }
    
}
*/

namespace boost
{
    
    template <typename T>
    T* range_begin(optional<T> &v)
    {
        return v ? &v.value() : nullptr;
    }

    template <typename T>
    const T* range_begin(const optional<T> &v)
    {
        return v ? &v.value() : nullptr;
    }

    template <typename T>
    T* range_end(optional<T> &v)
    {
        return v ? &v.value() + 1 : nullptr;
    }

    template <typename T>
    const T* range_end(const optional<T> &v)
    {
        return v ? &v.value() + 1 : nullptr;
    }
    
    template <typename T>
    std::size_t range_calculate_size(optional<T> &v)
    {
        return v ? 1 : 0;
    }
    
    template <typename T>
    std::size_t range_calculate_size(const optional<T> &v)
    {
        return v ? 1 : 0;
    }
}

#include <csp/range/Actions.h>

namespace csp { namespace range { namespace actions
{
namespace details
{
    template<>
    struct InCompleteContainerContructor<boost::optional>
    {
        template<typename InputRange>
        static auto create(InputRange &&range)
        {
            using WrappedType = std::decay_t<decltype(*asRange(range).begin())>;
            return !std::empty(range)
                ? boost::make_optional(*asRange(range).begin())
                : boost::optional<WrappedType>();
        }
    };

} } } }

#include <boost/range/adaptor/transformed.hpp>
#include "csp/range/Adaptors.h"


template <typename T>
std::ostream& operator<<(std::ostream &ostr, const boost::optional<T> &v)
{
    return v
        ? ostr << v.get()
        : ostr << "boost::none";
}

struct FormInput { std::string input; };
struct Ratio { int n; int d; };
using Voltage = double;

boost::optional<FormInput> parseInput(std::string s)
{
    return boost::make_optional(FormInput{ s + " as input" });
}

Ratio toRatio(FormInput)
{
    return { 2, 3 };
}

boost::optional<Voltage> toVoltage(Ratio r)
{
    return r.n / static_cast<double>(r.d);
}



void testRange()
{
    const auto composed = parseInput("whatever")
        | boost::adaptors::transformed(toRatio)
        | boost::adaptors::transformed(toVoltage)
        | csp::range::actions::to<boost::optional>();
    const auto result = *composed;

    {
        auto result = boost::optional<int>(5)
            | boost::adaptors::transformed([](int v){ return v * 2; })
//            | csp::adaptors::joined(boost::optional<int>(15))
            | csp::range::actions::to<boost::optional>();
        auto r = *result;
        /* */
    }

    
    
    {
        /*
        auto result = boost::optional<int>()
            //| boost::adaptors::transformed([](int v){ return v * 2; })
            //| csp::adaptors::joined(boost::optional<int>(15))
            | csp::range::actions::to<boost::optional>();
        std::cout << "expected 10, actual: " << result << std::endl;
        /* */
    }
}

//////////////////////////////////////////////////////////////////////////


int main()
{
    testRange();

    return 0;
}
