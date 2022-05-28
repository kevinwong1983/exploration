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

// Adapter
// A construct which adapts an existing interface X to conform to the required interface Y.

// Motivation

// Classic implementation

class Duck {
public:
    virtual void fly() const = 0;
    virtual void quack() const = 0;
};

class MallardDuck : public Duck{
public:
    MallardDuck(){}

    void fly() const override {
       std::cout << "fly" << std::endl;
    };
    void quack() const override {
        std::cout << "quack" << std::endl;
    };
};

// this is the client
class DuckSimulator {
public:
    void TestDuck(const Duck& duck) {
        duck.fly();
        duck.quack();
    }
};

TEST(adapter, duck) {
    DuckSimulator ds;
    MallardDuck mallardDuck;
    ds.TestDuck(mallardDuck);
}

class Turkey {
public:
     virtual void gobble() = 0;
     virtual void fly() = 0;
};

class WildTurkey : public Turkey {
    void gobble() {
        std::cout << "gobble" << std::endl;
    }
    void fly() {
        std::cout << "fly" << std::endl;
    }
};

// we can't use turkeys in the duck simulator because it expects a duck interface.
// we need to create an adapter.
class TurkeyAdapter : public Duck {
public:
    TurkeyAdapter(std::shared_ptr<Turkey> turkey) :
        turkey_(turkey){};
    void fly() const override{
        turkey_->gobble();
    };
    void quack() const override {
        turkey_->fly();
    };
private:
    std::shared_ptr<Turkey> turkey_;
};

TEST(adapter, turkey_duck_adapter) {
    DuckSimulator ds;
    MallardDuck mallardDuck;
    ds.TestDuck(mallardDuck);

    auto wildTurkey = std::make_shared<WildTurkey>();
    TurkeyAdapter turkeyAdapter(wildTurkey);
    ds.TestDuck(turkeyAdapter);
}

////////////////////////////////////////////////////////////////////////////////////////////////

TEST(adapter, stack_adapter) {
    // stack is an adapter for other container that only exposes necessary functionality .
    std::stack<int> s; // stack gebruik een dequeue als de default container.
    s.push(123);
    int x = s.top();
    s.pop();

    EXPECT_EQ(x, 123);

    //
    std::stack<int, std::vector<int>> s2; //stack implemented with vector, I can do the same
    s2.push(123);
    int x2 = s.top();
    s2.pop();
    EXPECT_EQ(x2, 123);

    // why not just use a vector?
    // you want to communicate intent.
    // you container should behave as a stack e.g.
    // - should not change element in the middle of the stack.
    // - should not iterate stack from begin to end
}

#include <boost/algorithm/string.hpp>

class MyString
{
    std::string s_;
public:
    MyString(const std::string& cs) : s_(cs) {
    }

    MyString to_lower() const {
        std::string ss{s_};
        boost::to_lower(ss);
        return {ss};
    }

    std::vector<std::string> split(const std::string& delimeter = " ") const {
        std::vector<std::string> result;
        boost::split(result, s_, boost::is_any_of(delimeter), boost::token_compress_on);
        return result;
    }
};

TEST(adapter, simple_custom_adapter) {

    // boost has number of function that can operate on a string.
    std::string s{"Hello world"};
    boost::to_lower(s);
    std::vector<std::string> parts;
    boost::split(parts, s, boost::is_any_of(" "));

    EXPECT_EQ(parts.size(), 2);
    EXPECT_EQ("hello", parts[0]);
    EXPECT_EQ("world", parts[1]);

    for (const auto& p: parts){
        std::cout << "<" << p << ">" << std::endl;
    }

    // I want to create a adapter to improve the discoverability of boost functions
    MyString S{"Hello world"};
    auto P = S.to_lower().split();
    EXPECT_EQ( P.size(), 2);
    EXPECT_EQ("hello", P[0]);
    EXPECT_EQ("world", P[1]);
}

// implementing an Adapter is easy:
// Determine the api you have and the api you need.
// Create a component which aggregates (has a reference to, ...) the adaptee.
// Intermediate representation can pile up: use caching and other optimizations.

// Drawing geometric objects on a window.
struct Point {
    int x, y;
};

struct Line {
    Point start, end;
};

struct VectorObject {
    virtual std::vector<Line>::iterator begin() = 0;
    virtual std::vector<Line>::iterator end() = 0;
};

struct VectorRectangle : VectorObject {
    VectorRectangle(int x, int y, int width, int height) {
        lines_.emplace_back(Line{Point{x,y}, Point{x+width, y}});
        lines_.emplace_back(Line{Point{x+width,y}, Point{x+width, y+height}});
        lines_.emplace_back(Line{Point{x,y}, Point{x, y+height}});
        lines_.emplace_back(Line{Point{x,y+height}, Point{x+width, y+height}});
    }

    std::vector<Line>::iterator begin() override{
        return lines_.begin();
    }
    std::vector<Line>::iterator end() override {
        return lines_.end();
    }

private:
    std::vector<Line> lines_;
};


// I have objects in Vector form, but need to render them in raster (pixels) form
// here we need an adapter that takes lines and change them in raster points

std::vector<std::shared_ptr<VectorObject>> vectorObjects{ // these are vector points, but we need to render them in raster form
    std::make_shared<VectorRectangle>(10,10,100,100),
    std::make_shared<VectorRectangle>(30,30,60,60)
};

struct LineToPointAdapter {
    typedef std::vector<Point> Points;

    LineToPointAdapter(Line& line) {
        int left = std::min(line.start.x, line.end.x);
        int right = std::max(line.start.x, line.end.x);
        int top = std::min(line.start.y, line.end.y);
        int bottom = std::max(line.start.y, line.end.y);
        int dx = right - left;
        int dy = line.end.y - line.start.y;

        if (dx == 0) { // isVertical
            for (int y = top; y <= bottom; y++){
                points_.emplace_back(Point{left, y});
            }
        } else if (dy == 0) { // isHorizontal
            for (int x = left; x <= right; x++){
                points_.emplace_back(Point{x, top});
            }
        }
    }

    virtual Points::iterator begin() {
        return points_.begin();
    }

    virtual Points::iterator end() {
        return points_.end();
    }
private:
    Points points_;
};