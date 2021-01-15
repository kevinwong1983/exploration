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

// Compositite:
// A mechanism for treating individual (scalar) and compositions of objects in a uniform manner.
// With polymorpysm you might swap on with the other.

// Motivation
// Objects use other objects's fields and functions through 1. inheritance 2. composition

// Composition lets ua make compound objects e.g.
// - a mathemativacl expression composed of simpler expression
// - A grouping of shapes that make up several shapes

// Composition design pattern us used to treat both single and composite objects uniformly. (e.g. with identical API's)

// 2 + (3 + 4)
struct Expression {
    virtual double eval() = 0;
    // if i want ot get all the composite expression I need to collect them
    virtual void collect(std::vector<double>& v) = 0; //visitor
};

struct Literal : Expression {
    double value_;

    explicit Literal(const double value) : value_{value} {
    }

    double eval() override {
        return value_;
    }

    void collect(std::vector<double>& v) override {
        v.push_back(value_);
    }
};

struct AdditionExpression : Expression {
    std::shared_ptr<Expression> left, right;

    AdditionExpression(const std::shared_ptr<Expression> &expression1,
                       const std::shared_ptr<Expression> expression2)
            : left{expression1},
              right{expression2} {
    }

    double eval() override {
        return left->eval() + right->eval();
    }

    void collect(std::vector<double>& v) override {
       left->collect(v);
       right->collect(v);
    }
};

TEST(composite, simple) {
    AdditionExpression sum {
        std::make_shared<Literal>(2),
                std::make_shared<AdditionExpression>(
                        std::make_shared<Literal>(3),
                        std::make_shared<Literal>(4)
                        )
    };
    std::cout << "2+(3+4) = " << sum.eval() << std::endl;

    std::vector <double> v;
    sum.collect(v);
    for (auto& x: v){
        std::cout<< x << "\t";
    }
}

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
using namespace boost::accumulators;

TEST(composite, boost_accumulate) {
    // get avarage of value
    std::vector <double> values{1,2,3,4};
    double s = 0;
    for (auto& x : values) {
        s += x;
    }
    std::cout << "avarage is " << (s / values.size()) << std::endl;
    // disadvantage: 1. risk of overflow 2. need begin() and end()

    // using boost
    accumulator_set<double, stats<tag::mean>> acc;
    for (auto x : values) {
        acc(x);
    }
    std::cout << "average is: " << mean(acc) << std::endl;
}

////////////////////////////////////////////////////////////////////////
struct GraphicsObject {
    virtual void draw() = 0;
};

struct MyCircle : GraphicsObject {
    void draw() override {
        std::cout<< "Cirlce" << std::endl;
    }
};

struct Group : GraphicsObject {
    std::string name_;
    std::vector<GraphicsObject*> objects_;

    Group(const std::string& name) : name_{name} {
    }

    void draw() override {
       std::cout << "Group " << name_ << " contains: " << std::endl;
       for (auto&& o : objects_) {
           o->draw();
       }
    }
};

TEST(composite, geometric_shapes) {
    MyCircle c1, c2;

    Group root("root");
    root.objects_.push_back(&c1);

    Group subgroup("sub");
    subgroup.objects_.push_back(&c2);

    root.objects_.push_back(&subgroup); // dit kan omdat group ook zelfde interface heeft van GraphicsObject

    root.draw(); // this draws all the graphical objects in the "tree", from the root down.
    // we have a uniform interface that treed both singular objects as groups in a uniform manner.
}

////////////////////////////////////////////////////////////////////////
using namespace std;

struct Neuron {
    std::vector <Neuron*> in, out;
    int id;

    Neuron() {
        static int id = 1;
        this->id = id ++;
    }

    // There is no base class for containers.
    // Hack so you are able to us iterators in the templates
    Neuron* begin() {return this;}
    Neuron* end() {return this+1;}

    void connect_to (Neuron& other) {
        out.push_back(&other);
        other.in.push_back(this);
    }

    // to connecto_to support for all 4 different variations:
    template <typename T> void connect_to(T& other) {
        for (Neuron& target : other) {
            connect_to(target);
        }
    }

    friend ostream& operator << (ostream& os, const Neuron& obj) {
        for (Neuron* n: obj.in) {
            os << n->id << "\t-->\t[" << obj.id << "]" <<std::endl;
        }
        for (Neuron* n: obj.out) {
            os << "[" << obj.id << "]\t-->\t" << n->id <<std::endl;
        }
        return os;
    }
};

struct NeuronLayer : std::vector<Neuron> {
    NeuronLayer (int count) {
        while (count-- > 0) {
            emplace_back(Neuron{});
        }
    }

    template <typename T> void connect_to(T& other) {
        for (Neuron& from : *this) {
            for (Neuron& to : other) {
                from.connect_to(to);
            }
        }
    }

    friend ostream& operator << (ostream& os, const NeuronLayer& obj) {
        for (auto& n : obj) os << n;
        return os;
    }
};

// how can we treed a connection of neurons and a single neuron in a similar fashion.
// for connect_to we need connect 1. neuron to neuron 2. neuron to neuronlayer 3. neuronlayer to neuron 4 neuronlayer to neuronlayer
// how can we use composite to only have one function


TEST(composite, neurons) {
    Neuron n1, n2;
    n1.connect_to(n2);

    std::cout << n1 << n2 << std::endl;

    NeuronLayer l1{5};
    Neuron n3;
    l1.connect_to(n3);
    std::cout << "l1\n" << l1 << std::endl;
    std::cout << "n3\n" << n3 << std::endl;

    NeuronLayer l2{2}, l3{3};
    l2.connect_to(l3);
    std::cout << "l2:\n" << l2 << std::endl;
    std::cout << "l3:\n" << l3 << std::endl;
}

// Summary
// Objects use other objects's fields and functions through 1. inheritance 2. composition
// Some coposed and singular objects need similar/identical behaviors
// Composite design pattern let use treat both types of objects uniformly
// c++ has no special support for the idea of "enumeration" of objects, so we need single objects to 'masquerade' to
// become suitable for begin/end iteration.

