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

TEST(threads, conditional_variable) {
    std::mutex m;
    std::condition_variable c;
    int data = 0;
    std::atomic<bool> run(true);

    auto t1 = std::thread([&](){
        while (run) {
            std::cout << "T1 start" << std::endl;
            std::unique_lock<std::mutex> lk(m);
            c.wait(lk, [&data, &run] { return !run || data == 1; });
            EXPECT_EQ(1, data);
            std::cout << "T1 end" << std::endl;
            data = 2;

            lk.unlock();
            c.notify_all();
        }
    });
    auto t2 = std::thread([&](){
        while (run) {
            std::cout << "T2 start" << std::endl;
            std::unique_lock<std::mutex> lk(m);
            c.wait(lk, [&data, &run] { return !run || data == 2; });
            EXPECT_EQ(2, data);
            std::cout << "T2 end" << std::endl;
            data = 3;

            lk.unlock();
            c.notify_all();
        }
    });
    auto t3 = std::thread([&](){
        while (run) {
            std::cout << "T3 start." << std::endl;
            std::unique_lock<std::mutex> lk(m);
            c.wait(lk, [&data, &run]{return !run || data == 3;});
            EXPECT_EQ(3, data);
            std::cout << "T3 end" << std::endl;
            data = 1;

            lk.unlock();
            c.notify_all();
        }
    });
    auto t4 = std::thread([&](){
        std::this_thread::sleep_for(std::chrono::seconds(2));
        run = false;
        std::lock_guard<std::mutex> guard(m);
        c.notify_all();
    });

    std::cout << "START" << std::endl;
    {
        std::lock_guard<std::mutex> lk(m);
        data = 1;
        c.notify_all();
    }

    std::cout << "END" << std::endl;
    t1.join();
    std::cout << "END1" << std::endl;
    t2.join();
    std::cout << "END2" << std::endl;
    t3.join();
    std::cout << "END3" << std::endl;
    t4.join();
    std::cout << "JOINED" << std::endl;
}


#include <thread>
#include <iostream>
#include <atomic>
#include <vector>
using namespace std;

const unsigned int NTHREADS = 20;
const int ITERS = 10000000;

atomic<int> counter;

void increment()
{
    for(int i=0; i<ITERS; i++)
        counter++;
}

void decrement()
{
    for(int i=0; i<ITERS; i++)
        counter--;
}

TEST(threads, atomic) {
    vector<thread> vt;

    cout << "The counter is " << counter << endl;

    for(unsigned int i=0; i<NTHREADS; i++)
        if(i%2==0)
            vt.push_back(thread(increment));
        else
            vt.push_back(thread(decrement));

    for(thread &t : vt)
        t.join();

    cout << "The counter is " << counter << endl;;
}


#include <atomic>
int accum = 0;
//atomic<int> accum(0);
mutex accum_mutex;
void square(int x) {
    // The first thread that calls lock() gets the lock.
    // During this time, all other threads that call lock(),
    // will simply halt, waiting at that line for the mutex to be unlocked.
//    accum_mutex.lock();
    std::lock_guard<std::mutex> guard (accum_mutex);
    accum += x * x;
//    accum_mutex.unlock();
}

TEST(threads, race_condition) {
    for (int f = 0; f <1000; f++){
        accum = 0;
        vector<thread> ths;
        for (int i = 1; i <= 20; i++) {
            ths.push_back(thread(&square, i));
        }

        for (auto& th : ths) {
            th.join();
        }
        cout << "accum = " << accum << endl;
        EXPECT_EQ(2870, accum);
    }

}