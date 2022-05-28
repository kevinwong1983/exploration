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

// Visitor Method
// A pattern where a component (visitor) is allowed to traverse the entire inheritance hierarchy.
// Implemented by propagating a single visit() function through the entire hierarchy, that gives controll
// to the visitor.
// - Dispatch: which function to call?
// -- Single Dispatch: function to call, depends on name of request and type of receiver
// -- Double dispatch: function to call, depends on name of request and type of two receivers
// --- type of visitor (visitor that renders html, visitor that renders markdown);
// --- type of element being visited (e.g. paragroph or list item);

// Motivation
// Something we need to define a new operation on an entire class hierarchy.
// - e.g. make a existing document (with headers/paragraphs etc) class pri ntable to HTML/Markdown.
// - we do not want to keep modifying every class in the hierarchy.
// We want: create external componenten (the visitor) to handle the rendering.
// - But avoid type checks


#include "model.h"
#include <algorithm>

namespace visitor {

TEST(visitor, static_visitor) {
    Paragraph p {"Here are some colors"};
    ListItems red{"red"};
    ListItems green{"green"};
    ListItems blue{"blue"};
    List colors{red, green, blue};

    std::vector<Element*> document{&p, &colors};
    std::ostringstream oss;

    std::for_each(document.begin(), document.end(), [&](const Element* e){
        e->print_bad_example(oss);
    });
    std::cout << oss.str() << std::endl;

    // problem here is that for each new print type e.g. Markdown, we need to create another print (print_markdown) here.
    // we now need to modify the while hierarchy chain adding this new print_markdown function.
    // this violated the open-close principle.
    // we want separate components handling the printing.
}

struct HtmlPrinter{
    void Print(const Element* e) {
        // this dynamic cast is ok, but we might forget to update when there is a new type
        const Paragraph* p = dynamic_cast<const Paragraph*>(e);
        if (p) {
            oss << "<p>" << p->text << "</p>" << std::endl;
        }
        // else if.. else if.. // this violated the OCP!
        // a more structured approach is doing double dispatch.
    }

    std::string mystr() const {
        return oss.str();
    }
private:
    std::ostringstream oss;
};

#include "visitor.h"

struct HtmlVisitor : Visitor {
    void visit(const Paragraph &p) override {
        oss << "<p>" << p.text << "</p>" << std::endl;
    }
    void visit(const ListItems &p) override {
        oss << "<li>" << p.text << "</li>" << std::endl;
    }
    void visit(const List &p) override {
        oss << "<ul>" << std::endl;
        for (auto x : p){
            x.accept(*this);
        }
        oss << "</ul>" << std::endl;
    }

    std::string str() const override {
        return oss.str();
    }
private:
    std::ostringstream oss;
};

TEST(visitor, double_dispatch_visitor) {
    Paragraph p {"Here are some colors"};
    ListItems red{"red"};
    ListItems green{"green"};
    ListItems blue{"blue"};
    List colors{red, green, blue};

    std::vector<Element*> document{&p, &colors};

    HtmlVisitor v;
    for (auto x: document) {
        x->accept(v);
    }
    std::cout << v.str() << std::endl;

    // good thing about this is, when you need an other visitor e.g. markdown visitor,
    // you simply need to add another visitor without the need of changing the entire
    // hierarchy. this conforming to OCP
}

struct GameObject;  // forward delare
void collide (GameObject& first, GameObject& second); // forward declare

struct GameObject {
    virtual ~GameObject() = default;
    virtual std::type_index type() const = 0;
    virtual void collide(GameObject& other) {
        visitor::collide(*this, other);
    }
};

template<typename T> struct GameObjectImpl : GameObject {
    std::type_index type() const override { // exposing the type index of a particular type
        return typeid(T);
    }
};

struct Planet : GameObjectImpl<Planet>{};
struct Asteroid : GameObjectImpl<Asteroid>{};
struct Spaceship : GameObjectImpl<Spaceship>{};

struct ArmedSpaceship : Spaceship{ // extend
    std::type_index type() const override { // need to explicitly expose your typeid
        return typeid(ArmedSpaceship);
    }
};

// depending on which two collide with each other, we will have different results
void spaceship_planet() {std::cout<< "spaceship lands on planet" << std::endl;}
void asteroid_planet() {std::cout<< "asteroid burns up in atmosphere" << std::endl;}
void asteroid_spaceship() {std::cout<< "asteroid hist and destroys spaceship" << std::endl;}
void armed_spaceship_asteroid() {std::cout<< "spaceship shoots astroid" << std::endl;}

std::map<std::pair<type_index, type_index>, void(*)(void)>  outcomes {
        { {typeid(Spaceship), typeid(Planet)}, spaceship_planet},
        { {typeid(Asteroid), typeid(Planet)}, asteroid_planet},
        { {typeid(Asteroid), typeid(Spaceship)}, asteroid_spaceship},
        { {typeid(ArmedSpaceship), typeid(Asteroid)}, armed_spaceship_asteroid}};

// we want a general interface that dispatched ont o these game object references
void collide (GameObject& first, GameObject& second) {
    auto it = outcomes.find({first.type(), second.type()});
    if (it == outcomes.end()) { //
        it = outcomes.find({second.type(), first.type()});
        if (it == outcomes.end()){
            std::cout << "object pass each other harmlessly" << std::endl;
            return;
        }
    }
    it->second();
};

TEST(visitor, multiple_dispatch) {
    ArmedSpaceship spaceship;
    Asteroid asteroid;
    Planet planet;

    collide(planet, spaceship);
    collide(planet, asteroid);
    collide(asteroid, spaceship);
    // collide(planet, planet);
    planet.collide(planet);

    // dispatch on multiple arguments in a function.
}

// summary:
// double dispatch:
// -1. propagate a pure virutal accept(Visotor&) function through the entire hierarchy
// -2. create a visitor(interface) with visit(Foo&), visit&Bar) for each element in the hierarchy
// -3. each accept() simply calls v.visit(*this);
// it beats some polymorphic limitation where you need to check typeid's


} // namespace visitor
