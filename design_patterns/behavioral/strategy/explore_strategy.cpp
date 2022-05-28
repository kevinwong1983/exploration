#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>
#include <vector>
#include <boost/any.hpp>
#include <mutex>
#include <boost/signals2.hpp>
#include <map>
#include <iostream>
#include <sstream>
using namespace std;

// Strategy
// Enables the exact behavior of a system to be selected at either run-time (dynamic) or
// compile-time (static). Also known as a policy.

// Motivation
// Many algorithms can be decomposed into higher and lower level parts
// e.g. Make tea can be decomposed into
// - The process of making a hot beverage (boil water, pour into cup)
// - Tea specific things (get a tea bag)
// The high-level algorithm can then be reused for making coffee or hot chocolate
// - Supported by beverage-specific strategies

namespace strategy {

class FlyBehavior {
public:
    virtual ~FlyBehavior() = default;
    virtual void fly() = 0;
};

class FlyWithWings : public FlyBehavior {
public:
    void fly() override {
        std::cout << "Flap flap!" << std::endl;
    }
};

class FlyNoWay : public FlyBehavior {
public:
    void fly() override {
        std::cout << "I cant fly!" << std::endl;
    }
};

class QuackBehavior {
public:
    virtual ~QuackBehavior() = default;
    virtual void quack() = 0;
};

class Quack : public QuackBehavior {
public:
    void quack() override {
        std::cout << "Quuuuaaaack!" << std::endl;
    }
};

class Mute : public QuackBehavior {
public:
    void quack() override {
        std::cout << "...........!" << std::endl;
    }
};

class Duck {
public:
    Duck(std::unique_ptr<FlyBehavior> fly_behavior, std::unique_ptr<QuackBehavior> quack_behavior)
        : fly_behavior_(move(fly_behavior)), quack_behavior_(move(quack_behavior)){}

    virtual ~Duck() = default;
    void doFly() {
        fly_behavior_->fly();
    };
    void doQuack() {
        quack_behavior_->quack();
    };
    virtual void Display() = 0;

protected:
    std::unique_ptr<FlyBehavior> fly_behavior_;
    std::unique_ptr<QuackBehavior> quack_behavior_;
};

class MallardDuck : public Duck {
public:
    MallardDuck()
            : Duck(std::make_unique<FlyWithWings>(), std::make_unique<Quack>()){}
    MallardDuck(std::unique_ptr<FlyBehavior> fly_behavior, std::unique_ptr<QuackBehavior> quack_behavior)
            : Duck(move(fly_behavior), move(quack_behavior)){}
    void Display() override {
        std::cout << "Display a Mallard Duck" << std::endl;
    };
};

class DecoyDuck : public Duck {
public:
    DecoyDuck()
            : Duck(std::make_unique<FlyNoWay>(), std::make_unique<Mute>()){}
    void Display() override {
        std::cout << "Display a Decoy Duck" << std::endl;
    };
};

TEST(strategy, duck) {
    MallardDuck mallerd;
    DecoyDuck decoy;

    std::vector<Duck*> ducks;
    ducks.push_back(&mallerd);
    ducks.push_back(&decoy);

    for(auto& duck: ducks){
        duck->doFly();
        duck->doQuack();
        duck->Display();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////

enum class OutputFormat{
    MarkDown,
    Html
};

struct ListStrategy {
    virtual ~ListStrategy() = default;
    virtual void start(std::ostringstream& oss) = 0;
    virtual void end(std::ostringstream& oss) = 0;
    virtual void add_list_time(std::ostringstream& oss, const string& item) = 0;
};

struct MarkdownListStrategy: ListStrategy {
    void start(std::ostringstream& oss) override {};
    void end(std::ostringstream& oss) override {};
    void add_list_time(std::ostringstream& oss, const string& item) override {
        oss << " * " << item << std::endl;
    }
};

struct HtmlListStrategy: ListStrategy {
    void start(std::ostringstream& oss) override {
        oss << "<ul>" << std::endl;
    };
    void end(std::ostringstream& oss) override {
        oss << "</ul>" << std::endl;
    };
    void add_list_time(std::ostringstream& oss, const string& item) override {
        oss << "<li>" << item << "</li>" << std::endl;
    }
};

struct TextProcessor {
    void clear (){
        oss.str("");
        oss.clear();
    }

    std::string str() const {
        return oss.str();
    }

    void append_list(const std::vector<string> items){
        list_stategy->start(oss);
        for (auto& item: items) {
            list_stategy->add_list_time(oss, item);
        }
        list_stategy->end(oss);
    }

    void set_output_format(OutputFormat format){
        switch (format) {
            case OutputFormat::MarkDown:
                list_stategy = std::make_unique<MarkdownListStrategy>();
                break;
            case OutputFormat::Html:
                list_stategy = std::make_unique<HtmlListStrategy>();
                break;
            default:
                break;
        }
    }
private:
    std::ostringstream oss;
    std::unique_ptr<ListStrategy> list_stategy;
};

TEST(strategy, dynamic) {
    TextProcessor tp;
    tp.set_output_format(OutputFormat::MarkDown);
    tp.append_list({"foo", "bar", "baz"});
    std::cout << tp.str() << std::endl;

    tp.clear();
    tp.set_output_format(OutputFormat::Html);
    tp.append_list({"foo", "bar", "baz"});
    std::cout << tp.str() << std::endl;
}

template <typename LS>
struct TextProcessor2 {
    TextProcessor2(): list_stategy(std::make_unique<LS>()){
    }

    void clear (){
        oss.str("");
        oss.clear();
    }

    std::string str() const {
        return oss.str();
    }

    void append_list(const std::vector<string> items){
        list_stategy->start(oss);
        for (auto& item: items) {
            list_stategy->add_list_time(oss, item);
        }
        list_stategy->end(oss);
    }

private:
    std::ostringstream oss;
    std::unique_ptr<ListStrategy> list_stategy;
};

TEST(strategy, static) {
    TextProcessor2<MarkdownListStrategy> tp;
    tp.append_list({"foo", "bar", "baz"});
    std::cout << tp.str() << std::endl;

    TextProcessor2<HtmlListStrategy> tp2;
    tp2.append_list({"foo", "bar", "baz"});
    std::cout << tp2.str() << std::endl;
}

// summary
// - define an algorithm at a high level.
// - define the interface you expect each strategy to follow.
// - provide for either dynamic or static (template) composition of strategy in the overal algorithm.

} // namespace strategy



