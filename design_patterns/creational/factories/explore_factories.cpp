#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>

using namespace std;
// Abstract Factory pattern:
// - Purpose: provide means whereby independently deployable application, can create the objects that they use, without depending on those objects.
// - Using Dependency Inversion Principle, we are able to decouple
//  - App -> IShape <|- Circle/Square
//  - All dependencies point to the Shape Abstraction
//  - This allows us to separate in two components: .
//   - App and IShape
//   - Circle/Square
//  - Allows independent deployment for different reasons:
//          e.g. if you change the Shape derivatives, you do not need to re-deploy the App
//  - Independent deploy-ability means independent develop-abilty
//  - But if the App does not know about the Circle/Square (derivatives), who creates them?
//   - If App needs to know about Circle/Square (derivatives) in order to create them, it violates the DIP!!
//   - App -> Circle/Square
//  - Conclusion: Creating Object always violates DIP and make it difficult to independently deploy objects
//  - Solution: Abstract Factory pattern
//   - IShapeFactory with make_circle and make_square method.
//   - ImplShapeFactory that has create dependency to Circle/Square
//   - How ever there is still a dependence to the number of shaped that can be made. If we add Triangle shape,
//     we need to add make_triangle method to factory
//    - can be solved 1. passing a string to a make. 2 provinding getShapeNames method.
//    - Not typesafe => Typesafety depend on coupling which interfare with independent deployability.
//    - This typesafety “gap” needs to be unit-tested
//   - Abstract Factory pattern becomes problematic when we add new derivatives to an existing hierarchy.
//          Subtle dependencies crosses the red line. This can be solved by using untyped data. We simply
//          pas a string to the factory telling it what to make.
//   - Alternative patterns: Factory Pattern and the Prototype Pattern
//    - However all of them have the same problem: Initialization

// Factory:
// - A separate component responsible solely for the wholesale (not piecewise) creation of objects
// Abstract Factory:
// - A factory construct used to construct object in hierarchies
// Factory Method:
// A function that helps create objects. Like a constructor but more descriptive.

#include <cmath>
#include <iostream>

class Product {
public:
    virtual void doSomethingWithProduct() = 0;
};

class RegularProduct : public Product {
public:
    void doSomethingWithProduct() override {
        std::cout << "I am regular" << std::endl;
    };
};

class SpecialProduct : public Product {
public:
    void doSomethingWithProduct() override {
        std::cout << "I am special" << std::endl;
    };
};

class ProductFactory {
public:
    std::unique_ptr<Product> MakeProduct(std::string type) {
        if (type == "regular") {
            return std::make_unique<RegularProduct>();
        } else if  (type == "special") {
            return std::make_unique<SpecialProduct>();
        }
    }
};

TEST(SimpleFactory, simple) {
    auto factory = ProductFactory();
    std::vector<std::unique_ptr<Product>> products;
    products.emplace_back(factory.MakeProduct("regular"));
    products.emplace_back(factory.MakeProduct("special"));

    for (auto& p : products){
        p->doSomethingWithProduct();
    }
}

// carthesian vs polar Points
class CarthesianPoint {
public:
    float x, y;

    CarthesianPoint(const float x, const float y) : x(x), y(y) { // can only take in carthesian coirdinated
    }
//    CarthesianPoint(const float r, const float theta) { // this give error, constructor cannot be recreated
//    }
};

// bad example !!!
enum class PointType {
    carthesian,
    polar
};
class CarthesianOrPolarPoint {
public:
    float x, y;

    //THIS IS VERY BAD !!!! Lets improve this using 1.Factory Methods, 2.Factory and 3.Abstract Factory patterns
    CarthesianOrPolarPoint(const float a, const float b, PointType type = PointType::carthesian) {
        if (type == PointType::carthesian){
            x = a;
            y = b;
        }
        else {
            x = a * cos(b);
            y = a * sin(b);
        }
    }
};

// 1. Factory methods
// advantage: you can give your factory method a chosen name (while constructor needs to be of the same name as the class)
class Point_UsingFactoryMethod {
public:
    float x_, y_;

    // Factory Methods:
    static Point_UsingFactoryMethod MakeCarthesian(const float x, const float y) {
        return Point_UsingFactoryMethod(x,y);
    }

    static Point_UsingFactoryMethod MakePolar(const float r, const float theta) {
        return Point_UsingFactoryMethod(r * cos(theta), r * sin(theta));
    }

private:
    Point_UsingFactoryMethod(const float x, const float y) : x_(x), y_(y) {     // private constructor
    }

    friend class PointFactory;      // Factory needs to be friend of this class to get access to private members/methods
};

TEST(FactoryMethod, simple_factory_method) {
    // constructor is  private: give error calling private constructor
    // Point_UsingFactoryMethod a(1,2);

    // use factory method
    auto p = Point_UsingFactoryMethod::MakeCarthesian(1,2);
    EXPECT_EQ(p.x_, 1);
    EXPECT_EQ(p.y_, 2);

    auto c = Point_UsingFactoryMethod::MakePolar(5, M_PI_4);
    EXPECT_NEAR(c.x_, 3.5, 0.1);
    EXPECT_NEAR(c.y_, 3.5, 0.1);
}

// 2. Factory Class pattern vs. Factory Method Pattern
// Only difference is that it is a separate object that creates the Point
class PointFactory {
public:
    // Factory Methods:
    static Point_UsingFactoryMethod MakeCarthesian(const float x, const float y) {
        return Point_UsingFactoryMethod(x,y);   // needs to be "friends" of what it makes
    }

    static Point_UsingFactoryMethod MakePolar(const float r, const float theta) {
        return Point_UsingFactoryMethod(r * cos(theta), r * sin(theta));
    }
};

TEST(Factory, simple_factory) {
    // constructor is  private: give error calling private constructor
    // Point_UsingFactoryMethod a(1,2);

    // use factory method
    auto p = PointFactory::MakeCarthesian(1,2);
    EXPECT_EQ(p.x_, 1);
    EXPECT_EQ(p.y_, 2);

    auto c = PointFactory::MakePolar(5, M_PI_4);
    EXPECT_NEAR(c.x_, 3.5, 0.1);
    EXPECT_NEAR(c.y_, 3.5, 0.1);
}


#include <memory>
#include <map>

// 3. Abstract Factory:
// until now we only looked at single types
// lets look at multiple types in hierarchy
struct HotDrink {
    virtual void prepare(int volume) = 0;
    virtual ~HotDrink() {
        std::cout << "bla" << std::endl;
    }
};

struct Tea : HotDrink {
    void prepare(int volume) {
        std::cout << "Take tea bag, boil water, serve " << volume << " ml" <<std::endl;
    }
};

struct Coffee : HotDrink {
    void prepare(int volume) {
        std::cout << "Grind Coffee beans, boil water, serve " << volume << " ml" <<std::endl;
    }
};

// This is NOT what you want. Each time you add a new derivitave, this needs to be changed.
std::unique_ptr<HotDrink> make_drink(std::string type){
    std::unique_ptr<HotDrink> drink;
    if (type == "tea") {
        drink = std::make_unique<Tea>();
        drink->prepare(200);
    }
    else {
        drink = std::make_unique<Coffee>();
        drink->prepare(200);
    }
    return drink;
}

TEST(AbstractClass, simple_abstract_factory) {
    auto drink = make_drink("tea");
    EXPECT_TRUE(dynamic_cast<Tea*>(drink.get()));
}

// abstract factory
struct HotDrinkFactory {
    virtual std::unique_ptr<HotDrink> make() = 0;
    virtual ~HotDrinkFactory() {
        std::cout << "bla" << std::endl;
    }
};

//
struct CoffeeFactory : HotDrinkFactory {
    std::unique_ptr<HotDrink> make() override {
        return std::make_unique<Coffee>();
    }
};

struct TeaFactory : HotDrinkFactory {
    std::unique_ptr<HotDrink> make() override {
        return std::make_unique<Tea>();
    }
};

// we are going to have families of factories addition to the families of types
// we do mapping of the strings to the types
class DrinkFactory {
public:
    DrinkFactory() {
        factories_["coffee"] = std::make_unique<CoffeeFactory>();
        factories_["tea"] = std::make_unique<TeaFactory>();
    }
    std::unique_ptr<HotDrink> make(const std::string& name) {
        auto drink = factories_[name]->make();
        drink->prepare(200);
        return drink;
    }
private:
    std::map<std::string, std::unique_ptr<HotDrinkFactory>> factories_;
};

TEST(AbstractClass, simple_abstract_factory_correct_way) {
    DrinkFactory df;
    auto drink = df.make("coffee");
    EXPECT_TRUE(dynamic_cast<Coffee*>(drink.get()));

    auto drink2 = df.make("tea");
    EXPECT_TRUE(dynamic_cast<Tea*>(drink2.get()));
}


// we are going to have families of factories addition to the families of types
// we do mapping of the strings to the types
class DrinkFactory_2 {
private:
    static std::map<std::string, std::unique_ptr<HotDrinkFactory>> factories_;
public:
    DrinkFactory_2() {
    }
    static std::unique_ptr<HotDrink> make(const std::string& name) {
        auto drink = factories_[name]->make();
        return drink;
    }
    static  void registers(const std::string& name, std::unique_ptr<HotDrinkFactory> factory) {
        factories_[name] = std::move(factory);
    }
};

std::map<std::string, std::unique_ptr<HotDrinkFactory>> DrinkFactory_2::factories_;

TEST(AbstractClass, simple_abstract_factory_correct_way2) {
    DrinkFactory_2::registers("coffee", std::make_unique<CoffeeFactory>());

    auto drink = DrinkFactory_2::make("coffee");
    EXPECT_TRUE(dynamic_cast<Coffee*>(drink.get()));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "animalFactory.h"
#include "animalMock.h"
#include "dog.h"
#include "ape.h"

class zoo {
    zoo() : animal_(animalFactory::make("dog")) {
        animal_->talk();
        animal_->walk();
    }

    std::unique_ptr<animal> animal_;
};

using testing::Return;

TEST(time, factory_test) {
//    auto a = animalFactory::make("ape");
//    EXPECT_TRUE(dynamic_cast<ape *>(a.get()));
//    auto d = animalFactory::make("dog");
//    EXPECT_TRUE(dynamic_cast<dog *>(d.get()));
//
//    auto amf = std::make_unique<animalMockFactory>();
//
//    //
//    std::unique_ptr<animalMock> mock = std::make_unique<animalMock>();
//    EXPECT_CALL(*mock, talk())
//            .Times(1)
//            .WillOnce(Return("hey I talk like a mock"));
//
//    amf->setReturnMock(std::move(mock));
//    animalFactory::registers("mock", std::move(amf));
//
//    auto m = animalFactory::make("mock");
//    EXPECT_TRUE(dynamic_cast<animalMock *>(m.get()));
//    EXPECT_EQ("hey I talk like a mock", m->talk());
//
//    //
//    std::unique_ptr<animalMock> mock2 = std::make_unique<animalMock>();
//    EXPECT_CALL(*mock2, talk())
//            .Times(1)
//            .WillOnce(Return("hey I talk like a mock"));
//
//    amf->setReturnMock(std::move(mock2));
//    animalFactory::registers("dog", std::move(amf));    // dog factory is now overwritten with mock factory
//
//    auto n = animalFactory::make("dog");
//    EXPECT_TRUE(dynamic_cast<animalMock *>(n.get()));
//    EXPECT_EQ("hey I talk like a mock", n->talk());
}

TEST(time, factory_test2) {
    auto animal_mock_factory = std::make_unique<animalMockFactory>();
    std::unique_ptr<animalMock> mock = std::make_unique<animalMock>();
    EXPECT_CALL(*mock, talk())
            .WillOnce(Return("hey I talk like a mock"));
    EXPECT_CALL(*animal_mock_factory, make())
            .WillOnce(Return(testing::ByMove(std::move(mock))));

    animalFactory::registers("dog",
                             std::move(animal_mock_factory));    // dog factory is now overwritten with mock factory

    auto n = animalFactory::make("dog");
    EXPECT_TRUE(dynamic_cast<animalMock *>(n.get()));
    EXPECT_EQ("hey I talk like a mock", n->talk());
}

TEST(time, polymorphism) {
    // this will not work: When you need polymorphism, you need to use either pointers or references
    // And since references can only be bound once, you cannot really use them in standard containers.
    auto d = std::make_unique<dog>();
    auto a = std::make_unique<ape>();
    auto animals = std::vector<std::unique_ptr<animal>>{};
    animals.emplace_back(std::move(d));
    animals.emplace_back(std::move(a));
    for(auto&& animal: animals) {
        animal->talk();
        animal->walk();
    }
}