#include "boost/variant.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>
#include <stack>

// Decorator
// Allows for adding behavior to individual objects without affecting the behavior of other objects of the same class.
// e.g. we are not interfering with class hierarchy, we are not defining interfaces. we are just adding new functionality
// to the side. And we make our best effort to make the functionality accessible and usable in terms of constructor invocations.

// motivation
// We want to augment existing functionality, however we do not want to rewrite or alter existing code (OCP).
// We also want to keep new functionality separate (SRP).
// Need to be able to interact with existing structures.

namespace decorator {

class Beverage {
protected:
    std::string description_ = "Unknown Beverage";
public:
    virtual std::string getDescription() {
        return description_;
    }
    virtual double cost() = 0;
};

class DarkRoast : public Beverage {
public:
    DarkRoast(){
        description_ = "Dark Roast Coffee";
    }
    double cost() {
        return 0.99;
    }
};

class CondimentDecorator : public Beverage {
    virtual std::string getDescription() = 0;
    virtual double cost() = 0;
};

class Whip : public CondimentDecorator {
private:
    std::shared_ptr<Beverage> beverage_;
public:
    Whip (std::shared_ptr<Beverage> beverage): beverage_(beverage) {
    }
    std::string getDescription() {
        return beverage_->getDescription() + ", Whip";
    };
    double cost() {
        return beverage_->cost() + 0.10;
    };
};

TEST(decorator, StarBuzzCoffee) {
    std::shared_ptr<Beverage> beverage = std::make_shared<DarkRoast>();
    std::cout << beverage->getDescription() << std::endl;
    std::cout << beverage->cost() << std::endl;

    beverage = std::make_shared<Whip>(beverage);
    std::cout << beverage->getDescription() << std::endl;
    std::cout << beverage->cost() << std::endl;
}

////////////////////////////////////////////////////////////////////////

// here we create a decorator logger.
struct Logger {
    std::function<void()> func_;
    std::string name_;

    Logger(const std::function<void()> func, const std::string &name)
            : func_{func},
              name_{name} {
    }

    void operator()() const {
        std::cout << "entering " << name_ << std::endl;
        func_();
        std::cout << "exiting " << name_ << std::endl;
    }
};

TEST(decorator, simpel_functional) {
    Logger logger{[]() { std::cout << "Hello" << std::endl; }, "HelloFunc"};
    logger();
    // problem with this logger is it does not take any argument
}

template<typename Func>
struct Logger2 {
    Func func_;
    std::string name_;

    Logger2(Func func, const std::string &name)
            : func_{func},
              name_{name} {
    }

    void operator()() const {
        std::cout << "entering " << name_ << std::endl;
        func_();
        std::cout << "exiting " << name_ << std::endl;
    }
};

// templated function, to invire the type
template<typename Func>
auto make_logger2(Func func, const std::string &name) {
    return Logger2<Func>{func, name};
}

TEST(decorator, simpel_functional_with_argument) {
    // functional decorators lets you wrap functions with befor/after code for e.g. logging, latency measurements
    auto logger = make_logger2([]() { std::cout << "Hello" << std::endl; }, "HelloFunc");

    logger();
}

// we want the logger (the decorator) to behave in a similar way as the actual function

template<typename>
struct Logger3;

// the only way this will work is with partial specialization
template<typename R, typename... Args>
struct Logger3<R(Args...)> {
    std::function<R(Args...)> func;
    std::string name;

    Logger3(const std::function<R(Args...)> &func, const std::string &name)
            : func{func},
              name{name} {
    }

    R operator()(Args ...args) {
        std::cout << "Entering" << name << std::endl;
        R result = func(args...);
        std::cout << "Exiting" << name << std::endl;
        return result;
    }
};

template<typename R, typename... Args>
auto make_logger3(R (*func)(Args...), const std::string &name) {
    return Logger3<R(Args...)>{std::function<R(Args...)>(func), name};
}

double add(double a, double b) {
    std::cout << a << " + " << b << " = " << (a + b) << std::endl;
    return a + b;
}

TEST(decorator, decorator_behaving_similar_as_actual_function) {
    auto logged_add = make_logger3(add, "Add");
    auto result = logged_add(2, 3);
    // this is a bit more complex because you need to proxy the argument from decorator to function
    // we use variadic templates
}

//////////////////////////////////////////////////////////////////////////////////////////

// we have shapes and we want to have colors for these shapes
struct Shape {
    virtual std::string str() const = 0;
};

// we can hava s Colored shape...
//struct ColorShape {
//    std::string color;
//    explicit ColorShape(const std::string& color) : color{color} {
//    }
//};

// and then change inheretance from Shape to Color Shape...
// however, what if we do not have the source code of Cirlce...
// also we are violating the OCP
// SOLUTION: decorator pattern allows us to add additional traits to these shapes without interfering with the baseclass definition
struct Circle : Shape {
    float radius;

    Circle() = default;

    explicit Circle(const float radius) : radius{radius} {
    }

    std::string str() const override {
        std::ostringstream oss;
        oss << "A circle of radius " << radius;
        return oss.str();
    }

    void resize(int factor){
        radius *= factor;
    }
};

struct Square : Shape {
    float side;

    explicit Square(const float side) : side{side} {
    }

    std::string str() const override {
        std::ostringstream oss;
        oss << "A square with side " << side;
        return oss.str();
    }
};

// decorator colored shape
struct ColoredShape : Shape {
    Shape &shape;
    std::string color;

    ColoredShape(Shape &shape, const std::string &color)
            : shape{shape},
              color{color} {
    }

    std::string str() const override {
        std::ostringstream oss;
        oss << shape.str() << " has the color " << color;
        return oss.str();
    }
};

// decorator transparant shape
struct TransparantShape : Shape {
    Shape &shape;
    uint8_t transparency;

    TransparantShape(Shape &shape, uint8_t transparency)
            : shape{shape},
             transparency{transparency} {
    }

    std::string str() const override {
        std::ostringstream oss;
        oss << shape.str() << " with " << static_cast<float>(transparency)/255.f * 100 << " % traparency";
        return oss.str();
    }
};

TEST(decorator, wrapping_decorator) {
    // this aggregate decorator does not give you the undrelying object's feature, but can be composed at runtime.

    Circle circle{5};
    std::cout << circle.str() << std::endl;

    // color decorator
    ColoredShape red_circle {circle, "red"};
    std::cout << red_circle.str() << std::endl;

    // transparancy decorator
    TransparantShape half_transparent_circle{circle, 128};
    std::cout << half_transparent_circle.str() << std::endl;

    // combined color and transparancy decorators
    TransparantShape hald_tr_red_circle {red_circle, 128};
    std::cout << hald_tr_red_circle.str() << std::endl;

    // you can have a decorator of a decorator
    // there are a bunch of issues with the wrapping decorator:
    // 1. if circle has a resize function, the decorated (color and transparent) circle do not have this in there api. because
    // the decorators are inherited from shape, not circle.
    //      - we can add resize to Shape instead, but what if the shape does not support resize, then we need to throw exceptions etc.
    // hald_tr_red_circle.resize() not available
    // 2. we need to create a circle first before decorating it.

}


// the Mixin Inheritance solve this.

template <typename T>
struct ColoredShape2 : T {
    std::string color;

    // the decorator accepts everything, we want to limit it to shapes
    static_assert(std::is_base_of<Shape, T>::value, "Template argument must be a Shape");

    ColoredShape2() = default;

    explicit ColoredShape2(const std::string& color)
     : color{color}{
    }

    std::string str() const override {
        std::ostringstream oss;
        oss << T::str() << " has the color " << color;
        return oss.str();
    }
};

template <typename T>
struct TranparentShape2 : T {
    uint8_t transparency;

    TranparentShape2() = default;

    TranparentShape2(const uint8_t transparency)
            : transparency{transparency}{
    }

    std::string str() const override {
        std::ostringstream oss;
        oss << T::str() << " with " << static_cast<float>(transparency)/255.f * 100 << " % traparency";
        return oss.str();
    }
};

TEST(decorator, mixin_inheritance) {
    // A decoratore based on mixin inheritance is more flexible, exposes underlying object's features,
    // but is only constructible at compile time (using templates)

    ColoredShape2<Circle> red_circle{"red"};
    red_circle.radius = 5;
    std::cout << red_circle.str() << std::endl;

    TranparentShape2<Circle> half_tr_circle{128};
    half_tr_circle.radius = 5;
    std::cout << half_tr_circle.str() << std::endl;

    // combine the decorators
    TranparentShape2<ColoredShape2<Circle>> half_tr_red_circle{128};
    half_tr_red_circle.color = "red";
    half_tr_red_circle.radius = 100;
    std::cout << half_tr_red_circle.str() << std::endl;

    // also not resize is available.
    half_tr_red_circle.resize(4);
    std::cout << half_tr_red_circle.str() << std::endl;
}



} // namespace decorator
