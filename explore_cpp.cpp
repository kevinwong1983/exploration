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

TEST(time, boost_posix_time) {
    //
    boost::posix_time::ptime now(boost::posix_time::second_clock::local_time());
    std::cout << "now: " << now << std::endl;

    boost::posix_time::time_duration time_duration = (boost::posix_time::seconds(0));
    std::cout << "time_duration: " << time_duration << std::endl;

    //
    std::string time_string("2019-03-01 0:0:1.000");
    boost::posix_time::ptime time_from_string(boost::posix_time::time_from_string(time_string));

    boost::posix_time::time_duration ahead = (boost::posix_time::hours(1) + boost::posix_time::minutes(2) +
                                              boost::posix_time::seconds(3));
    auto a = time_from_string - ahead;
    EXPECT_EQ(boost::posix_time::time_from_string("2019-Feb-28 22:57:58"), a);

    // same as: boost::posix_time::ptime local_time(boost::posix_time::second_clock::local_time());
    boost::posix_time::ptime local_time = boost::posix_time::second_clock::local_time();
    std::cout << "local_time: " << local_time << std::endl;

    // time struct
    time_t utc_time = time(NULL);
    std::cout << "utc_time: " << boost::posix_time::from_time_t(utc_time) << std::endl;
}

class Aap {
public:
    Aap(int a): a_(a) {};
    ~Aap() {};
    int getA() const {return a_;};

private:
    int a_;
};

TEST(find_if, simpleTest) {
    auto aapjes = std::list<std::unique_ptr<Aap>>();

    auto a = make_unique<Aap>(Aap(1));
    auto b = make_unique<Aap>(Aap(2));
    auto c = make_unique<Aap>(Aap(3));
    int match = 3;

    EXPECT_NE(a, nullptr);

    aapjes.push_back(std::move(a));
    aapjes.push_back(std::move(b));
    aapjes.push_back(std::move(c));

    EXPECT_EQ(a, nullptr);

    auto it = std::find_if(aapjes.begin(), aapjes.end(), [&match](std::unique_ptr<Aap>& aap){ return ((*aap).getA() == match); });
    if (it == aapjes.end()) {
        ASSERT_TRUE(false);
    }

    EXPECT_EQ((*it)->getA(), 3);
}

TEST(Lamda, refAndVal) {
    int x = 1;

    // all variable as copy
    auto valueLambda = [=]() {
        std::cout << x <<  std::endl;
    };

    // all variable as reference
    std::function<void(void)> /*auto*/ refLambda = [&]() {
        std::cout << x <<  std::endl;
    };

    x = 13;
    valueLambda();
    refLambda();
}

TEST(promises, promiseUsingAsio) {
    std::promise<std::string> done;
    boost::asio::io_context ioc;

    std::function<void(boost::system::error_code, int)> function = [&done](boost::system::error_code e, int a){
        EXPECT_EQ(a, 2);
        std::cout << "set hello: " << a << std::endl;
        done.set_value("hello");
    };

    // zelfde als boost::asio::deadline_timer timer(ioc, boost::posix_time::seconds(6);
    boost::asio::steady_timer timer(ioc, boost::asio::chrono::seconds(6));
    timer.async_wait(std::bind(function ,std::placeholders::_1, 2));

    std::thread thread([&ioc](){
        ioc.run();
    });

    // done.get_future().wait();
    std::cout << "\n>>1>>" << boost::posix_time::microsec_clock::local_time() << std::endl;;
    EXPECT_EQ(done.get_future().get(), "hello");
    std::cout << "\n>>2>>" << boost::posix_time::microsec_clock::local_time() << std::endl;;

    thread.join();
}

TEST(promises, promiseUsingAsio2) {
    boost::asio::io_context ioc;
    boost::asio::steady_timer timer(ioc, boost::asio::chrono::seconds(5));

    bool check = false;
    timer.async_wait(std::bind([&check](){
        check = true;
        std::cout << "timer expired" << std::endl;
    }));

    auto t = std::thread([&ioc](){
        ioc.run();
    });
    t.join();
    EXPECT_TRUE(check);
}

#include <boost/bind.hpp>

TEST(steady, steadytimer) {
    boost::asio::io_context io_context;

    auto function = [](int a){
        std::cout << "\n>>2>>" << boost::posix_time::microsec_clock::local_time();
    };

    boost::asio::steady_timer timer(io_context, boost::asio::chrono::seconds(6));
    timer.async_wait(std::bind(function, 1));

    std::cout << "\n>>1>>" << boost::posix_time::microsec_clock::local_time();
    io_context.run();
}

// lamda & recursion:
// cannot use auto:
// - the return type is not known at all.
// auto can't deduce the type. we know what the if statement does,
// returns 1, but we're not sure what the else statement would do.
// At least our compiler doesn't know
// - another reason is that the lambda function needs to be
// captured. To capture it, we just have to pass the same in
// the capture clause, [&]. The & says to pass everything as a reference.
TEST(promises, recursive_periodic_timers) {
    boost::asio::io_context ioc;
    auto count = std::make_shared<int>(3);
    auto t = std::make_shared<boost::asio::steady_timer>(ioc,  boost::asio::chrono::seconds(2));

    //std::function<int(A, B, C)> name = [&name](A a, B b, C c){return 5};
    std::function<void(boost::system::error_code, std::shared_ptr<boost::asio::steady_timer>, std::shared_ptr<int>)> print =
            [&print](const boost::system::error_code& e, std::shared_ptr<boost::asio::steady_timer> t, std::shared_ptr<int> count)
    {
        std::cout << " hallo " << std::endl;

        if ((*count) > 0 && (e == boost::system::errc::success)){
            (*count)--;
            t->expires_at(t->expiry() + boost::asio::chrono::seconds(2)); // now plus 2 sec
            // !!! hier gebruiken we boost::bind omdat we boost::asio::placeholders::error gebruiken. als je std::placeholder::_1 gebruikt, kan je ook std::bind doen
            t->async_wait(boost::bind(print, boost::asio::placeholders::error, t, count));
        }
    };

    t->async_wait(boost::bind(print, boost::asio::placeholders::error, t, count));

    ioc.run();
    EXPECT_EQ(*count, 0);
}

TEST(promises, recursive_periodic_timers_usingstdbind) {
    boost::asio::io_context ioc;
    auto count = std::make_shared<int>(3);
    auto timer = std::make_shared<boost::asio::steady_timer>(ioc, boost::asio::chrono::seconds(3));

    std::function<void(boost::system::error_code, std::shared_ptr<boost::asio::steady_timer>, std::shared_ptr<int>)> print =
            [&print](const boost::system::error_code &e, std::shared_ptr<boost::asio::steady_timer> t, std::shared_ptr<int> c) {
        std::cout << "hello" << std::endl;
        if (e == boost::system::errc::success && *c > 0) {
            (*c)--;
            t->expires_at(t->expiry() + boost::asio::chrono::seconds(2));
            t->async_wait(std::bind(print, std::placeholders::_1, t, c));
        }
    };

    timer->async_wait(std::bind(print, std::placeholders::_1, timer, count));
    auto thread = std::thread([&ioc](){
        ioc.run();
    });

    EXPECT_EQ(*count, 0);
    thread.join();
}

int add(int a, int b) {
    return a+b;
}

TEST(bind, bindOnFunction) {
    std::function<int(int,int)> add_func = std::bind(add,std::placeholders::_1,std::placeholders::_2);
    EXPECT_TRUE(add_func);
    if (add_func){
        std::cout << "blaa" << std::endl;
    }

    auto a = add_func(1,2);
    EXPECT_EQ(3, a);
}

TEST(functional, simpleTest) {
    struct myClass {
        int operator()(int a) {return a;}
    } myObject;

    int a = 10;
    int x = myObject(a);
    ASSERT_EQ(x,a);

    using TimerEventHandler = std::function<void(myClass)>;
}

TEST(hash, simple_hash) {
    boost::hash<std::string> string_hash;
    auto hash = string_hash("Hash me");
    std::cout << hash << std::endl;
}

class Entity {
public:
    Entity () {
        std::cout << "Created Entity" << std::endl;
    }

    ~Entity() {
        std::cout << "Destroyed Entitiy" << std::endl;
    }

    void Print () const {
        std::cout << "Print Entitiy" << std::endl;
    };
};

TEST(smartpointers, unique_pointer) {
    //unique pointer has preference to shared pointers!
    {
        std::unique_ptr<Entity> entity_1(new Entity); // !!! dit word niet gebruikt door exception safety, als er exception plaats find in the constructor heb je een dangling pointer

        std::unique_ptr<Entity> entity_2 = std::make_unique<Entity>();
        // std::unique_ptr<Entity> e1 = entity;
        entity_2->Print();
    } // out of scope is the same as entity_2.reset();
    // unique pointer is out of scope and is removed
}

TEST(smartpointers, shared_pointer) {
    // unique pointer has preference to shared pointers!
    // only use shared pointer if you cannot use unique pointers
    {
        std::shared_ptr<Entity> e1;
        EXPECT_EQ(0, e1.use_count());
        {
            // std::shared_ptr<Entity> entity_1(new Entity());
            // dit kan, maar wordt niet gebruikt: eerst word entity geheugen geallocceerd,
            // dan pas wordt shared pointer geheugen geallocceerd. makeshared doet dat in een keer
            // wat veel efficienter is.

            std::shared_ptr<Entity> shared_entity = std::make_shared<Entity>();
            std::cout << "shared_entity:" << shared_entity.use_count() << std::endl;
            std::cout << "e1:" << e1.use_count() << std::endl;
            EXPECT_EQ(1, shared_entity.use_count());
            EXPECT_EQ(0, e1.use_count());

            e1 = shared_entity;
            std::cout << "shared_entity:" << shared_entity.use_count() << std::endl;
            std::cout << "e1:" << e1.use_count() << std::endl;
            EXPECT_EQ(2, shared_entity.use_count());
            EXPECT_EQ(2, e1.use_count());
        }
        std::cout << "e1:" << e1.use_count() << std::endl;
        EXPECT_EQ(1, e1.use_count());
        //pointer is alive!
    }
    // pointer is destoyed
}

TEST(Lamda, shared_pointer_and_lamda) {
    {
        std::function<void(int)> lamda_captured;
        {
            std::shared_ptr<Entity> shared;
            EXPECT_EQ(0, shared.use_count());
            {
                shared = std::make_shared<Entity>();
                EXPECT_EQ(1, shared.use_count());

                lamda_captured = [shared](int t){ // shared pointer captured in lamda -> increase use count of shared ptr
                    shared->Print();
                    EXPECT_EQ(shared.use_count(), t);
                };

                lamda_captured(2);
                EXPECT_EQ(shared.use_count(), 2);
            }
            lamda_captured(2);
            EXPECT_EQ(shared.use_count(), 2);
        }
        lamda_captured(1);
    }
    // Entity destroyed!
}

TEST(Lamda, shared_pointer_and_lamda_capture_by_ref) {
    {
        std::function<void(int)> lamda_captured;
        {
            std::shared_ptr<Entity> shared;
            EXPECT_EQ(0, shared.use_count());
            {
                shared = std::make_shared<Entity>(); //Created Entity
                EXPECT_EQ(1, shared.use_count());

                lamda_captured = [&shared](int t){ // shared pointer captured in lamda by ref-> does not increase use count of shared ptr

                    shared->Print();
                    std::cout << shared.use_count() << std::endl;
                    EXPECT_EQ(shared.use_count(), t);
                };

                lamda_captured(1); // Print Entitiy
                EXPECT_EQ(shared.use_count(), 1);
            }
            lamda_captured(1); // Print Entitiy
            EXPECT_EQ(1, shared.use_count());
            shared.reset();
            EXPECT_EQ(0, shared.use_count());
        }  // Entity destroyed!
        lamda_captured(0);
    }
}

int Pass(std::function<int(int)> lamda, int arg) {
    return lamda(arg);
}

TEST(Lamda, passing_a_lamda) {
    std::function<int(int)> lamda = [](int a){
        std::cout << " lamda called " << std::endl;
        return 2*a;
    };

    EXPECT_EQ(Pass(lamda, 2), 4);
}

TEST(Lamda, reference_to_a_shared_pointer_does_not_increase_ref_count) {
    {
        std::shared_ptr<Entity> shared;
        EXPECT_EQ(0, shared.use_count());
        {
            shared = std::make_shared<Entity>();
            EXPECT_EQ(1, shared.use_count());

            auto lamda_captured = [&shared](int t){
                shared->Print();
                EXPECT_EQ(shared.use_count(), t);
            };

            lamda_captured(1);                      //reference of shared pointer does not increase ref count
            EXPECT_EQ(shared.use_count(), 1);
        }
        EXPECT_EQ(1, shared.use_count());
    }
}

TEST(smartpointers, weak_pointer) {
    // weak pointer does not increase the ref count. This is great when you dont
    // want to take ownership of the object .e.g. when you are storing a list of
    // reference of entities and you dont care if they are valid or not. need to check
    // if they are alive, but does not keep them alive
    std::weak_ptr<Entity> weak_entity;
    std::shared_ptr<Entity> s = weak_entity.lock();
    EXPECT_EQ(weak_entity.use_count(), 0);
    EXPECT_EQ(s.use_count(), 0);
    {
        std::shared_ptr<Entity> shared_entity = std::make_shared<Entity>();
        EXPECT_EQ(shared_entity.use_count(), 1);
        EXPECT_EQ(weak_entity.use_count(), 0);

        weak_entity = shared_entity;
        EXPECT_EQ(shared_entity.use_count(), 1); // does not increase
        EXPECT_EQ(weak_entity.use_count(), 1);

        // Creates a new std::shared_ptr that shares ownership of the managed object
        s = weak_entity.lock();
        EXPECT_EQ(shared_entity.use_count(), 2); // now does increase
        EXPECT_EQ(weak_entity.use_count(), 2);
        std::cout << "s:" << s.use_count() << std::endl;
    }
    EXPECT_EQ(weak_entity.use_count(), 1);
    // pointer is still alive
    s.reset();
    EXPECT_EQ(weak_entity.use_count(), 0);
    // pointer is destoyed
}

TEST(review, lists) {
    std::vector<std::string> test_list;
    test_list.emplace_back("hallo 1");
    test_list.emplace_back("hallo 2");
    test_list.emplace_back("hallo 3");
    test_list.emplace_back("hallo 4");
    test_list.emplace_back("hallo 5");
    test_list.emplace_back("hallo 6");

    const std::string compare = "hallo 3";
    const auto iterator = std::find(test_list.begin(),test_list.end(), compare);
    auto index = iterator - test_list.begin();
    EXPECT_EQ(*iterator, compare);

    // auto iterator2 = std::find_if(test_list.begin(), test_list.end(), [&compare](std::string& a){
    auto iterator2 = std::find_if(std::begin(test_list), std::end(test_list), [&compare](std::string& a){
        return (0 == a.compare(compare));
    });
    EXPECT_EQ(*iterator, compare);;
}

TEST(async, simple) {
    int a = 10;
    int b = 10;

    // std::async vs std::threads?
    // One simple reason I've found is the case when you want a
    // way to detect (via polling) whether an asynchronous job is done.
    // With std::thread, you have to manage it yourself. With std::async
    // you can query std::future::valid()
    // (or use std::future::wait_for/wait_until(...)) to know when it is done.
    auto future = std::async(std::launch::deferred, [&] {
        while (a < 1000) {
            std::cout << " a : " << ++a << std::endl;
        }
        return a;
    });

    future.get();

    while (b < 1000) {
        std::cout << " b : " << ++b << std::endl;
    }

    //future.get();
}

//// traditional index based - good for vectorized environments
//for(std::size_t i=0; i<v.size(); ++i)
//std::cout << v[i] << '\n';
//
////iterator based range - emulated C pointer shift
//for(auto t=v.begin(); t!=v.end(); ++t)
//std::cout << *t << '\n';
//
////packed range-based for loop - shortcut for the previous one
//for(auto&& x: v)
//std::cout << x << '\n';
//
////functional based - for hidden inside std::for_each
//std::for_each(
//        v.begin(), v.end(),
//[&](auto&& x){ std::cout << x << '\n'; });
//
////algorithm based - for hidden inside std::copy
//std::copy(
//        v.begin(), v.end(),
//        std::ostream_iterator<int>(std::cout, "\n"));
TEST(programming, simple_match_in_lists) {
  std::vector<std::string> a, b, c;
  a.emplace_back("a");
  a.emplace_back("b");
  a.emplace_back("c");
  a.emplace_back("d");
  a.emplace_back("e");

  b.emplace_back("d");
  b.emplace_back("e");
  b.emplace_back("f");
  b.emplace_back("g");
  b.emplace_back("h");

  for(auto a_local: a){
    for(auto b_local: b){
        if ( a_local == b_local ) {
            c.emplace_back(a_local);
            std::cout << a_local << std::endl;

        }
    }
  }

  EXPECT_NE(std::find(begin(c), end(c), "e"), c.end());
  EXPECT_NE(std::find(begin(c), end(c), "d"), c.end());

  c.empty();
  std::for_each(begin(a), end(a), [&](auto&& a_local){
     std::for_each(begin(b), end(b), [&](auto&& b_local){
         if ( a_local == b_local ) {
             c.emplace_back(a_local);
         }
     });
  });
  EXPECT_NE(std::find(begin(c), end(c), "e"), c.end());
  EXPECT_NE(std::find(begin(c), end(c), "d"), c.end());
}

TEST(simple, simple1) {
    std::vector<std::vector<int>> a;
    a.emplace_back(std::vector<int>{1,2,3,4,5});
    a.emplace_back(std::vector<int>{6,7,8,9,0});

    EXPECT_EQ(a[0][0], 1);
    EXPECT_EQ(a[0][1], 2);
    EXPECT_EQ(a[0][2], 3);
    EXPECT_EQ(a[0][3], 4);
    EXPECT_EQ(a[0][4], 5);

    EXPECT_EQ(a[1][0], 6);
    EXPECT_EQ(a[1][1], 7);
    EXPECT_EQ(a[1][2], 8);
    EXPECT_EQ(a[1][3], 9);
    EXPECT_EQ(a[1][4], 0);
}

#include <boost/signals2.hpp>

int free_function_arg = 0;
void free_function() {
    std::cout<<"bind_function"<<std::endl;
    free_function_arg ++;
}
void free_function_with_arg(int& ref) {
    std::cout<<"bind_function_with_arg"<<std::endl;
    ref ++;
}
int free_function_with_arg3(int a, int b, int c) {
    std::cout<<"bind_function_with_arg"<<std::endl;
    return a-b-c;
}

bool bind_class_exist = false;
class bind_class {
public:

    bind_class(){
        bind_class_exist = true;
        std::cout<<"constucted the bind class"<<std::endl;
    }
    ~bind_class(){
        bind_class_exist = false;
        std::cout<<"destructed the bind class"<<std::endl;
    }

    void bind_member_function(){
        std::cout<<"bind_member_function"<<std::endl;
        count_ ++;
    }
    void bind_member_function_with_arg(int& ref){
        std::cout<<"bind_member_function"<<std::endl;
        ref ++;
    }
    int count_ = 0;
};

TEST(dummy, binding) {
    // understanding the return value of a bind... its just a std::function<>
    std::function<int(int,int)> binded_function = std::bind(free_function_with_arg3, std::placeholders::_2, 3,
                                                            std::placeholders::_1);
    auto val = binded_function(1, 10); // 1 = c, 10 = a, 3 = b
    EXPECT_EQ(6, val);

    // binding of lambda
    std::function<int(int,int)> lambda = [](int a, int b){
        return a + b;
    };
    EXPECT_EQ(4, lambda(1,3));
    std::function<int(int)> binded_lambda = std::bind(lambda, std::placeholders::_1, std::placeholders::_1);
    EXPECT_EQ(6, binded_lambda(3));

    // use a free function as a callback
    // connection a free function to a signal
    auto signal_free_normal = boost::signals2::signal<void()>();
    signal_free_normal.connect(free_function);
    signal_free_normal();
    EXPECT_EQ(1, free_function_arg);

    // use a free function with args as a callback
    // connecting a binded free function to a signal .. if without arguments, its the same as previous
    auto signal_free_bind = boost::signals2::signal<void()>();
    auto /* std::function<void()> */ a = std::bind(free_function);
    signal_free_bind.connect(a);
    signal_free_bind();
    EXPECT_EQ(2, free_function_arg);

    // use a free function with args as a callback
    // connection a free function with args to a signal
    int count = 0;
    auto signal_free_with_arg_normal = boost::signals2::signal<void(int&)>();
    signal_free_with_arg_normal.connect(free_function_with_arg);
    signal_free_with_arg_normal(count); // this has benefit that I can signal with a argument
    EXPECT_EQ(1,count);

    // use a binded free function with args as a callback
    // connection a binded free function with args to a signal
    auto signal_free_with_arg_bind = boost::signals2::signal<void()>();
    auto b = std::bind(free_function_with_arg, std::ref(count)); // here I binded the argument
    signal_free_with_arg_bind.connect(b);
    signal_free_with_arg_bind(); // here I dont have to give argument
    EXPECT_EQ(2,count);

    // use a member function as a callback -> this is where I really need bind
    // connection a binded member function to a signal
    auto bc = bind_class();
    auto signal_member_bind = boost::signals2::signal<void()>();
    signal_member_bind.connect(std::bind(&bind_class::bind_member_function, &bc));  // bc needs to be a pointer, otherwise it just created a copy
    signal_member_bind();
    EXPECT_EQ(1,bc.count_);

    // connection a member function with args to a signal
    int signal_member_bind_with_arg_value = 0;
    auto signal_member_bind_with_arg = boost::signals2::signal<void()>();
    signal_member_bind_with_arg.connect(std::bind(&bind_class::bind_member_function_with_arg, &bc, std::ref(signal_member_bind_with_arg_value)));
    signal_member_bind_with_arg();
    EXPECT_EQ(1, signal_member_bind_with_arg_value);

    int signal_member_normal_with_arg_value = 0;
    auto signal_member_normal_with_arg = boost::signals2::signal<void(int&)>();
    signal_member_normal_with_arg.connect(boost::bind(&bind_class::bind_member_function_with_arg, &bc, _1)); // only works with boost!!!!
    signal_member_normal_with_arg(std::ref(signal_member_normal_with_arg_value)); // this has benefit that I can signal with a argument
    EXPECT_EQ(1, signal_member_normal_with_arg_value);
}

TEST(dummy, binding_to_member_scoping) {
    int count = 0;
    EXPECT_EQ(false, bind_class_exist);
    {
        auto signal_member_normal_with_arg = boost::signals2::signal<void(int&)>();
        EXPECT_EQ(false, bind_class_exist);
        {
            std::shared_ptr<bind_class> bc;
            EXPECT_EQ(false, bind_class_exist);
            EXPECT_EQ(0, bc.use_count());

            bc = std::make_shared<bind_class>();
            EXPECT_EQ(1, bc.use_count());
            EXPECT_EQ(true, bind_class_exist);

            signal_member_normal_with_arg.connect(boost::bind(&bind_class::bind_member_function_with_arg, bc, _1));  // increased use_count of smart pointer
            EXPECT_EQ(2, bc.use_count());
            EXPECT_EQ(true, bind_class_exist);
        }
        EXPECT_EQ(true, bind_class_exist); // object still exists!!
        signal_member_normal_with_arg(count); // this is save :D
    }
    EXPECT_EQ(false, bind_class_exist); // now that signal is out of scope, object is gone.
    EXPECT_EQ(1, count);
}

TEST(dummy, bool_to_int_cast) {
    bool a = true;
    bool b = true;
    bool c = false;
    bool d = false;

    EXPECT_EQ(a + b, 2);
    EXPECT_EQ(c + d, 0);
}

TEST(dummy, char_to_string_cast) {
    const char* original_string = "hello wereld";

    void* payload = (void*) original_string;
    int payload_len = sizeof("hello wereld");

    char b[payload_len];
    int t = 0;
    while (t < payload_len) {
        b[t] = ((char*)payload)[t];
        t++;
    }

    EXPECT_EQ(std::string(b), "hello wereld");
}

// wrap code that might throw an exception in a try block
// try/catch as close to the problem as possible
// add one or more catch blocks after the try
// catch more speficif exception first
// catch exception by reference
// error types derived from std::exception => these are "marker classes" only think what they have is a "name"
// - logic_error
//      domain_error
//      invalid_argument
//      length_error
//      out_of_range
// - runtime_error
//      overflow_error
//      range_error
//      underflow_error
TEST(dummy, exceptions) {
    try {
        auto a = std::vector<int>();
        a.emplace_back(1);

        auto b = a.at(99);
        std::cout << "b:" << b << std::endl;
    }
    catch (std::out_of_range& e) { // exception hyrarchie
        std::cout << "e:" << e.what() << std::endl;
    }
    catch (std::exception& e) { // catch exception by reference! otherwise we experience slicing of derived exceptions
        std::cout << "e:" << e.what() << std::endl;
    }
}

class MqttError : public std::runtime_error {
public:
    MqttError( const std::string &what ) :
            std::runtime_error( "MqttError: " + what ) {}
};

bool destructed_a = false;
bool destructed_b = false;
class Vos {
public:
    Vos(int a): a_(a) {
        std::cout << "Vos constructor" << a_ << std::endl;
        if (a == 0) {
            throw std::invalid_argument("Arbitrary number for a Vos cannot be zero");
        }
    };
    ~Vos() {
        std::cout << "Vos destructor" << a_ << std::endl;
        if (a_ == 0) {
            destructed_a = true;
        }
        if (a_ == 2) {
            destructed_b = true;
        }
    };
    void ThrowStuf() {
        throw std::invalid_argument("Stuf is thrown.");
    };
    void ThrowMqttError() {
        throw MqttError("sum ting wong");
    };
private:
    int a_;
};

TEST(dummy, throw_exceptions) {
    bool done = false;
    Vos c(1);
    try {
        // everything local to try goes out of scope
        // thus if using pointers, alway use smart pointer
        // this will call destructor when out of scope.
        Vos b(2);   // destructor called
        Vos a(0);   // exception thrown in constructor, destructor not called!
        done = true;
    }
    catch (std::out_of_range& e) { // exception hyrarchie
        std::cout << "e:" << e.what() << std::endl;
    }
    catch (std::exception& e) {
        std::cout << "e:" << e.what() << std::endl;
    }
    EXPECT_FALSE(done);
    EXPECT_FALSE(destructed_a);
    EXPECT_TRUE(destructed_b);

    // ........
    destructed_a = false;
    destructed_b = false;
    try {
        Vos b(2);   // destructor called
        b.ThrowStuf();  // exception thown out side of constructor
    }
    catch (std::exception& e) {
        std::cout << "e:" << e.what() << std::endl;
    }
    EXPECT_TRUE(destructed_b);
}

TEST(dummy, throw_custom_exceptions) {
    bool done = false;
    try {
        Vos b(2);   // destructor called
        b.ThrowMqttError();  // exception thown out side of constructor
    }
    catch (MqttError& e) {
        std::cout << "MqttError>>>>:" << e.what() << std::endl;
        bool done = true;
    }
    catch (std::exception& e) {
        std::cout << "e:" << e.what() << std::endl;
    }
    EXPECT_TRUE(done);
}

class DefaultArgument {
public:
    void TestFuntion(const std::string& a = "asdf" ) const {
        std::cout << "string is: " << a << std::endl;
    }
};
TEST(dummy, default_argument_for_const_ref) {
    DefaultArgument a;
    a.TestFuntion();
    a.TestFuntion("ASDASD");
}

TEST(dummy, pairs) {
    auto a = std::make_pair(1,2);
}

class Animal {
public:
    virtual ~Animal() = 0;      // item 7. Polymorphic base classes should declare virtual destructors. If a class has any virtual functions, it should have a virtual destructor
    virtual std::string walk() const = 0;
    virtual std::string talk() const = 0;
};

Animal::~Animal(){
    std::cout << __func__ << std::endl;
}

class Cow : public Animal {
public:
    Cow(int legs) : legs_(legs) {}

    ~Cow() {
        std::cout << __func__ << std::endl;
    }

    // copy constructor/assignment operator is needed when you have a
    // non-copy-able variable in your class, and you want to use it.
    Cow(const Cow &that) : legs_(that.legs_) {
        notify_state_change_.connect(that.notify_state_change_);
    }

    Cow &operator=(const Cow &that) {
        legs_ = that.legs_;
        notify_state_change_.connect(that.notify_state_change_);
        return *this;
    }

    std::string walk() const override {
        return "walks with " + std::to_string(legs_) + " legs";
    }

    std::string talk() const override {
        return "BOOOO";
    }

    void registers (std::function<void(void)> a) {
        notify_state_change_.connect(a);
    }

    void call () {
        notify_state_change_();
    }

private :
    int legs_;
    boost::signals2::signal<void(void)> notify_state_change_;
};

class Ape : public Animal {
public:
    Ape(int legs):legs_(legs){}
    ~Ape(){
        std::cout << __func__ << std::endl;
    }

    std::string  walk() const override {
        return "walks with " + std::to_string(legs_) + " legs";
    }
    std::string  talk() const override {
        return "hohoho";
    }
private :
    int legs_;
};

class AnimalFactory {
public:
    static std::shared_ptr<Animal> get(const std::string& type, int legs) {
        if (type == "cow"){
            return std::make_shared<Cow>(legs);
        }
        else {
            return std::make_shared<Ape>(legs);
        }
    }
};

TEST(dummy, copy_constructor) {
    int x = 0;
    std::function<void(void)> f = [&x](){
        x++;
    };

    Cow c(4);
    c.registers(f);

    auto d = c;
    EXPECT_EQ("walks with 4 legs", d.walk());
    EXPECT_EQ("BOOOO", d.talk());
    EXPECT_EQ("walks with 4 legs", c.walk());
    EXPECT_EQ("BOOOO", c.talk());

    c.call();
    EXPECT_EQ(1, x);
    d.call();
    EXPECT_EQ(2, x);
}

TEST(dummy, virtual_functions) {
    Cow c(4);
    EXPECT_EQ("walks with 4 legs", c.walk());
    EXPECT_EQ("BOOOO", c.talk());

    // assignment operator
    auto d = c;
    EXPECT_EQ("walks with 4 legs", d.walk());
    EXPECT_EQ("BOOOO", d.talk());

    // copy operator
    auto e(c);
    EXPECT_EQ("walks with 4 legs", e.walk());
    EXPECT_EQ("BOOOO", e.talk());

    auto f = new Cow(4);
    delete(f);

    std::shared_ptr<Animal> g = AnimalFactory::get("cow", 4);
    EXPECT_EQ("walks with 4 legs",  g->walk());
    EXPECT_EQ("BOOOO",  g->talk());
    std::shared_ptr<Animal> h = AnimalFactory::get("ape", 2);
    EXPECT_EQ("walks with 2 legs",  h->walk());
    EXPECT_EQ("hohoho",  h->talk());

    std::vector<Cow> inner;
    std::generate_n(std::back_inserter(inner), 5, [x = 0]() mutable { // this uses a copy of what is returned to the vector
        Cow c(3);
        x++;
        return c;   // returned is then copied to vector
    });
}

int Get_RValue(){
    return 10;  // this function returns a rvalue
}
int& Get_LValue(){
    static int value = 10;
    return value;  // this function returns a rvalue
}
void SetValue1(int value){
}
void SetValue2(int& value){
}
void SetValue3(const int& value){
}
TEST(dummy, lvalue_rvalue_rules) {
    // lvalue on left of equal sign, rvalue on right of equal sign -> however this does not always apply
    int i = 10; // i is variable with location in memory, 10 is value which has no storage or location.

    // you cannot assign anything to an rvalue e.g.
    // 10 = i;

    int j = Get_LValue();   // GetValue() return rvalue (which is a temporay value with not location or storage)
    // Get_LValue() = 5;    // assignment of rvalue does not work!
    Get_LValue() = 5;       // assignment of lvalue works!

    SetValue1(10);    // assignment of rvalue, this rvalue will be used to create an lvalue when called
    SetValue1(i);           // assignment of lvalue

    // we can easily see which on is temp. value and which one is not
    // RULE: you cannot take a lvalue ref from an rvalue;   i.e. you can only have lvalue ref of an lvalue

    // SetValue2(10);       // will not work. you cannot take lvalue ref of rvalue
    // int& a = 10;         // also gives error: non-const lvalue reference to type 'int' cannot bind to a temporary of type 'int'
    SetValue2(i);           // however assingment of lvalue works

    // SPECIAL RULE:
    // while I cannot have lvalue ref of rvalue, I can have an CONST lvalue ref I can!!!
    const int& a = 10;      // this works!
    // what compiler does: creates an temporary variable.
    // i.e. int temp = 10; const int& a = temp;

    // const int& accepts boths rvalue and lvalue!
    SetValue3(i);
    SetValue3(10);
}

void PrintName1(std::string& name) {        // only accepts lvalue
    std::cout << "[lvalue]" << name << std::endl;
}
void PrintName2(const std::string& name) {  // accepts both lvalue and rvalue!
    std::cout << name << std::endl;
}
void PrintName3(std::string&& name) {       // accepts both lvalue and rvalue!
    std::cout << "[rvalue]" <<name << std::endl;
}

TEST(dummy, lvalue_rvalue_with_strings) {
    std::string firstName = "Kevin";
    std::string lastName = "Wong";
    std::string fullName = firstName + lastName;    // temp string is created from firstName + lastName, which is then assigned to lvalue
    // what is lvalue, what is rvalue?
    // everything on left side is lvalue, everything on right side is rvalue

    // rvalue temp string is created from firstName + lastName
    PrintName1(fullName);  // rvalue: works!
    // PrintName1(firstName + lastName);   // lvalue: does not work, because its an rvalue, candidate function not viable: expects an l-value for 1st argument

    PrintName2(fullName);  // rvalue works!
    PrintName2(firstName + lastName);  // rvalue: works!

    // can we write function that only accepts rvalue object?
    // YES: use a rvalue ref &&
    // PrintName3(fullName);  // lvalue: does not accepts!
    PrintName3(firstName + lastName);   // rvalue: works!

    // advantage is with optimization, if we know we are taking in a temporary object, than we do not need to worry about ie keeping object alive
    // and keep it intact (by coping it). We can just steal the resources that are attached to the object and use them somewhere else, because
    // know it is temporary.

    // summary:
    // 1. lvalue are value which have starage backing them
    // 2. rvalue are tempory values
    // 3. lvalue refs & can only take in lvalues, unless they are const
    // 4. rvalue refs && can only take in temporay rvalues
}

class String {
public:
    String() = default;
    String(const char* string) {
        std::cout << __func__ << " created" << std::endl;

        size_ = strlen(string);
        data_ = new char[size_];
        memcpy(data_, string, size_);
    }
    String(const String& other) {
        std::cout << __func__ << " copied" << std::endl;

        size_ = other.size_;
        data_ = new char[size_];
        memcpy(data_, other.data_, size_);
    }

    String (String&& other) noexcept {
        std::cout << __func__ << " moved" << std::endl;
        // take ownership of other's data
        size_ = other.size_;
        data_ = other.data_;

        //create a hollow object from other
        other.size_ = 0;
        other.data_ = nullptr;
    }

    ~String() {
        std::cout << __func__ << " destroyed" << std::endl;

        delete data_;
    }

    void Print() {
        for (uint32_t i = 0; i < size_; i++) {
            std::cout << data_[i];
        }
        std::cout << std::endl;
    }
private:
    char* data_;
    uint32_t size_;

};

class MyEntity {
public:
    MyEntity (const String& name) : m_Name(name) {
        std::cout << __func__ <<" created" << std::endl;
    }

    MyEntity (String&& name) : m_Name(std::move(name)) {
        std::cout << __func__ << " moved" << std::endl;
    }

    void PrintName(){
        m_Name.Print();
    }
private:
    String m_Name;
};

TEST(dummy, move_semantics) {
    std::cout << "1" << std::endl;
    MyEntity entity("Kevin"); // "Kevin" is a temporay rvale String. It is created/allocated and then Copied in copy constructor!
    std::cout << "2" << std::endl;
    entity.PrintName();

    // we are able to create/allocaed here and then MOVE it.
}

#include <numeric>

TEST(core_guidelines, P_3) {    // express your intent
    std::vector<int> numbers{4, 5, 6, 9, -2, 27, 14, 99};

    // this does not show intent, only a for loop. A reviewer never knows it is doing what you wanted it to do.
    int total = 0;                                  // classic loop
    for (int i = 0; i < numbers.size(); i++) {
        total += numbers[i];
    }
    EXPECT_EQ(162, total);
    // here we need to check number of things:
    // 1. i starts from 0,
    // 2. i smaller than appropriate number,
    // 3. i ++ in loop or in for etc
    // 4. does total start from 0

    // already better but still does not show intent:
    total = 0;
    for (int element : numbers) {                   // range loop
        total += element;
    }
    EXPECT_EQ(162, total);

    // also better but still does not show intent:
    total = 0;
    std::for_each(begin(numbers), end(numbers), [&total](int element){
        total += element;
    });
    EXPECT_EQ(162, total);

    // finally this shows intent:
    total = std::accumulate(begin(numbers), end(numbers), 0);
    EXPECT_EQ(162, total);
}

class Account {
public:
    bool Deposite(double dollars) {
        balanceInPennies += static_cast<int>(dollars * 100);
        return true;
    }

    bool Withdraw(double dollars) {
        if (balanceInPennies < static_cast<int>(dollars * 100)) {
            return false;
        }

        balanceInPennies -= static_cast<int>( dollars * 100);
        return true;
    }
    double getBalance() const {
        serviceChargesInPennies = 700; // actually a more complicated calculation
        // error: Cannot assign to non-static data member
        // within const member function 'getBalance' member
        // function 'Account::getBalance' is declared const here

        // I can cast away const:
        // never do this! this is a lie.
        // Account* This = const_cast<Account*> (this);
        // This->serviceChargesInPennies = 700; // act

        // what we should do is make serviceChargesInPennies mutable

        return (balanceInPennies-serviceChargesInPennies) / 100;
    }
private:
    int balanceInPennies = 0;
    int mutable serviceChargesInPennies = 0; // mutable keyword examps this variable to be changed in const functions
};

TEST(core_guidelines, ES_50) { // never cast away const, instead use mutable
    Account a;
    a.Deposite(10);
    EXPECT_EQ(3,a.getBalance());
}

class Manupilation {
public:
    Manupilation(Account* a) : account(a) {};
    bool takeMoney(double dollars) {
        if (account) {      // null check
            return account->Deposite(dollars);
        }
        return false;
    }
    double confirmAssets() const {
        if (account) {      // null check
            return account->getBalance();
        }
        return 0.0; // this is wierd
    }
private:
    Account* account;
};

TEST(core_guidelines, I_12) { // declare a pointer that must not be null as not_null
    std::stringstream ss;
    auto i = 234;
    ss << std::setw(1) << std::setfill('0') << i;
    std::string s = ss.str();

    std::cout << s ;
}

#include <sstream>
#include <iomanip>

TEST(printing_digits, prefix) {
    char randomTag[5]{};
    memset(randomTag, 'H', sizeof(randomTag));
    std::ostringstream ss;

    std::cout << sizeof(randomTag) << std::endl;
    ss << randomTag;

    std::cout << ss.str() << std::endl;

    auto myString = std::string(randomTag);
    std::cout << myString << std::endl;
}

#include <queue>

using Done = std::function<void(void)>;
using Callback = std::function<void(Done)>;
using Command = std::function<void(void)>;
boost::asio::io_context io_context;

class Duck {
public:
    Duck(std::string name) : name_{name} {
    }

    void Quack(Done done) {
        auto timer = std::make_shared<boost::asio::steady_timer>(io_context, boost::asio::chrono::seconds(2));
        timer->async_wait( std::bind([this, timer, done](){
            std::cout << name_ << " said Quack! at " << boost::posix_time::microsec_clock::local_time() << std::endl;
            done();
        }));
        std::cout << name_ << " scheduled at " << boost::posix_time::microsec_clock::local_time() << std::endl;
    }

private:
    std::string name_;
};

class CommandQueue {
public:
    void Push(Callback callback) {
        queue_.push([this, callback]() {
            callback(std::bind(&CommandQueue::exe, this));
        });
    }

    void Execute() {
        if (allowExecute_ == true) {
            exe();
        }
    }

private:
    void exe() {
        if (!queue_.empty()) {
            allowExecute_ = false;
            auto command = queue_.front();
            queue_.pop();
            command();
        } else {
            allowExecute_ = true;
        }
    }

    std::queue<Command> queue_;
    bool allowExecute_ = true;
};

class Simulator {
public:
    void Simulate(std::vector<std::shared_ptr<Duck>> &ducks) {
        for (auto &&duck: ducks) {
            queue_.Push( std::bind(&Simulator::MakeItQuack, this, duck, std::placeholders::_1));
        }
        queue_.Execute();
    }

    void MakeItQuack(std::shared_ptr<Duck> duck, Done done) {
        duck->Quack(std::move(done));
    }

private:
    CommandQueue queue_;
};

TEST(queue, simple_queue) {
    auto simulator = Simulator();

    std::vector<std::shared_ptr<Duck>> ducks1;
    auto d1 = std::make_shared<Duck>("daan");
    auto d2 = std::make_shared<Duck>("gijs");
    auto d3 = std::make_shared<Duck>("mier");
    ducks1.push_back(d1);
    ducks1.push_back(d2);
    ducks1.push_back(d3);
    simulator.Simulate(ducks1);

    std::vector<std::shared_ptr<Duck>> ducks2;
    auto d4 = std::make_shared<Duck>("hek");
    auto d5 = std::make_shared<Duck>("au");
    auto d6 = std::make_shared<Duck>("vos");
    ducks2.push_back(d4);
    ducks2.push_back(d5);
    ducks2.push_back(d6);
    simulator.Simulate(ducks2);

    io_context.run();
}


class Child;

class GrandGrandParent : public std::enable_shared_from_this<GrandGrandParent> {
public:
    virtual ~GrandGrandParent() = default;
};

class GrandParent : public GrandGrandParent {
};

class Parent : public GrandParent {
public:
    void SetChild(std::shared_ptr<Child> child);

private:
    std::weak_ptr<Child> c_;
};

class Child {
public:
    void SetParent(std::shared_ptr<Parent> parent) {
        p_ = parent;
    }

private:
    std::shared_ptr<Parent> p_;
};

void Parent::SetChild(std::shared_ptr<Child> child) {
    c_ = child;
    c_.lock()->SetParent(std::dynamic_pointer_cast<Parent>(shared_from_this()));
}

// Finally, there are two points to note:
// 1. Using std::dynamic_pointer_cast<T>() requires a virtual function in the base class,
// This is because this conversion function uses the input type and the target type whether
// there is a virtual function with the same signature as an indicator of whether the
// conversion can be successful. The simplest and correct solution is to declare the destructor
// in the base class as a virtual function. To
// 2. Cannot use shared_form_this() in the constructor. This is because std::enable_share_from_this
// uses an object's weak_ptr in its implementation, and this weak_ptr needs the object's shared_ptr
// to initialize. Since the object has not yet been constructed at this time, an exception of
// std::bad_weak_ptr will be thrown. There is currently no perfect solution for this. You can try
// to write an init() function, which is called manually after the object is constructed. Or write
// a std::shared_ptr<Derived>(this) manually, but this solution may cause circular references.
// For more solutions, please refer to StackOverFlow.

TEST(compound_patterns, make_shared_from_this) {

    auto parent = std::make_shared<Parent>();
    auto child = std::make_shared<Child>();

    parent->SetChild(child);
}

#include "sqlite3.h"

TEST(static_casting, static_casting) {
    sqlite3 * connection = nullptr;

    int result = sqlite3_open(":memory:", &connection);

    if (SQLITE_OK != result)
    {
        std::cout << std::string(sqlite3_errmsg(connection));
        sqlite3_close(connection);
    }
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for(i = 0; i<argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

static void exec(sqlite3* connection, char* sql) {
    char *zErrMsg = 0;
    int rc;

    /* Execute SQL statement */
    rc = sqlite3_exec(connection, sql, callback, 0, &zErrMsg);

    if( rc != SQLITE_OK ){
        std::cout << "SQL error: " << zErrMsg << " return code: " << rc << std::endl;
        sqlite3_free(zErrMsg);
    } else {
        std::cout << "execute successfull" << std::endl;
    }
}

TEST(sql, create) {
    auto path = "/Users/user/CLionProjects/experimental/database.db";
    char *zErrMsg = 0;
    int rc;
    char *sql;
    sqlite3 * connection = nullptr;

    rc = sqlite3_open(path , &connection);

    if( rc ) {
        std::cout << "Can't open database: " << sqlite3_errmsg(connection) << std::endl;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    /************** Create SQL statement **************/
    sql = "CREATE TABLE COMPANY("  \
      "ID INT PRIMARY KEY     NOT NULL," \
      "NAME           TEXT    NOT NULL," \
      "AGE            INT     NOT NULL," \
      "ADDRESS        CHAR(50)," \
      "SALARY         REAL );";
    exec(connection, sql);

    sqlite3_close(connection);
}

TEST(sql, read) {
    auto path = "/Users/user/CLionProjects/experimental/database.db";
    char *zErrMsg = 0;
    int rc;
    char *sql;
    sqlite3 * connection = nullptr;

    rc = sqlite3_open(path , &connection);

    if( rc ) {
        std::cout << "Can't open database: " << sqlite3_errmsg(connection) << std::endl;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    sql = "SELECT * from COMPANY";
    exec(connection, sql);

    sqlite3_close(connection);
}

TEST(sql, open) {
    auto path = "/Users/user/CLionProjects/experimental/database.db";
    char *zErrMsg = 0;
    int rc;
    char *sql;
    sqlite3 * connection = nullptr;

    rc = sqlite3_open(path , &connection);

    if( rc ) {
        std::cout << "Can't open database: " << sqlite3_errmsg(connection) << std::endl;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    /************** BEGIN **************/
    sql = "BEGIN TRANSACTION ";
    exec(connection, sql);

    /************** Create SQL statement **************/
    sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
         "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
         "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
         "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
         "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";
    exec(connection, sql);
    /************** Create SQL statement **************/
    sql = "SELECT * from COMPANY";
    exec(connection, sql);
    /************** Create merged SQL statement **************/
    sql = "UPDATE COMPANY set SALARY = 25000.00 where ID=1; " \
         "SELECT * from COMPANY";
    exec(connection, sql);
    /************** Create merged SQL statement **************/
    sql = "UPDATE COMPANY set SALARY = 25000.00 where ID=1; " \
         "SELECT * from COMPANY";
    exec(connection, sql);
    /************** Create merged SQL statement **************/
    sql = "DELETE from COMPANY where ID=2; " \
         "SELECT * from COMPANY";
    exec(connection, sql);

    /************** COMMIT **************/
    sql = "COMMIT ";
    exec(connection, sql);

    sqlite3_close(connection);
}


TEST(sql, recommit) {
    auto path = "/Users/user/CLionProjects/experimental/database.db";
    char *zErrMsg = 0;
    int rc;
    char *sql;
    sqlite3 * connection = nullptr;

    rc = sqlite3_open(path , &connection);

    if( rc ) {
        std::cout << "Can't open database: " << sqlite3_errmsg(connection) << std::endl;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    /************** BEGIN **************/
    sql = "BEGIN TRANSACTION ";
    exec(connection, sql);

    /************** Create SQL statement **************/
    sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
         "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
         "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
         "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
         "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";
    exec(connection, sql);
    /************** Create SQL statement **************/
    sql = "SELECT * from COMPANY";
    exec(connection, sql);
    /************** Create merged SQL statement **************/
    sql = "UPDATE COMPANY set SALARY = 25000.00 where ID=1; " \
         "SELECT * from COMPANY";
    exec(connection, sql);
    /************** Create merged SQL statement **************/
    sql = "DELETE from COMPANY where ID=2; " \
         "SELECT * from COMPANY";
    exec(connection, sql);

    /************** COMMIT **************/
    sql = "BEGIN TRANSACTION "; // SQL error: cannot start a transaction within a transaction return code: 1
    exec(connection, sql);

    sql = "COMMIT ";
    exec(connection, sql);

    sql = "COMMIT ";
    exec(connection, sql); // SQL error: cannot commit - no transaction is active return code: 1

    sqlite3_close(connection);
}
