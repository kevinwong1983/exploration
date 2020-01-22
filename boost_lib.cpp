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
        std::cout << *(p.middle_name_) << std::endl;    // need to dereference the optional
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
// Queries: empty() -> checks if we have a value, type() -> returns typeif of containing instance
// getting value: use global_function any_cast -> either pass pointer or reference
TEST(boost_lib, any) {
    boost::any w;
    EXPECT_TRUE(w.empty());

    boost::any x(2.0);
    EXPECT_FALSE(x.empty());
    EXPECT_EQ(2.0, boost::any_cast<double>(x));
    std::cout << "type name: " << x.type().name() << std::endl;
    // EXPECT_EQ(x.type().name(), "double");

    std::vector<boost::any> y{42, "life"};
    int a = boost::any_cast<int>(y[0]);
    EXPECT_EQ(42, a);

    // First method: Passing a reference to any will return
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
// -> A class can publish a particular event, e.g. NameChanged
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

    // connect signal with multiple slots
    auto a = p.Scores.connect([&scored](){
        std::cout << "well done 1" << std::endl;
        scored ++;
    });
    p.Scores.connect([&scored](){
        std::cout << "well done 2" << std::endl;
        scored ++;
    });

    p.Scores();
    EXPECT_EQ(2 ,scored);

    // connect signal with a slot
    auto b = p.Scores_with_name.connect([&scored](std::string name){
        std::cout << "well done: " << name << std::endl;
        EXPECT_EQ("Jan" ,name);
        scored ++;
    });

    p.Scores_with_name(p.name_);
    EXPECT_EQ(3 ,scored);

    //disconnect signal from all slots
    p.Scores.disconnect_all_slots();
    p.Scores_with_name.disconnect_all_slots();

    p.Scores_with_name(p.name_);
    p.Scores();
    EXPECT_EQ(3 ,scored); // not increased

    ////////////////////////////////

    auto c = p.Scores_with_name_and_goals.connect([&scored](std::string name, int count) {
        std::cout << "player " << name << " has scored " << count << std::endl;
        scored ++;
    });
    p.Scores_function();
    EXPECT_EQ(4 ,scored);
    p.Scores_function();
    EXPECT_EQ(5 ,scored);

    {
        boost::signals2::shared_connection_block d(c);
        p.Scores_function();
        EXPECT_EQ(5 ,scored); // connection blocked when block is in scope
    }

    p.Scores_function();
    EXPECT_EQ(6 ,scored);
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
        auto c = s.connect(1,[](){                // c = connection
            std::cout << "first" << std::endl;
        });
        boost::signals2::scoped_connection sc(c); //scopes your connection

        s.connect(0,[](){                         // connection is not scoped
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

class subscriber {
public:
    subscriber(std::string name): name_(name){}
    void timerHandler(){
        if (name_){
            std::cout<< "hello, my name is: " << *name_ << std::endl;
        } else {
            std::cout<< "subscriber has no name." << std::endl;
        }
    }
private:
    boost::optional<std::string> name_;
};

class timer {
public:
    timer(boost::asio::io_context& ioc, boost::asio::chrono::seconds seconds): t_(std::make_shared<boost::asio::steady_timer> (ioc, seconds)), seconds_(seconds) {
        t_->async_wait(boost::bind(&timer::trigger, this, boost::asio::placeholders::error, t_));
    }

    ~timer() {
        publisher_.disconnect_all_slots();
        t_->cancel();
    }

    void subscribe(std::function<void(void)> subscribe_cb) {
        publisher_.connect(subscribe_cb);
    }

private:
    void trigger(const boost::system::error_code& e, std::shared_ptr<boost::asio::steady_timer> t) {
        std::cout << "hello" << std::endl;
        publisher_();
        t_->expires_at(t->expiry() + boost::asio::chrono::seconds(seconds_));
        t_->async_wait(boost::bind(&timer::trigger, this, boost::asio::placeholders::error, t_));
    }

    boost::signals2::signal<void()> publisher_;
    std::shared_ptr<boost::asio::steady_timer> t_;
    boost::asio::chrono::seconds seconds_;
};

TEST(boost_lib, Signals2_customized_methods2) {
    boost::asio::io_context ioc;

    subscriber s1("jan");
    subscriber s2("piet");
    subscriber s3("klaas");
    timer t(ioc, boost::asio::chrono::seconds(2));
    t.subscribe(std::bind(&subscriber::timerHandler, &s1));
    t.subscribe(std::bind(&subscriber::timerHandler, &s2));
    t.subscribe(std::bind(&subscriber::timerHandler, &s3));

    ioc.run();
}

#include <boost/smart_ptr.hpp>
// lifetime tracking
// -> Keep the connection alive only while the source is alive
// -> Explicity create slot_type and use track
TEST(boost_lib, Signals2_customized_manage) {
    Player p("John");
    {
        auto coach = boost::make_shared<Coach>();       // need to use boost!
        p.Scores_with_name.connect(
                Player::signalType::slot_type
                        (&Coach::PlayerScoredWithName, coach.get(), _1).track(coach)
                        // can also use track_foreign(coach) for non boost pointers e.g. std::shared_ptr
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