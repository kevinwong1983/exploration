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

TEST(time, timeTest) {
    boost::posix_time::ptime now(boost::posix_time::second_clock::local_time());
    std::cout << "now: " << now << std::endl;

    std::string time_string("2019-03-01 0:0:1.000");
    boost::posix_time::ptime from_string(boost::posix_time::time_from_string(time_string));

    boost::posix_time::time_duration time_duration = (boost::posix_time::seconds(0));
    std::cout << "time_duration: " << time_duration << std::endl;

    boost::posix_time::time_duration ahead = (boost::posix_time::hours(1) + boost::posix_time::minutes(2) +
                                              boost::posix_time::seconds(3));
    std::cout << "ahead: " << from_string - ahead << std::endl;

    boost::posix_time::ptime ahead2 = boost::posix_time::second_clock::local_time();
    std::cout << "ahead2: " << ahead2 << std::endl;

    time_t ahead3 = time(NULL);
    std::cout << "ahead3: " << boost::posix_time::from_time_t(ahead3) << std::endl;
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
    auto valueLambda = [=]() {
        std::cout << x <<  std::endl;
    };
    auto refLambda = [&]() {
        std::cout << x <<  std::endl;
    };
    x = 13;
    valueLambda();
    refLambda();
}

TEST(promises, promiseUsingAsio) {
    std::promise<std::string> done;
    boost::asio::io_context io_context;

    std::function<void(int)> function = [&done](int a){
        done.set_value("hello");
    };

    boost::asio::deadline_timer timer(io_context, boost::posix_time::seconds(6));
    timer.async_wait(std::bind(function,2));

    std::thread thread([&io_context](){
        io_context.run();
    });

//    done.get_future().wait();
    std::cout << "\n>>1>>" << boost::posix_time::microsec_clock::local_time();
    std::cout << done.get_future().get() << std::endl;
    std::cout << "\n>>2>>" << boost::posix_time::microsec_clock::local_time();

    thread.join();
}

TEST(promises, promiseUsingAsio2) {
    boost::asio::io_context ioc;
    boost::asio::deadline_timer timer(ioc, boost::posix_time::seconds(5));

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

    //std::function<void<A, B, C> name = [&name](A a, B b, C c){};
    std::function<void(boost::system::error_code, std::shared_ptr<boost::asio::steady_timer>, std::shared_ptr<int>)> print =
            [&print](const boost::system::error_code& e, std::shared_ptr<boost::asio::steady_timer> t, std::shared_ptr<int> count)
    {
        std::cout << " hallo " << std::endl;

        if ((*count) > 0 && (e == boost::system::errc::success)){
            (*count)--;
            t->expires_at(t->expiry() + boost::asio::chrono::seconds(2)); // now plus 2 sec
            t->async_wait(boost::bind(print, boost::asio::placeholders::error, t, count));
        }
    };

    t->async_wait(boost::bind(print, boost::asio::placeholders::error, t, count));

    ioc.run();
    EXPECT_EQ(*count, 0);
}

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

int add(int a, int b) {
    return a+b;
}

TEST(bind, bindOnFunction) {
    auto add_func = std::bind(&add,std::placeholders::_1,std::placeholders::_2);
    std::cout << "\n" << add_func(1,2);
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
        std::unique_ptr<Entity> entity_1(new Entity); // dit word niet gebruikt door exception safety, als er exception plaats find in the constructor heb je een dangling pointer

        std::unique_ptr<Entity> entity_2 = std::make_unique<Entity>();
        // std::unique_ptr<Entity> e1 = entity;
        entity_2->Print();
    } // out of scope is the same as entity_2.reset();
    // unique pointer is out of scope and is removed
}

TEST(smartpointers, shared_pointer) {
    // unique pointer has preference to shared pointers!
    // only use shared pointer is you cannot use unique pointers
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
            EXPECT_EQ(2, shared.use_count());
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

    auto iterator2 = std::find_if(test_list.begin(), test_list.end(), [&compare](std::string& a){
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
////packed range-based for loop - shortcut for the previos one
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

int bind_free_function_arg = 0;
void bind_free_function() {
    std::cout<<"bind_function"<<std::endl;
    bind_free_function_arg ++;
}
void bind_free_function_with_arg(int& ref) {
    std::cout<<"bind_function_with_arg"<<std::endl;
    ref ++;
}
int bind_free_function_with_arg3(int a, int b, int c) {
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
    std::function<int(int,int)> binded_function = std::bind(bind_free_function_with_arg3, std::placeholders::_2, 3, std::placeholders::_1);
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
    signal_free_normal.connect(bind_free_function);
    signal_free_normal();
    EXPECT_EQ(1, bind_free_function_arg);

    // use a free function with args as a callback
    // connecting a binded free function to a signal .. if without arguments, its the same as previous
    auto signal_free_bind = boost::signals2::signal<void()>();
    auto /* std::function<void()> */ a = std::bind(bind_free_function);
    signal_free_bind.connect(a);
    signal_free_bind();
    EXPECT_EQ(2, bind_free_function_arg);

    // use a free function with args as a callback
    // connection a free function with args to a signal
    int count = 0;
    auto signal_free_with_arg_normal = boost::signals2::signal<void(int&)>();
    signal_free_with_arg_normal.connect(bind_free_function_with_arg);
    signal_free_with_arg_normal(count);
    EXPECT_EQ(1,count);

    // use a binded free function with args as a callback
    // connection a binded free function with args to a signal
    auto signal_free_with_arg_bind = boost::signals2::signal<void()>();
    auto b = std::bind(bind_free_function_with_arg, std::ref(count)); // here I binded the argument
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

    // connection a member function to a signal
    int signal_member_bind_with_arg_value = 0;
    auto signal_member_bind_with_arg = boost::signals2::signal<void()>();
    signal_member_bind_with_arg.connect(std::bind(&bind_class::bind_member_function_with_arg, &bc, std::ref(signal_member_bind_with_arg_value)));
    signal_member_bind_with_arg();
    EXPECT_EQ(1, signal_member_bind_with_arg_value);

    int signal_member_normal_with_arg_value = 0;
    auto signal_member_normal_with_arg = boost::signals2::signal<void(int&)>();
    signal_member_normal_with_arg.connect(boost::bind(&bind_class::bind_member_function_with_arg, &bc, _1)); // only works with boost!!!!
    signal_member_normal_with_arg(std::ref(signal_member_normal_with_arg_value));
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