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

// Motivation: When peicewise object construction is complicated, provide an API for doing it succinctly
// Builder
// Fluent Builder
// Groovy-style builder
// Builder Facets

TEST(builder, simple_builder) {
    // <p>hello</p>

    // without builder: need to type "start" and "end" tag
    auto text = "hello";
    std::string output;
    output += "<p>";
    output += text;
    output += "</p>";
    std::cout << output << std::endl;

    //
    printf("<p>%s</p>\n", text);

    // however this is not general.
    // more difficult example
    // <ul><li>hello</li><li>world</li></ul>
    std::string words[] = {"hello", "world"};
    std::ostringstream oss;
    oss << "<ul>";
    for (auto w:words){
        oss << "<li>" << w << "</li>";
    }
    oss << "</ul>";
    printf("%s", oss.str().c_str());

    // it would be nice to have a OO way of doing this.
}

struct HtmlBuilder;
struct HtmlFluentBuilder;

struct HtmlElement {
    std::string name;
    std::string text;
    std::vector<HtmlElement> elements;
    const size_t indent_size = 2;

    HtmlElement(){
    }
    HtmlElement(const std::string& name, const std::string& text) : name{name}, text{text} {
    }

    std::string str(int indent = 0) const {
        std::ostringstream oss;
        std::string i(indent_size * indent, ' ');
        oss << i << "<" << name << ">"  << std::endl;

        if (text.size() > 0) {
            oss << std::string(indent_size * (indent + 1), ' ') << text << std::endl;
        }

        for (const auto& e : elements) {
            oss << e.str(indent + 1);
        }

        oss << i << "</" << name << ">" << std::endl;
        return oss.str();
    }

    static HtmlFluentBuilder build(std::string root_name);
    static std::unique_ptr<HtmlFluentBuilder> create(std::string root_name);
};

struct HtmlBuilder {
    HtmlBuilder(std::string root_name) {
        root.name = root_name;
    }

    void add_child(std::string child_name, std::string child_text) {
        HtmlElement e {child_name, child_text};
        root.elements.emplace_back(e);
    }

    std::string str() const {
        return root.str();
    }

    HtmlElement root;
};

TEST(builder, using_builder) {
    HtmlBuilder builder{"ul"};
    builder.add_child("li", "hello");
    builder.add_child("li", "world");
    std::cout << builder.str() << std::endl;
    //building an object from parts with an convenient api
}

struct HtmlFluentBuilder {
    HtmlFluentBuilder(std::string root_name) {
        root.name = root_name;
    }

    HtmlFluentBuilder& add_child(std::string child_name, std::string child_text) {
        HtmlElement e {child_name, child_text};
        root.elements.emplace_back(e);
        return *this;
    }

    HtmlFluentBuilder* add_child2(std::string child_name, std::string child_text) {
        HtmlElement e {child_name, child_text};
        root.elements.emplace_back(e);
        return this;
    }

    std::string str() const {
        return root.str();
    }

    operator HtmlElement() {
        return root;
    }

    HtmlElement root;
};

HtmlFluentBuilder HtmlElement::build(std::string root_name) {
    return HtmlFluentBuilder{root_name};
}

std::unique_ptr<HtmlFluentBuilder> HtmlElement::create(std::string root_name) {
    return std::make_unique<HtmlFluentBuilder>(root_name);
}

// fluent builder: chain several calls to the builder
TEST(builder, using_fluent_builder) {
    HtmlFluentBuilder builder{"ul"};
    builder.add_child("li", "hello")
        .add_child("li", "world")
        .add_child("bla", "kitty");
    HtmlElement e = builder;
    std::cout << e.str() << std::endl;

    auto e2 = HtmlElement::build("ul")
            .add_child("li", "hello")
            .add_child("li", "world");
    std::cout << e2.str() << std::endl;

    auto e3 = HtmlElement::create("daan")
            ->add_child2("asdf","gijs")
            ->add_child2("asdw", "mier");
    std::cout << e3->str() << std::endl;

    //building an object from parts with an convenient api
}

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "Person.h"
#include "PersonBuilder.h"
#include "PersonAddressBuilder.h"
#include "PersonJobBuilder.h"

TEST(builder, facet_builder) {
    Person p = Person::create()
        .lives().at("123 London Road").with_postcode("SW 1GB").in("London")
        .works().at("PragmaSoft").as_a("Consultant").earning(10e6);

    std::cout << p << std::endl;
}

class CarBuilder;   // forward declare

class Car {
private:
    int doors;
    string color;
    string motor;
    Car() = default;
public:
    static CarBuilder create();
    friend CarBuilder;

    friend std::ostream& operator << (std::ostream &os, const Car &obj) {
        return os << "doors: " << obj.doors
            << " color: " << obj.color
            << " motor: " << obj.motor;
    }
};

class CarBuilder {
private:
    typedef CarBuilder Self;
    Car c;
public:
    Car& car;

    explicit CarBuilder(Car& car) : car{car} {}

    CarBuilder() : car {c} {}

    operator Car()
    {
        return car;
    }

    // setters
    Self& withDoors(int doors) {
        car.doors = doors;
        return *this;
    }
    Self& withMotor(string motor) {
        car.motor = motor;
        return *this;
    }
    Self& withColor(string color) {
        car.color = color;
        return *this;
    }
};

CarBuilder Car::create() {
        return CarBuilder();
};

TEST(builder, simple_facet_builder) {
    Car c = Car::create()
            .withDoors(4)
            .withMotor("tesla motor")
            .withColor("red");

    std::cout << c << std::endl;
}