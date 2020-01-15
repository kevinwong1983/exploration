#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#include <functional>
TEST(language_features, func) {
    std::function<bool(int)> test;
    test = [](int a) {return a>5;};
    auto b = test(4);
    std::cout << "b:" << b <<std::endl;
}

// normally you do not know what type this template returns.
// using decltype it is possible to return a value of type
// depending on the argument types.
template<typename X, typename Y>
auto multiply(X x, Y y) -> decltype(x * y)
{
    return x * y;
}

TEST(language_features, lamda) {
    // lamda:
    // [capture_block](parameter_list) mutable exception_spec -> return_type {body}

    auto a = multiply(3,2);
    EXPECT_EQ(a, 6);
}
