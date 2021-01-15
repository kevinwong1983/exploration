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

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

//////////////////////////////////////////////////////////////////////////////////////////
//      LSP - Liskov’s Substitution Principle
//      Object should be replaceable with instances of their subtypes without altering program correctness.
//History and theory of types
//- it does not matter what is inside a type, all that matters are the operation that can be performed on that type
//- e.g. as long as integer that represents 1, plus integer that represents 2 equals an integer that represents 3
//- thus a type is just a bag of operations (there might be data inside it, but those are hidden behind the operations)
//- that is precisely what a class is looking from outside in.
//Subtypes (e.g. typedef point{ double x,y; } and typedef describedPoint{double x,y; char* description}
//- describedPoint can be cast to point, but not the other way around
//- relationship between the point and describePoint is asymmetrical
//- describe point is a SUBPOINT of point
//        Subtypes - definition of Liskov
//- subtypes can be used as there parent types, without the user knowing it
//- subtypes can be substituted for its parent, subtypes must have same methods as its parents, even though it has different implementations
//Both for static and dynamic
//- static: use inheritance
//- dynamic: use same methods name
//Square paradox: what can go wrong if you use inheritance intuitively instead of according to Liskovs definition
//- a square is a subtype of rectangle…?
//- however when you change height of the square, the with also changes
//- this is something that users don’t expect when they change a rectangle, but a square was passed
//- only way is to use “isInstanceOf(recatangle)” to check… this creates a dependency and violated the OCP
//- Best way of avoid these problems is by keeping square and rectangles as two completely different types, and never pass a square to a function that expect a rectangle
//- the “is a” relationship holds… but the problem is, this is not a rectangle, it is a piece of code that just represents a rectangle.
//- Representatives do not share the relationships of the things that they represent.
//Heuristics, examples and rules to detect violation of LSP
//- if the base class does something, the derived class must do it too. And it must do so in such a way, that it does not violate the expectation of the callers
//- you cannot take expected behaviours away from a subtype, it can do more, but never do less!
//- So if you have empty function (degenerative implementation), probably you have violated LSP
//- So if a derived function just throws an exception, only way to solve it again is using “isInstanceOf()”
//- what you see “if" IsInstanceOf()… this violate LSP when there is also a “else”
//- when you see a typecast
//        Easy to violate the LSP and how to repair them
//- use adepter pattern instead of inheritance

class Rectangle {
protected:
    int width_;
    int height_;
public:
    Rectangle(const int width, const int height) :
            width_(width), height_(height) {
    }

    virtual int GetWidth() const {
        return width_;
    }

    virtual void SetWidth(const int width) {
        this->width_ = width;
    }

    virtual int GetHeight() const {
        return height_;
    }

    virtual void SetHeight(const int height) {
        this->height_ = height;
    }

    int Area() const {
        return width_ * height_;
    }
};

//int MultiplyHeightAndGetArea(Rectangle &r, int multiplier) {
//    auto height = r.GetHeight();
//    r.SetHeight(multiplier * height);
//    return r.Area();
//}

// now we violate LSP and see how it all breaks down!
class Square : public Rectangle {
public:
    Square(int size) : Rectangle{size, size} {
    }

    void SetWidth(const int width) override {
        this->width_ = height_ = width;
    }
    void SetHeight(const int height) override {
        this->height_ = width_ = height;
    }
};

int MultiplyHeightAndGetArea(Rectangle &r, int multiplier) {
//    ...
//    if (dynamic_cast<Square*>(&r)) { // typecheck for square
//        // throw an error or return 0.
//    }
//    ...
    auto height = r.GetHeight();
    r.SetHeight(multiplier * height);
    return r.Area();
}


TEST(lsp, LSP) {
    Rectangle r{5, 5};
    int a = MultiplyHeightAndGetArea(r, 2);
    EXPECT_EQ(a, 50);
    // this all works fine
}

TEST(lsp, LSP_violation) {
    Rectangle r{5, 5};
    int a = MultiplyHeightAndGetArea(r, 2);
    EXPECT_EQ(a, 50);

    Square s{5};
    a = MultiplyHeightAndGetArea(s, 2);
    EXPECT_EQ(a, 0);
//    EXPECT_NE(a, 50); // we expected this to be 50... however is 100:
    // LSP: Object should be replaceable with instances of their subtypes without altering program correctness.
}

// How do mitigate LSP violations? 2. in terms of initialization
// Do not use Square Class
// For initialization: Use a Rectangular Factory instead
struct RectangleFactory {
    static Rectangle MakeRectangle(int w, int h);

    static Rectangle MakeSquare(int size);
};
