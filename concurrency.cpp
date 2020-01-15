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

#include <thread>
#include <mutex>
TEST(concurrency, threads) {
    auto t = std::thread{}; // deault
    EXPECT_TRUE(std::thread::id{} == t.get_id());
    EXPECT_FALSE(t.joinable());

    int test = 0;
    auto do_stuff = [&test](int value){
        test = value;
        std::cout << "stuff: " << value << std::endl;
    };

    t = std::thread {do_stuff, 123};    // OS thread is constructed and imidiatly begins executing
    // constructore blocks, waiting for thread to execute -> not very efficient
    // any argument forwarded to constructed thread, must be evaluated in the target thread rather that the calling thread
    // calling stack is pegged until constructed thread is spun up.

    EXPECT_FALSE(std::thread::id{} == t.get_id());
    EXPECT_TRUE(t.joinable());

    auto handle = t.native_handle();
    std::cout << "handle: " << handle << std::endl;

    t.join();   // join t with calling thread; -> this is actually calling the destructor!
    // calling thread will block
    EXPECT_EQ(123, test);

    EXPECT_TRUE(std::thread::id{} == t.get_id());
    EXPECT_FALSE(t.joinable());

    handle = t.native_handle();
    std::cout << "handle: " << handle << std::endl;
    // thread does not handle the thread resources -> does not join or clean up reasources when e.g. goes out of scope
}