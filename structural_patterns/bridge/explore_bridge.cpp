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

// Bridge
// A mechanism that decouples an abstraction (hierarchy) from an implementation (hierarchy),
// so that the two can vary independently.

// The bridge pattern is useful when both the class and what it does vary often. The class
// itself can be thought of as the abstraction and what the class can do as the implementation.
// The bridge pattern can also be thought of as two layers of abstraction.

// When there is only one fixed implementation, this pattern is known as the Pimpl idiom in the C++ world.

//When:
//
//            A
//         /     \
//       Aa      Ab
//      / \     /  \
//    Aa1 Aa2  Ab1 Ab2

//Refactor to:
//
//       A         N
//    /     \     / \
//  Aa(N) Ab(N)  1   2

// Pimple idiom

// Person.h
struct Person {
    std::string name_;

    struct PersonImpl;  // inner class that implements
    PersonImpl* impl_;  // pointer to implementation

    Person();
    ~Person();

    void greet();
};

// Person.cpp
struct Person::PersonImpl {
    void greet(Person* p);
};

void Person::PersonImpl::greet(Person* p) {
    std::cout << "Hello, my name is " << p->name_ << std::endl;
}

Person::Person() : impl_(new PersonImpl){
}

Person::~Person() {
    delete impl_;
}

void Person::greet() {
    impl_->greet(this);
}


TEST(bridge, pimple) {
   Person p;
   p.name_ = "john";
   p.greet();

   // goal of pimple idiom:
   // 1. keep the interface of Person unchanged. If the Person interace changes, you need to
   // recompile every cpp file that uses this interface.
   // 2. In addition it is used to hide the implementation details, so you dont have to ship
   // them to the end users

   // pimpl idiom is the foundation to the bridge pattern.
   //
}

/////////////////////////////////////////////////////////////////////////////////////

struct Renderer {
    virtual void render_circle(float x, float y, float radius) = 0;
};

struct VectorRender : Renderer {
    void render_circle(float x, float y, float radius) override {
        std::cout << "Drawing a vector circle of radius " << radius << std::endl;
    }
};

struct RasterRenderer : Renderer {
    void render_circle(float x, float y, float radius) override {
        std::cout << "Rasterizing a circle of radius " << radius << std::endl;
    }
};

// the Shape class is going to be the bridge between the different shapes an how they are to be rendered.
struct Shape {
protected:
    Renderer &renderer;// this base class is taking a reference of who is going to render it.
    Shape(Renderer& renderer) : renderer {renderer} {}

public:
    virtual void draw() = 0;
    virtual void resize(float factor) = 0;
};

struct Circle : Shape {
    float x, y, radius;

    Circle(Renderer& renderer, const float x, const float y, const float radius) :
        Shape{renderer},
        x{x},
        y{y},
        radius{radius}
    {
    }

    void draw() override {
        renderer.render_circle(x,y,radius);
    }
    void resize(float factor) override {
        radius *= factor;
    }
};

TEST(bridge, simple_bridge) {
    RasterRenderer rr;
    Circle raster_circle {rr, 10, 10, 5};
    raster_circle.draw();
    raster_circle.resize(2);
    raster_circle.draw();
}

// At first sight, the Bridge pattern looks a lot like the Adapter pattern in that a class
// is used to convert one kind of interface to another. However, the intent of the Adapter
// pattern is to make one or more classes' interfaces look the same as that of a particular
// class. The Bridge pattern is designed to separate a class's interface from its
// implementation so you can vary or replace the implementation without changing the client code.

// summary:
// bridge pattern decouples abstraction from implemenation
// A stronger form of encapsulation: by splitting types into different classes and files
// and just put in a single link between the two, so the two parts can talk to each other