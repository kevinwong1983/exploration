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

    boost::posix_time::time_duration ahead = (boost::posix_time::hours(1)+ boost::posix_time::minutes(2) + boost::posix_time::seconds(3));
    std::cout << "ahead: " << from_string - ahead << std::endl;

    boost::posix_time::ptime ahead2 = boost::posix_time::second_clock::local_time();
    std::cout << "ahead2: " << ahead2 << std::endl;

    time_t ahead3 = time(NULL);
    std::cout << "ahead3: " << boost::posix_time::from_time_t(ahead3) << std::endl;
}

TEST(FactorialTest, Zero) {
    EXPECT_EQ(1, 1);
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

    auto function = [&done](int a){
        done.set_value("hello");
    };

    boost::asio::deadline_timer timer(io_context, boost::posix_time::seconds(6));
    timer.async_wait(std::bind(function,2));

    std::thread thread([&io_context](){
        std::cout << "\n>>1>>" << boost::posix_time::microsec_clock::local_time();
        io_context.run();
    });

//    done.get_future().wait();
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
// - cannot use auto: the return type is not known at all.
// auto can't deduce the type. we know what the if statement does,
// returns 1, but we're not sure what the else statement would do.
// At least our compiler doesn't know
// - Another the reason is that the lambda function needs to be
// captured. To capture it, we just have to pass the same in
// the capture clause, [&]. The & says to pass everything as a reference.
TEST(promises, periodic_timers) {
    boost::asio::io_context ioc;
    auto count = std::make_shared<int>(3);
    auto t = std::make_shared<boost::asio::steady_timer>(ioc,  boost::asio::chrono::seconds(2));

    std::function<void(boost::system::error_code, std::shared_ptr<boost::asio::steady_timer>, std::shared_ptr<int>)> print =
            [&print](const boost::system::error_code& e, std::shared_ptr<boost::asio::steady_timer> t, std::shared_ptr<int> count)
    {
        std::cout << " hallo " << std::endl;

        if ((*count) > 0 && (e == boost::system::errc::success)){
            (*count)--;
            t->expires_at(t->expiry() + boost::asio::chrono::seconds(2));
            t->async_wait(boost::bind(print, boost::asio::placeholders::error, t, count));
        }
    };

    t->async_wait(boost::bind(print, boost::asio::placeholders::error, t, count));

    ioc.run();
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

TEST(bind, bindoFunction) {
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
        std::unique_ptr<Entity> entity_1(new Entity); // dit word niet gebruikt door exception savety, als er exception plaats find in the constructor heb je een dangling pointer

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
        std::cout << "e1:" << e1.use_count() << std::endl;
        EXPECT_EQ(0, e1.use_count());
        {
            // dit kan, maar wordt niet gebruikt: eerst word entity geheugen geallocceerd,
            // dan pas wordt shared pointer geheugen geallocceerd. makeshared doet dat in een keer
            // wat veel efficienter is.
            // std::shared_ptr<Entity> entity_1(new Entity());

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
    } // Entity destroyed!
}

TEST(Lamda, shared_pointer_and_lamda2) {
    {
        std::function<void(int)> lamda_captured;
        {
            std::shared_ptr<Entity> shared;
            EXPECT_EQ(0, shared.use_count());
            {
                shared = std::make_shared<Entity>(); //Created Entity
                EXPECT_EQ(1, shared.use_count());

                lamda_captured = [&shared](int t){ // shared pointer captured in lamda -> does not increase use count of shared ptr

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
    std::cout << "weak_entity:" << weak_entity.use_count() << std::endl;
    {
        std::shared_ptr<Entity> shared_entity = std::make_shared<Entity>();
        std::cout << "shared_entity:" << shared_entity.use_count() << std::endl;
        std::cout << "weak_entity:" << weak_entity.use_count() << std::endl;

        weak_entity = shared_entity;
        std::cout << "shared_entity:" << shared_entity.use_count() << std::endl;
        std::cout << "weak_entity:" << weak_entity.use_count() << std::endl;

        // Creates a new std::shared_ptr that shares ownership of the managed object
        s = weak_entity.lock();
        std::cout << "shared_entity:" << shared_entity.use_count() << std::endl;
        std::cout << "weak_entity:" << weak_entity.use_count() << std::endl;
        std::cout << "s:" << s.use_count() << std::endl;
    }
    std::cout << "weak_entity:" << weak_entity.use_count() << std::endl;
    // pointer is still alive
    s.reset();
    std::cout << "weak_entity:" << weak_entity.use_count() << std::endl;
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

    const auto iterator = std::find(test_list.begin(),test_list.end(),"hallo 123");
    auto index =  iterator - test_list.begin();
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

struct Hen
{
    char const * m_name;

    Hen() : m_name("") {
        std::cout<< "default constructor" << std::endl;
    }

    explicit Hen(char const * name) : m_name(name) {
        std::cout<< "explicit constuctor" << std::endl;
    }

    Hen(Hen const& other) : m_name(other.m_name) {
        std::cout<< "copy constuctor with name " << m_name << std::endl;
    }

    Hen(Hen && other) : m_name(other.m_name) {
        other.m_name = "null";
        std::cout<< "move constuctor with name " << m_name << std::endl;
    }

    ~Hen() {
        std::cout<< "destuctor with name " << m_name  << " " << id_ << " " << eggs_ << std::endl;
    }

    auto operator = (Hen const & other) -> Hen& {
        m_name = other.m_name;
        std::cout<< "copy assingment " << m_name << std::endl;
        return *this;
    }

    auto operator = (Hen && other) -> Hen& {
        m_name = other.m_name;
        other.m_name = "null";
        std::cout << "move assignment " << m_name << std::endl;
        return *this;
    }

    void swap (Hen & other){
        std::swap(m_name, other.m_name);
    }

    Hen(unsigned id, float eggs):
        id_(id), eggs_(eggs), m_name("") {
        std::cout << "custom constructor " << id_ << " " << eggs_ << std::endl;
    }

    unsigned id_;
    float eggs_;
};

TEST(modern_cpp, unique_pointers) {

  auto hen1 = make_unique<Hen>(1, 2.2f);
  EXPECT_TRUE(hen1);

  auto hen2 = move(hen1); // move ownership of hen1 to hen2
  EXPECT_FALSE(hen1);     //
  EXPECT_TRUE(hen2);

  Hen& hen_ref1 = *hen1;
  //Hen hen_copy1 = *hen1;

  Hen& hen_ref2 = *hen2;
  Hen hen_copy2 = *hen2;
  EXPECT_EQ(hen_ref2.eggs_, 2.2f);
  EXPECT_EQ(hen_ref2.id_, 1);

  hen1.reset(hen2.release());
  EXPECT_TRUE(hen1);
  EXPECT_FALSE(hen2);

  // dit blijft geldig. het referentie wijst nog steeds
  // naar geheugen plek dat geldig is.
  EXPECT_EQ(hen_ref2.eggs_, 2.2f);
  EXPECT_EQ(hen_ref2.id_, 1);

  hen1.reset();

  // WHY????
  EXPECT_EQ(hen_ref2.eggs_, 2.2f);
  EXPECT_EQ(hen_ref2.id_, 1);
}

// transfer from ownership of pointer in an out of function explicit
auto GenHen(int id, float eggs) -> std::unique_ptr<Hen>
{
    return std::make_unique<Hen> (id, eggs);// this does a MOVE!!!!
    // same as: return move(std::make_unique<Hen> (id, eggs));
};

auto UpdateHen(std::unique_ptr<Hen> hen) -> std::unique_ptr<Hen>
{
    hen->eggs_ += 1.8f;
    return hen; // dit is een move
}

TEST(modern_cpp, unique_pointers2) {
  auto hen1 = GenHen(6, 3.3f);
  // UpdateHen(hen1); // dit gaat fout!
  auto hen2 = UpdateHen(move(hen1));
  EXPECT_EQ(hen2->eggs_, 3.3f+1.8f);
}

TEST(modern_cpp, shared_pointers1) {
    auto sp = std::shared_ptr<int> {};

    EXPECT_EQ(nullptr, sp);
    EXPECT_EQ(0, sp.use_count());
    EXPECT_FALSE(sp.unique());

    sp = std::make_shared<int>(123);

    EXPECT_NE(nullptr, sp);
    EXPECT_EQ(1, sp.use_count());
    EXPECT_TRUE(sp.unique());

    auto sp2 = sp; // this already copies the pointer and increases the ref count

    EXPECT_NE(nullptr, sp);
    EXPECT_EQ(2, sp.use_count());
    EXPECT_FALSE(sp.unique());

    EXPECT_TRUE(sp.get() == sp2.get()); // zelfde als sp == sp2
    EXPECT_TRUE(sp == sp2);
}

TEST(modern_cpp, vectors) {
    auto c = std::vector<int> {};
    EXPECT_EQ(c.empty(), true);
    EXPECT_EQ(c.size(), 0);

    c = std::vector<int> {1,2,3,4,5};
    EXPECT_EQ(c.size(), 5);

    // mogelijk om de waardes als array aan te passen
    c[0] = 9;
    c[1] = 8;

    // mogelijk om waardes toe te voegen aan vector
    c.emplace_back(6);          // add to end
    c.emplace(begin(c), 0);     // add to begin

    for (auto&& e : c) {
        std::cout << std::to_string(e) << std::endl; // e is waarde
    }
    //zelfde als
    for (auto i = begin(c); i != end(c); ++i)
    {
        std::cout << "i: " << std::to_string(*i) << std::endl; // e is iterator
    }
    // deze is krachter en flexibeler
    for (auto i = rbegin(c); i != rend(c); ++i)     //reverse
    {
        std::cout << "i: " << std::to_string(*i) << std::endl; // e is iterator
    }
}

class Cell {
public:
  Cell():i_(0){
      std::cout << "default constructor" << std::endl;
  };

  Cell(int i):i_(i){
      std::cout << "constructor" << std::endl;
  };

  ~Cell() {
      std::cout << "destructor" << std::endl;
  }

  int GetI() {
      return i_;
  }

  auto operator==(const Cell& rhs) -> bool {
      return i_ == rhs.i_;
  }

  bool operator==(const Hen& rhs){
      return i_ == rhs.id_;
  }


private:
  int i_;
  bool alive_;
};

TEST(gol, vector_of_vectors_unique_ptr) {
    auto number_of_columns = 5;
    auto number_of_rows = 9;
    std::vector <std::vector< std::unique_ptr<int>>> matrix;
    //matrix.resize(number_of_columns, std::vector<std::unique_ptr<int>>(number_of_rows, std::make_unique<int>()));
    matrix.resize(number_of_columns);
    for (auto&& c : matrix){
        c.reserve(number_of_rows);
        for (auto i = 0; i < number_of_rows; ++i) {
            c.emplace_back(std::make_unique<int>());
        }
    }
}

TEST(gol, vector_of_vectors) {
    auto number_of_columns = 5;
    auto number_of_rows = 9;
    std::vector <std::vector<Cell>> matrix;
    matrix.resize(number_of_columns, std::vector<Cell>(number_of_rows, Cell()));
}

TEST(gol, vector_of_vectors2) {
    auto number_of_columns = 5;
    auto number_of_rows = 9;
    std::vector <std::vector< std::shared_ptr<Cell>>> matrix;
    matrix.resize(number_of_columns, std::vector<std::shared_ptr<Cell>>(number_of_rows, std::make_shared<Cell>()));
}

TEST(modern_cpp, list) {
  auto l = std::list<int> {1,2,3,4,5};
  EXPECT_EQ(l.size(), 5);
  EXPECT_FALSE(l.empty());

  auto m = std::list<int> (begin(l), end(l));
  EXPECT_EQ(m.size(), 5);
  EXPECT_FALSE(m.empty());

  auto n = std::list<int> (++begin(l), --end(l));
  EXPECT_EQ(n.size(), 3);
  EXPECT_FALSE(n.empty());

  auto o = std::list<int> (10, 1234); // size 10, all value 1234
  EXPECT_EQ(o.size(), 10);
  for (auto o_local : o) {
      EXPECT_EQ(1234, o_local);
  }

  for (auto i = begin(l); i != end(l); ++i) {
      std::cout << *i << std::endl;
  }

  l.emplace_back(6);
  l.emplace_front(0);
  EXPECT_EQ(l.size(), 7);

  l.pop_front();
  l.pop_back();
  EXPECT_EQ(l.size(), 5);

  l.reverse();
  for (auto i = begin(l); i != end(l); ++i) {
      std::cout << *i << std::endl;
  }
  l.sort();
  for (auto i = begin(l); i != end(l); ++i) {
      std::cout << *i << std::endl;
  }
  l.remove_if([](int value) {
      return true; // remove if return true
  });
  EXPECT_TRUE(l.empty());
}

TEST(modern_cpp, movement_within_containers) {
    auto b = std::list <Hen> ();
    auto a = Hen("Henrietta");
    b.push_back(a);
    // move constuctor with name Henrietta -> IF NO MOVE CONSTUCTOR IS THERE, IT WILL COPY
    // destuctor with name null

    auto c = std::list <Hen> ();
    c.emplace_back(); // this will call default constructor
    c.emplace_back("Henrietta"); // this will fill in arguments in explicit constructor
    c.clear();
}

TEST(modern_cpp, set) {
    // A set is ordered. It is guaranteed to remain
    // in a specific ordering, according to a functor
    // that you provide. No matter what elements you
    // add or remove (unless you add a duplicate, which
    // is not allowed in a set), it will always be ordered.
    auto c = std::set<int, std::less<int>> ();
    EXPECT_TRUE(c.empty());
    EXPECT_EQ(c.size(), 0);

    c = std::set<int> {1 ,3, 5, 2, 4};
    EXPECT_EQ(c.size(), 5);

    for (auto number : c) {
        std::cout << number << std::endl;
    }

    auto result = c.emplace(6); // result is a pair pair<iterator, bool>
    EXPECT_EQ(*result.first, 6);
    EXPECT_TRUE(result.second);

    result = c.emplace(6);
    EXPECT_EQ(*result.first, 6);
    EXPECT_FALSE(result.second);

    EXPECT_EQ(c.erase(6), 1);
    EXPECT_FALSE(c.erase(1234));

    auto exist = c.find(3);
    EXPECT_EQ(*exist, 3);

    auto does_not_exist = c.find(1234); // if not found it will give back iterator to the end of set
    EXPECT_EQ(does_not_exist, end(c)); //
}

TEST(modern_cpp, map) {
    auto c = std::map<int, double>();

    EXPECT_EQ(c.size(), 0);

    c = std::map<int, double> {
        {1, 1.1},
        {4, 4.1},
        {2, 2.1},
        {3, 3.1},
        {5, 1.1}
    };

    EXPECT_EQ(c.size(), 5);
    EXPECT_EQ(c[4], 4.1);

    c[6] = 6.1;

    auto v = c[7];
    EXPECT_EQ(v, 0.0);

    EXPECT_EQ(c.size(), 7);

    auto result = c.emplace(8, 8.1); // returns itter and bool
    EXPECT_TRUE(result.second);

    auto i = c.find(6); // find elements by value
    EXPECT_EQ(i->first, 6); // key
    EXPECT_EQ(i->second, 6.1); //value

    for (auto&& a : c) {
        std::cout << a.first << " " << a.second << std::endl;
    }
}

struct Han {
    std::string name;
    int id;
    int GetId() const {
        return id;
    }
    Han(): name(""), id(0) {}
    Han(std::string name_, int id_): name(name_), id(id_) {}
};

auto operator==(Han const& left, Han const& right) -> bool {
    return left.name == right.name &&
        left.id == right.id;
}

namespace std
{
    template <>
    struct hash<Han>
    {
        // hash function -> two calls with the same value must give the same results!
        auto operator()(Han const & han) const -> size_t
        {
            boost::hash<std::string> string_hash;
            auto hash = string_hash(han.name + std::to_string(han.id));
            return hash;
        }
    };
}

TEST(modern_cpp, unorderd_map) {
    auto c = std::unordered_map<Han, double>  // Han moet een Hash fuctie en een equal functie hebben
    {
            {{"jaap", 123}, 1.2},
            {{"geid", 124}, 3.2},
            {{"mier", 125}, 4.2}
    };

    c[{"piet", 345}] = 6.3;

    auto k = Han("gek", 2311);
    c[k] = 8.6;

    for (auto&& h: c){
        std::cout << h.first.name << " " << h.first.id << " " << h.second << std::endl;
    }
}


auto trim (std::string const & s) -> std::string {
    auto front = find_if_not(begin(s), end(s), isspace);    //standaard find algorithms voor containers werkt voor strings
    auto back = find_if_not(rbegin(s), rend(s), isspace);

    return std::string(front, back.base());
}

TEST(modern_cpp, string_basic) {
    auto s = std::string();
    // same as -> string is typedef voor basic_string<char> container
    s = std::basic_string<char>();

    s = std::string("cluck!");

    EXPECT_FALSE(s.empty());
    EXPECT_EQ(s.size(), 6);

    std::cout<< s.c_str() << std::endl;

    for (auto&& c : s){
        std::cout<< c << std::endl;
    }

    auto hen = std::string("Mathilda");
    auto pasture = std::string("Tomatoes");
    auto id = hen + "@" + pasture;

    std::cout<< id.c_str() << std::endl;

    auto pos = id.find('@');
    auto pos2 = id.find('m');
    std::cout<< "pos " << pos << " pos2 " << pos2 << std::endl;
    //auto domain = std::string(id, pos, pos2-pos); zelfde als
    auto domain = id.substr(pos, pos2-pos);
    std::cout<< "domain:" << domain << std::endl;

    auto trimmed = trim ("      \t Matilda the hen  \r\n   ");
    std::cout << trimmed << std::endl;
    EXPECT_EQ(trimmed, "Matilda the hen");
}

#include <regex>
TEST(modern_cpp, regular_expressions) {
    auto s = std::string("Call 8770-808-2321 to reach Amedlia the hen!");

    auto r = std::regex{ R"((\d{3})-(\d{3})-(\d{4}))"};

    auto m = std::smatch (); // match results of string iterators
    //m = std::match_results<std::string::const_iterator>();

    EXPECT_TRUE(m.empty());

    EXPECT_TRUE(std::regex_search(s, m, r));

    EXPECT_FALSE(m.empty());

    for (auto&& sub : m){
        std::cout << "[" << std::string(&*sub.first, sub.length()) << "]" << std::endl;
    }
}

TEST(algorithms, count) {
    // Use auto && for the ability to modify and discard values of the sequence within the loop. (That is, unless the container provides a read-only view, such as std::initializer_list, in which case it will be effectively an auto const &.)
    // Use auto & to modify the values of the sequence in a meaningful way.
    // Use auto const & for read-only access.
    // Use auto to work with (modifiable) copies.
    auto v = std::vector<int> {1,2,3,4,5,6,7,8,9};
    auto s = std::vector<Cell> ();
    for(auto&& i : v) {
        s.emplace_back(Cell(i));
    }

    // count number of elements contain the integer 2
    auto c = std::count(begin(v), end(v), 2);
    EXPECT_EQ(1, c);
    std::cout << "count: " << c << std::endl;

    auto target = Hen("mar");
    target.id_ = 2;
    auto d = std::count_if(begin(s), end(s), [&target] (auto&& local_s) {
        return local_s == target;
    });

    EXPECT_EQ(1, d);
    std::cout << "count: " << d << std::endl;
}

TEST(algorithms, count_none_any_all) {
    auto v = std::vector<int> {1,2,3,4,5,6,7,8,9};
    auto all = std::all_of(begin(v),end(v),[](auto&& v_local){
       return v_local == 2;
    });
    auto any = std::any_of(begin(v),end(v),[](auto&& v_local){
        return v_local == 2;
    });
    auto none = std::none_of(begin(v),end(v),[](auto&& v_local){
        return v_local == 2;
    });
    EXPECT_FALSE(all);
    EXPECT_TRUE(any);
    EXPECT_FALSE(none);
}

TEST(algorithms, find) {
    auto v = std::vector<int> {2,4,6,6,1,3,-2,0,11,2,3,2,4,4,2,4};

    //find first zero in collection
    auto result = std::find(begin(v), end(v), 0);
    EXPECT_EQ(0, *result); // result is an iterator, de-reference the result to get the value;

    // find the first 2 after that zero
    result = std::find (result, end(v), 2);
    if (result != end(v)) {         // find will give back iterator after end
        EXPECT_EQ(2, *result);
    }

    // find first odd value
    result = std::find_if(begin(v), end(v), [](auto&& v_local) {return v_local%2 != 0;});
    EXPECT_EQ(1, *result);
    result = std::find_if_not(begin(v), end(v), [](auto&& v_local) {return v_local%2 != 0;});
    EXPECT_EQ(2, *result);

    auto primes = std::vector<int> {1,2,3,5,7,11,13};
    result = std::find_first_of(begin(v), end(v), begin(primes), end(primes));
    EXPECT_EQ(2, *result);

    //Searches the range [first1,last1) for the first occurrence of the sequence
    // defined by [first2,last2), and returns an iterator to its first element,
    // or last1 if no occurrences are found
    auto subsequence = std::vector<int> {2 ,4};
    result = std::search(begin(v), end(v), begin(subsequence), end(subsequence));

    result = std::search_n(begin(v), end(v), 2, 4); // two times a 4
    result = std::adjacent_find(begin(v), end(v));
}

TEST(algorithms, simple_sort) {
    auto v = std::vector<int> {4,1,0,-2,3,7,-6,2,0,0,-9,9};
    auto v2 = v;
    std::sort(begin(v2), end(v2));
    std::sort(begin(v2), end(v2), [](int a, int b) {return a > b;});
    std::sort(begin(v2), end(v2), [](int a, int b) {return std::abs(a) > std::abs(b);});
}

class Employee {
public:
    Employee(std::string first, std::string last, int sal): first_(first), last_(last), sal_(sal) {};
    int getSalary() const {return sal_;};
    std::string getSortingName() const {return last_ + ", " + first_;};
    bool operator<(const Employee that) const {return sal_ < that.sal_;};
private:
    std::string first_;
    std::string last_;
    int sal_;
};

TEST(algorithms, simple_sort2) {
    auto staff = std::vector<Employee> {
            {"Kate", "Greg", 1000},
            {"Ob", "Art", 2000},
            {"Fake", "Name", 1000},
            {"Alan", "Turing", 2000},
            {"Frace", "Hopper", 2000},
            {"Anita", "Borg", 2000},
    };

    //std::sort(begin(staff), end(staff)); // deze maakt gebruikt van de operator < van Employee. Als deze er niet is, dan geeft het een compile error
    std::sort(begin(staff), end(staff), [](auto&& a, auto&& b) {return a<b;});

    // als je will dat de gesorteerd salaris ook de naam is gesorteerd -> do eerst een sort op naam, dan stable_sort op salaris
    std::sort(begin(staff), end(staff), [](auto&& a, auto&& b) {return a.getSortingName() <b.getSortingName();});
    std::stable_sort(begin(staff), end(staff), [](auto&& a, auto&& b) {return a<b;}); // andere variable workden in zelfde volgorde gelaten
}

TEST(algorithms, find_largest) {
    auto v = std::vector<int>{2, 4, 6, 6, 1, 3, -2, 0, 11, 2, 3, 2, 4, 4, 2, 4};
    auto max = *std::max_element(begin(v), end(v)); // * dereference to value int
    EXPECT_EQ(11, max);
    auto min = *std::min_element(begin(v), end(v));
    EXPECT_EQ(-2, min);

    auto positive = *std::upper_bound(begin(v), end(v), 0);
}

TEST(algorithms, comparing) {
    auto a = std::vector<int>{1,2,3,4,5};
    auto b = std::vector<int>{1,2,0,4,5};

    auto same = std::equal(begin(a), end(a), begin(b), end(b)); // for own classe you need to make an "operator==" for comparing classes
    EXPECT_FALSE(same);

    // mismatch returns a pair of iterators!
    auto first_change = std::mismatch(begin(a), end(a), begin(b));
    int first_change_in_a = *(first_change.first);
    int first_change_in_b = *(first_change.second);
    EXPECT_EQ(3, first_change_in_a);
    EXPECT_EQ(0, first_change_in_b);
}

TEST(algorithms, loop_in_disquise) {
    auto v = std::vector<int>{2, 4, 6, 6, 1, 3, -2, 0, 11, 2, 3, 2, 4, 4, 2, 4};
    // manier 1
    for (auto it = begin(v); it != end(v); it++) {
        *it = 0;
    }
    // mamier 2
    for (auto& i: v) { // de ref zorgt ervoor dat de waardes van vector v aangepast worden!
        i = 1;
    }
    // manier 3
    for_each(begin(v), end(v), [](auto& v_local) { v_local = 2;}); // de ref zorgt ervoor dat de waardes van vector v aangepast worden!

    // Use auto && for the ability to modify and discard values of the sequence within the loop. (That is, unless the container provides a read-only view, such as std::initializer_list, in which case it will be effectively an auto const &.)
    // Use auto & to modify the values of the sequence.
    // Use auto const & for read-only access.
    // Use auto to work with (modifiable) copies.
    for (auto&& i: v) { // de ref zorgt ervoor dat de waardes van vector v aangepast worden!
        i = 1;
    }
    // of
    for_each(begin(v), end(v), [](auto&& v_local) { v_local = 2;}); // de ref zorgt ervoor dat de waardes van vector v aangepast worden!

    for (auto&& i: v) { // de ref ref zorgt ervoor dat de waardes van vector v aangepast worden EN verwijderd kan worden !
        v.pop_back();
    }

    v = std::vector<int>{2, 4, 6, 6, 1, 3, -2, 0, 11, 2, 3, 2, 4, 4, 2, 4};
    for (auto& i: v) { // Use auto & to modify the values of the sequence. ??????
        v.pop_back();
    }
}

TEST(algorithms, copy) {
    auto v = std::vector<int> {3,6,1,0,-2,5};
    auto v2 =  std::vector<int>(v.size());
    EXPECT_EQ(v2.size(), v.size());

    std::copy(begin(v), end(v), begin(v2));
    EXPECT_TRUE(std::equal(begin(v), end(v), begin(v2), end(v2)));

    auto v3 = v;                                // dit is het zelfde als voorgaande statement
    auto v4 = std::vector<int>(v.size());
    auto position = find(begin(v), end(v), 1);  // position is een iterator
    std::copy(begin(v), position, begin(v4));   // position is end -> wordt niet meegerekend in loop
    EXPECT_EQ(v4[0], 3);
    EXPECT_EQ(v4[1], 6);
    EXPECT_EQ(v4[2], 0);

    auto v5 = std::vector<int> (v.size());
    std::copy_if(begin(v), end(v), begin(v5), [](auto&& e) {
        return e%2 == 0;
    });
    EXPECT_EQ(v5[0], 6);
    EXPECT_EQ(v5[1], 0);
    EXPECT_EQ(v5[2], -2);
    EXPECT_EQ(v5[3], 0);

    auto v6 = std::vector<int> (v.size());
    copy_n(begin(v5), 3, begin(v6));
    EXPECT_EQ(v6[0], 6);
    EXPECT_EQ(v6[1], 0);
    EXPECT_EQ(v6[2], -2);

    // als je naar de zelfde vector copieerd, en er is overlap, dan moet je copy_backwards doen
    // std::copy(begin(v), end(v)-1, end(v)+1); // dit geeft problemen
    std::copy_backward(begin(v), end(v)-1, end(v)+1);
}

TEST(algorithms, remove) {
    auto v = std::vector<int> {3,6,1,0,-2,5};
    auto newEnd = std::remove(begin(v), end(v), 3);
    EXPECT_EQ(6, v.size()); // still size is 6, SIZE OF VECTOR HAS NOT CHANGED!
    int logicalSize = newEnd - begin(v);// logical size is 5
    v.erase(newEnd, end(v)); // here we erase it
    EXPECT_EQ(5, v.size());

    // one liner removes and erases from vector c
    v.erase(std::remove(begin(v), end(v), 1), end(v));
}

class Resource {

public:
    Resource() {
        r_object_count ++;
        std::cout << "constuctor " << r_object_count << std::endl;
    }
    Resource(const Resource& r) { // copy constructor
        i = r.i;
        r_object_count++;
        std::cout << "copy constuctor " << r_object_count << std::endl;
    }
    Resource& operator=(const Resource& r) { // assign operator
        std::cout << "assign " << r_object_count << std::endl;
        i = r.i;
        return *this;

    }
    ~Resource(){
        r_object_count--;
        std::cout << "destuctor " << r_object_count << std::endl;
    }
    void setValue(int ii) {i = ii;}
    int getValue() {return i;}

    static int r_object_count;// SIMPLE REFERENCING COUNTING
private:
    int i = 1;
};

int Resource::r_object_count = 0;

TEST(algorithms, remove2) {
    auto v = std::vector<Resource> (2);
    v[0].setValue(8);
    v[1].setValue(9);
    EXPECT_EQ(v[0].getValue(), 8);
    EXPECT_EQ(v[1].getValue(), 9);

    auto newEndIterator = std::remove_if(begin(v), end(v), [](auto&& e) {  // gives back iterator of new end
        return e.getValue() == 8;
    });
    EXPECT_EQ(v.size(), 2); // SIZE OF VECTOR HAS NOT CHANGED!
    EXPECT_EQ(v[0].getValue(), 9);  // shifted, but not erased
    EXPECT_EQ(v[1].getValue(), 9);

    v.erase(newEndIterator, end(v));  // now really erased
    EXPECT_EQ(v.size(), 1);
}

#include <numeric> // needed for iota
TEST(algorithms, fill) {
    auto v = std::vector<int> (10);
    std::fill(begin(v), end(v), 1);  // used copy
    EXPECT_EQ(v[0], 1);

    std::fill_n(begin(v), 6, 2);
    EXPECT_EQ(v[5], 2);
    EXPECT_EQ(v[6], 1);

    std::iota(begin(v), end(v), 1);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);

    int i = 1;
    std::generate(begin(v), end(v), [&i]() {return (i *= 2);});
    EXPECT_EQ(v[0], 2);
    EXPECT_EQ(v[1], 4);
    EXPECT_EQ(v[2], 8);
    EXPECT_EQ(v[3], 16);

    //////////
    i = 1;
    auto c = std::vector<std::unique_ptr<Cell>>(10 /*, std::make_shared<Cell>(Cell(++i))*/);
    std::generate(begin(c), end(c), [&i](){ return std::make_unique<Cell>(Cell(++i));});
}

TEST(algorithms, replace) {
    auto v = std::vector<int> {2,3,4,5,3,1,4,65,3,4,-3,-2};
    std::replace(begin(v), end(v), 3, 100);  // used copy
    EXPECT_EQ(v[1], 100);
    EXPECT_EQ(v[4], 100);
    EXPECT_EQ(v[8], 100);

    std::replace_if(begin(v), end(v), [](auto&& e) {
        return e < 0;
    }, 0);
    EXPECT_EQ(v[10], 0);
    EXPECT_EQ(v[11], 0);
}

TEST(algorithms, transform) {
    auto v = std::vector<int> {2,3,4,5,3,1,4,65,3,4,-3,-2};
    std::transform(begin(v), end(v), begin(v), [](auto&& e) {return e*2;});  // used copy
    EXPECT_EQ(v[0], 4);
    EXPECT_EQ(v[1], 6);
    EXPECT_EQ(v[2], 8);

    auto v1 = std::vector<int> {2,3,4,5,3,1,4,65,3,4,-3,-2};
    auto v2 = std::vector<int> {2,3,4,5,3,1,4,65,3,4,-3,-2};
    auto v3 = std::vector<int> (v.size());
    std::transform(begin(v1), end(v1), begin(v2), begin(v3), [](auto&& e1, auto&& e2) {return e1+e2;});  // used copy
    EXPECT_EQ(v3[0], 4);
    EXPECT_EQ(v3[1], 6);
    EXPECT_EQ(v3[2], 8);

    // transforms vector of Hans to vector of Cells
    auto hans = std::vector<Han> (10);
    int j = 1;
    std::generate(begin(hans), end(hans), [&j](){
        return Han("", j++);
    });
    auto cells = std::vector<Cell> (10);
    std::transform(begin(hans), end(hans), begin(cells), [](auto&& h) {return Cell(h.GetId());});
}

TEST(algorithms, back_inserter) {
    // with back_inserter iterator you dont have to pre-allocate memory.
    // memory will be added when needed
    auto v1 = std::vector<int>();
    std::fill_n(back_inserter(v1), 10, 2);
    for(auto&& e : v1){
        EXPECT_EQ(e, 2);
    }
    EXPECT_EQ(v1.size(), 10);

    // [n=0] is alias capturing -> zelde als 'n' buiten declarene met [&n].
    // mutable zegt dat mijn member variabelen van mijn lambda aan te passen zijn door de code van mijn lambda.
    std::generate_n(back_inserter(v1), 10, [n=0]()mutable{return n++;});
    EXPECT_EQ(v1.size(), 20);
    EXPECT_EQ(v1[10], 0);
    EXPECT_EQ(v1[11], 1);
    EXPECT_EQ(v1[12], 2);

    auto cells = std::vector<std::unique_ptr<Cell>>();
    std::generate_n(back_inserter(cells), 10, [n=0]()mutable{
        return std::make_unique<Cell>(Cell(n++));
    });

    // back_inserter werkt met de volgende argorithms
    // fill_n, generate_n, transform, copy, copy_if, unique_copy, reverse_copy
    auto v2 = std::vector<int>();
    std::transform(begin(v1), end(v1), back_inserter(v2), [](auto&& e) {return e*2;});
    EXPECT_EQ(v2.size(), 20);
    EXPECT_EQ(v2[0], 4);
    EXPECT_EQ(v2[1], 4);
    EXPECT_EQ(v2[2], 4);
    // ...
    EXPECT_EQ(v2[10], 0);
    EXPECT_EQ(v2[11], 2);
    EXPECT_EQ(v2[12], 4);

    auto v3 = std::vector<int> {3,6,1,0,-2,5,-2};
    auto v4 = std::vector<int> ();
    std::copy_if(begin(v3), end(v3), back_inserter(v4), [](auto&& e) {
        return e != 3;
    });
    EXPECT_EQ(v4.size(), v3.size()-1);
    EXPECT_EQ(v4[0], 6);
    EXPECT_EQ(v4[1], 1);

    auto v5 = std::vector<int>();
    std::unique_copy(begin(v3), end(v3), back_inserter(v5));
    EXPECT_EQ(v5.size(), v3.size()-1);
}

TEST(algorithms, reverse_iterators) {
    auto v1 = std::vector<int>(10);
    std::iota(begin(v1), end(v1), 1);
    EXPECT_EQ(v1[0], 1);
    EXPECT_EQ(v1[1], 2);
    EXPECT_EQ(v1[2], 3);

    // reverse copy
    auto v2 = std::vector<int>();
    copy(rbegin(v1), rend(v1), back_inserter(v2));
    EXPECT_EQ(v2[0], 10);
    EXPECT_EQ(v2[1], 9);
    EXPECT_EQ(v2[2], 8);
}

TEST(algorithms, swap) {
    auto even = std::vector<int> {2,4,6,8,10};
    auto odd = std::vector<int> {1,3,5,7,9};

    auto v1 = even;
    std::swap(v1[0], v1[1]);
    EXPECT_EQ(v1[0],4);
    EXPECT_EQ(v1[1],2);

    auto v2 = odd;
    std::swap(v1[0], v2[0]);
    EXPECT_EQ(v1[0],1);
    EXPECT_EQ(v2[0],4);

    v1 = even;
    v2 = odd;
    std::iter_swap(begin(v1), find(begin(v2), end(v2), 5));
    EXPECT_EQ(v1[0],5);
    EXPECT_EQ(v2[2],2);

    v1 = even;
    v2 = odd;
    std::swap_ranges(begin(v1), find(begin(v1),end(v1),6), begin(v2));
    EXPECT_EQ(v1[0],1);
    EXPECT_EQ(v1[1],3);
    EXPECT_EQ(v2[0],2);
    EXPECT_EQ(v2[1],4);
}

TEST(algorithms, partial_sort_rotate_stable_partition) {
    // handige algoritmes

    // partial_sort_copy     => top_n
    auto v = std::vector<int> {7,2,5,4,3,6,1};
    auto v2 = std::vector<int> (v.size());
    std::partial_sort_copy(begin(v), end(v), begin(v2), end(v2));

    // rotate                => move_range_within_collection
    auto v3 = std::vector<int> (6);
    std::iota(begin(v3), end(v3), 1);
    EXPECT_EQ(v3[0],1);
    EXPECT_EQ(v3[1],2); // <-
    EXPECT_EQ(v2[2],3); //
    EXPECT_EQ(v3[3],4); // ->
    EXPECT_EQ(v2[4],5);

    auto start_iter = find(begin(v3), end(v3), 2);
    auto end_iter = find(begin(v3), end(v3), 4);
    std::rotate(start_iter, end_iter, end_iter+1); // end_iter+1, so 4 will be includen
    EXPECT_EQ(v3[0],1);
    EXPECT_EQ(v3[1],4); // <-
    EXPECT_EQ(v3[2],2); // rotate
    EXPECT_EQ(v3[3],3); // ->
    EXPECT_EQ(v3[4],5);

    // stable_partition      => gather
    // 1 ->     1 <-    2
    // 2        3       1 <-
    // 3 ->     5       3
    // 4        7 ->    5
    // 5 ->     2       7 ->
    // 6        4       4
    // 7 ->     6       6
    // 8        8       8
    auto v4 = std::vector<int> (8);
    std::iota(begin(v4), end(v4), 1);
    auto selected = std::stable_partition(begin(v4), end(v4), [](auto&& e) {return e%2!=0;}); // gives back an iterator between the border of selected and unselected
    EXPECT_EQ(v4[0],1); // -> partitioned to the top
    EXPECT_EQ(v4[1],3);
    EXPECT_EQ(v4[2],5);
    EXPECT_EQ(v4[3],7); // -> partitioned to the top
    EXPECT_EQ(v4[4],2);
    EXPECT_EQ(v4[5],4); // stable partition will keep this in order
    EXPECT_EQ(v4[6],6);
    EXPECT_EQ(v4[7],8);
    auto four = std::find(begin(v4), end(v4), 4);
    std::rotate(begin(v4), selected, four);     // not include 4! rotate(first, newfirst, last)
    EXPECT_EQ(v4[0],2);
    EXPECT_EQ(v4[1],1); // <-
    EXPECT_EQ(v4[2],3);
    EXPECT_EQ(v4[3],5);
    EXPECT_EQ(v4[4],7); // ->
    EXPECT_EQ(v4[5],4);
    EXPECT_EQ(v4[6],6);
    EXPECT_EQ(v4[7],8);
}

// Iterator Parameter
// normal two iterators to define ranges to e.g find, generate etc
// -> end(v) is always 1 past the last of vector
// iterator and a number
// three iterator
// -> start, end, starting of destination
// -> uitzondering is rotate deze is rotate(first, newfirst, last)
// -> uitzondering is partial_sort => partial_sort(first, midle, last)
// four iterator
// -> start and end, define range of start sequence you are looking for
// -> uitzondering is rotate_copy => rotate_copy(first, newfirst, last, destfirst)
//
// Integer Parameter
// if function has _n than second parameter is size (number of elements)
//
// Perdicate Last = lambda
// meestal overloading. sort(first, last)  of  sort(first, last, predicate)
// -> uitzondering replace_if en replace_copy_if. have predicate in 3rd parameter, and preplace value in 4rth parameter

// Ends with _if
// -> add a predicate or, if original takes value, replace with predicate
// Ends with _n
// -> Replace fist, last  with  first, size
// Starts with is_
// -> returns bool
// -> uitzondering is_X_until; returns iterator
// Starts with stable_
// -> elements keep their relative order if the function has no reason to swap them

#include <functional>
TEST(language_features, function) {
    std::function<bool(int)> test;
    test = [](int a) {return a>5;};
    auto b = test(4);
    std::cout << "b:" << b <<std::endl;
}

template<typename X, typename Y>
auto multiply(X x, Y y) -> decltype(x * y)
{
    return x * y;
}

TEST(language_features, lamda) {
    // lamda:
    // [capture_block}(parameter_list) mutable exception_spec -> return_type { body}

   auto a = multiply(3,2);
   EXPECT_EQ(a, 6);
}

#include <boost/optional.hpp>
class Address {
public:
    Address (std::string street_name): street_name_(street_name){};
    std::string street_name_;
};

class Person {
public:
    Person(std::string first_name, std::string last_name): first_name_(first_name), last_name_(last_name) {};
    boost::optional<std::string> GetMiddleName() const {return middle_name_;};
    std::string first_name_;
    std::string last_name_;
    boost::optional<std::string> middle_name_;
    boost::optional<Address> address_;
};

TEST(boost_lib, optional) {
    auto p = Person("John", "Doe");
    // check if optional middle_name is assigned
    // optional implicitly coversion to bool
    EXPECT_FALSE(p.middle_name_);

    p.middle_name_ = "watson";
    EXPECT_TRUE(p.middle_name_);

    // use in code
    if (p.middle_name_) {
        std::cout << *(p.middle_name_) << std::endl; // need to dereference the optional
        std::cout << p.middle_name_.get() << std::endl; // or use get()
    }

    // return
    auto middle = p.GetMiddleName();
    if (middle) {
        std::cout << *middle << std::endl; // need to dereference the optional
    }

    EXPECT_FALSE(p.address_);
    p.address_ = Address ("Baker Streeth");
    EXPECT_TRUE(p.address_);
    if (p.address_) {
        std::cout << p.address_->street_name_ << std::endl;
        std::cout << (*p.address_).street_name_ << std::endl;
        std::cout << p.address_.get().street_name_ << std::endl;
    }
}

#include <boost/any.hpp>
// Interface: Boost::Any
// Boost::any<T> -> T can be anything, T must be copy-constuctable
// Creation is easy -> any w; //has no value, any x(2.0), vector<any> (42, "life") // vector of any value;
// Queries: empty() -> checks if we have a value, type() -> returs typeif of containing instance
// getting value: use global_function any_cast -> eigher pass pointer or reference
TEST(boost_lib, any) {
    boost::any w;
    EXPECT_TRUE(w.empty());

    boost::any x(2.0);
    EXPECT_FALSE(x.empty());
    std::cout << "type name: " << x.type().name() << std::endl;
    // EXPECT_EQ(x.type().name(), "double");

    std::vector<boost::any> y{42, "life"};
    int a = boost::any_cast<int>(y[0]);
    EXPECT_EQ(42, a);

    // First method: PAssing a reference to any will return
    // -> a reference to the conctained object OR will throw execption
    // here we wrap the any_cast in a try/catch
    // this wil thow a expection: "boost::bad_any_cast: failed conversion using boost::any_cast"
    try {
        int b = boost::any_cast<int>(y[1]);
    }
    catch (const boost::bad_any_cast& e) {
        std::cout << "wrong type: " << e.what() <<std::endl;
    }

    // Second method: Passing a pointer to any will return
    //-> A pointer to the contained object if the type match OR return null ptr otherwise
    int* c = boost::any_cast<int>(&y[0]);
    EXPECT_NE(nullptr, c);
    EXPECT_EQ(42, *c);

    int* d = boost::any_cast<int>(&y[1]);   // this will give an null pointer
    EXPECT_EQ(nullptr, d);

    const char** e = boost::any_cast<const char*>(&y[1]);
    EXPECT_EQ("life", *e);
}

// Observer pattern aka Events
// -> Component A wants to be notified when component B does something
// -> typical example: knowing when value has changed and updating the UI
// Publish & subscribe mechanism
// -> A clas can publish a particular event, e.g. NameChanged
// -> Other classes can choose to receive notification of when a name is changed.
// -> When the name is actually changed, all subscribers get notified (multicast)
// boost::signals2 support this mechanism: signals and slots

// boost:: signal<T>
// -> a signal that can be sent to anyone willing to listen
// -> T is the type of the slot function
// A slot is the function that receives the signal
// -> ordinary function
// -> Fuctor, std::function
// -> Lamda
// Connection
// -> signal<void()> s; // creates the signal
// -> auto c = s.connect([](){ cout<<"test"<<endl;}); // connects the signal to the slot
// more that one slot can be connected to a signal
// disonnection
// -> c.disconnect();
// -> disconnects all slots
// Slots can be blocked
// -> Temporarily disabled but not permanently disconnected
// -> Used to prevent infinite recursion
// -> shared_connection_block(c);
// -> Unblocked when block is destroyed (e.g. out of scope) or explicity via block.unblock();

#include <boost/signals2.hpp>

template<typename T> class INotifyPropertyChanged {
public:
    boost::signals2::signal<void(const T*, std::string, int)> PropertyChanged;
};

class Player: public INotifyPropertyChanged<Player>{
public:
    Player(std::string name): name_(name), number_of_goals(0){}

    std::string name_;
    int age_;
    int number_of_goals;

    boost::signals2::signal<void()> Scores;                                 //signal without arguments
    typedef boost::signals2::signal<void(std::string)> signalType;
    signalType Scores_with_name;                                            //signal with arguments
    boost::signals2::signal<void(std::string, int)> Scores_with_name_and_goals;

    void Scores_function() {
        number_of_goals ++;
        Scores_with_name_and_goals(name_, number_of_goals);
    }

    int GetAge() {return age_;}
    void SetAge(int age) {
        if (age == age_) return;

        age_ = age;
        PropertyChanged(this, "Age", age_);
    }
};

TEST(boost_lib, Signals2) {
    int scored = 0;
    Player p("Jan");

    // connect signal with a slot
    auto a = p.Scores.connect([&scored](){
        std::cout << "well done" << std::endl;
        scored ++;
    });

    p.Scores();
    EXPECT_EQ(1 ,scored);

    // connect signal with a slot
    auto b = p.Scores_with_name.connect([&scored](std::string name){
        std::cout << "well done: " << name << std::endl;
        EXPECT_EQ("Jan" ,name);
        scored ++;
    });

    p.Scores_with_name(p.name_);
    EXPECT_EQ(2 ,scored);

    //disconnect signal from all slots
    p.Scores.disconnect_all_slots();
    p.Scores_with_name.disconnect_all_slots();
    p.Scores_with_name(p.name_);

    p.Scores();
    EXPECT_EQ(2 ,scored); // not increased

    ////////////////////////////////

    auto c = p.Scores_with_name_and_goals.connect([&scored](std::string name, int count) {
        std::cout << "player " << name << " has scored " << count << std::endl;
        scored ++;
    });
    p.Scores_function();
    EXPECT_EQ(3 ,scored);
    p.Scores_function();
    EXPECT_EQ(4 ,scored);

    {
        boost::signals2::shared_connection_block d(c);
        p.Scores_function();
        EXPECT_EQ(4 ,scored); // connection blocked when block is in scope
    }

    p.Scores_function();
    EXPECT_EQ(5 ,scored);
}

// slots can have priority
TEST(boost_lib, Signals2_custumized_Prio) {
    boost::signals2::signal<void()> s;

    // prio
    s.connect(1,[](){
        std::cout << "first" << std::endl;
    });
    s.connect(0,[](){
        std::cout << "second" << std::endl;
    });

    s(); // "second" called, then "first"
}

void third() {
    std::cout << "third" << std::endl;
}

// Scoped connection
// -> connect signal and slot unti it goes out of scope
// -> able to disconnect a specific slot
TEST(boost_lib, Signals2_custumized_Scope) {
    boost::signals2::signal<void()> s;

    s.connect(third); // connect function

    {
        auto c = s.connect(1,[](){              // connection is scoped
            std::cout << "first" << std::endl;
        });
        boost::signals2::scoped_connection sc(c);

        s.connect(0,[](){                       // connection is not scoped
            std::cout << "second" << std::endl;
        });
        s();    // first, second and third will be called
    }
    // sc out of scope
    s.disconnect(third); // disconnect third from connection

    std::cout << "=============" << std::endl;


    s(); // c is out of scope, only "second" is called;
}

class Coach
{
public:
    void PlayerScored(){
        std::cout << "well done, " << std::endl;
    }
    void PlayerScoredWithName(std::string name){
        std::cout << "well done, " << name << std::endl;
    }
};
TEST(boost_lib, Signals2_customized_methods) {
    Player p("John");
    Coach coach;

    // boost::function<void(std::string)>
    auto b = boost::bind(&Coach::PlayerScored, &coach); // binding to a member function
    p.Scores.connect(b);
    p.Scores_with_name.connect(boost::bind(&Coach::PlayerScoredWithName, coach, _1));

    p.Scores();
    p.Scores_with_name("Mike");
}

#include <boost/smart_ptr.hpp>
// liftime tracking
// -> Keep the connection alive only while the source is alive
// -> Explicity create slot_type and use track
TEST(boost_lib, Signals2_customized_manage) {
    Player p("John");
    {
        auto coach = boost::make_shared<Coach>();       // need to use boost!
        p.Scores_with_name.connect(
                Player::signalType::slot_type
                (&Coach::PlayerScoredWithName, coach.get(), _1).track(coach)
                );
        p.Scores_with_name("John"); // signal handled
    }
    p.Scores_with_name("Mike"); // signal not handled
}

// slot returns value
// -> a slot function may return a value
// -> the result of direing a signal is a pointer to the LAST value
TEST(boost_lib, Signals2_advanced_return) {
    boost::signals2::signal<float(float,float)> s;
    s.connect(0,[](float a, float b){   // just one slot
        return a*b;
    });
    EXPECT_EQ(20, *s(4,5)); // signal needs to be dereferenced to get the values

    s.connect(1,[](float a, float b){   // multiple slots
        return a+b;
    });
    EXPECT_EQ(9, *s(4,5)); // the last connect returns to signal
}

// Accessing connection in the slot
TEST(boost_lib, Signals2_advanced_passing_connection) {
    // if we want to disconnect after number of calls
    boost::signals2::signal<void(int)> s;
    int v = 0;
    s.connect_extended([&v](const boost::signals2::connection& conn, int value){
        static int count = 0;
        if (count == 3){
            conn.disconnect();
        }
        else {
            v = v + value;
        }
        count ++;
    });
    s(1);
    s(1);
    s(1);
    s(1);
    s(1);
    EXPECT_EQ(v, 3);
}

// notification on property changes -> a.k.a INotifyPropertyChanged
TEST(boost_lib, Signals2_advanced_property_change) {
    Player p("John");
    p.PropertyChanged.connect([](const Player* p, std::string property, int value){
        std::cout << p->name_ << " " << property << " has changed with value " << value << std::endl;
    });
    p.SetAge(30);
}

// string operations
TEST(boost_lib, string_concat) {
    std::string s2 = "hallo";
    std::string s3 = "wereld";
    auto s4 = s2 + " " + s3;
    EXPECT_EQ(s4, "hallo wereld");

}

#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>
TEST(boost_lib, string_token_demo) {
    std::string s = "To be, or not to be?";

    boost::tokenizer<> t1(s);
    for (auto &part: t1) {
        std::cout << "<" << part << ">" << std::endl;
    }

    boost::char_separator<char> sep("o", " ", boost::keep_empty_tokens);
    boost::tokenizer<boost::char_separator<char>> t2(s, sep);
    for (auto &part: t2) {
        std::cout << "<" << part << ">" << std::endl;
    }
}

#include <boost/lexical_cast.hpp>
TEST(boost_lib, string_texical_cast) {
    std::string s = "2.1";
    double d = boost::lexical_cast<double>(s);
    EXPECT_EQ(2.1, d);

    std::string s2 = "123456";
    int n = boost::lexical_cast<int>(s2);
    EXPECT_EQ(123456, n);

    try {
        boost::lexical_cast<int>("abcdefg");
    }
    catch (const boost::bad_lexical_cast& e) {
        std::cout << e.what() << std::endl;
    }
}

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>
TEST(boost_lib, string_algorthm_string) {
    std::string t = "hello world\r\n";
    EXPECT_NE("hello world", t);

    boost::algorithm::trim(t);
    EXPECT_EQ("hello world", t);

    std::string t2 = "hello    world\r\n"; // double space becomes a single space
    EXPECT_NE("hello world", t2);
    boost::algorithm::trim_all(t2);
    EXPECT_EQ("hello world", t2);

    boost::algorithm::to_upper(t2);
    EXPECT_EQ("HELLO WORLD", t2);

    auto t3 = boost::algorithm::to_upper_copy(t);
    EXPECT_EQ("HELLO WORLD", t3);
}

#include <boost/bimap.hpp>
enum class Color {
    Red,
    Green,
    Blue
};
typedef boost::bimap<Color, std::string> ColorMapTypes;

TEST(boost_lib, string_algorthm_bimap) {
    auto r = Color::Red;
    // std::cout << c << std::endl; not able to print

    ColorMapTypes colorType;
    colorType.insert(ColorMapTypes::value_type(Color::Red, "Red"));

    Color c = colorType.right.at("Red");
    EXPECT_EQ(Color::Red, c);
    std::string s = colorType.left.at(Color::Red);
    EXPECT_EQ("Red", s);

    std::cout<< colorType.left.at(Color::Red) << std::endl;
}

#include <boost/units/unit.hpp>
#include <boost/units/systems/si.hpp>
#include <boost/units/systems/si/prefixes.hpp>
TEST(boost_lib, string_algorthm_units) {
    typedef boost::units::make_scaled_unit<boost::units::si::length, boost::units::scale<10, boost::units::static_rational<-2>>>::type cm;
    boost::units::quantity<cm> d(2.0 * boost::units::si::meters);
    boost::units::quantity<boost::units::si::time> t(100.0 * boost::units::si::seconds);
    boost::units::quantity<boost::units::si::velocity> x(d/t);
    // boost::units::quantity<boost::units::si::velocity> x(d/t/t); this will give compile error
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


TEST(simple, simple1) {
    std::vector<std::vector<int>> a;
    a.emplace_back(std::vector<int>{1,2,3,4,5});
    a.emplace_back(std::vector<int>{6,5,4,3,2});
    EXPECT_EQ(a[0][0], 1);

}

