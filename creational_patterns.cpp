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
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

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

/////////////////////// Prototype Pattern ///////////////////////

struct Address {
    std::string street;
    std::string city;
    int suite;

//  Streaming
//  The stream operators:
//  - operator << output
//  - operator >> input
//  When you use these as stream operators (rather than binary shift)
//  the first parameter is a stream. Since you do not have access to
//  the stream object (its not yours to modify) these can not be member
//  operators they have to be external to the class. Thus they must
//  either be friends of the class or have access to a public method
//  that will do the streaming for you.
//
//  "friend" s a way to extended the public interface without breaking encapsulation
//
//   It is also traditional for these objects to return a reference
//   to a stream object so you can chain stream operations together.

    friend std::ostream& operator<<(std::ostream& os, const Address& obj) {
        return os
            << "street: " << obj.street
            << " city: " << obj.city
            << " suite: " << obj.suite;
    }
};

struct Contact{
    std::string name;
    Address* work_address;

//    Contact(const std::string& name, Address* const work_address) : name(name), work_address(new Address(*work_address)) {
//    }
//
//    ~Contact() {
//        delete work_address;
//    }
//
//    Contact(const Contact& other) : name(other.name), work_address(new Address(*other.work_address)) {
//    } // rule of 5: when you define a copy constructor, you also need to create a constructor

    friend std::ostream& operator<<(std::ostream& os, const Contact& obj) {
        return os
                << "name: " << obj.name
                << " work_address: " << *obj.work_address;
    }
};

struct Contact2 {
    std::string name;
    Address* work_address;

    //constructor
    Contact2(const std::string& name, Address* const work_address) : name(name), work_address(new Address(*work_address)) { // deep copy
    }

    //destructor
    ~Contact2() {
        delete work_address;
    }

    //copy-constructor
    Contact2(const Contact2& other) : name(other.name), work_address(new Address(*other.work_address)) {    //deep copy
    } // rule of 5: when you define a copy constructor, you also need to create a constructor

    friend std::ostream& operator<<(std::ostream& os, const Contact2& obj) {
        return os
                << "name: " << obj.name
                << " work_address: " << *obj.work_address;
    }
};

TEST(prototype_pattern, simple_prototype_problem) {
    Address* addr = new Address {"123 East Dr", "London"};  // prefilled address -> this is the prototype!
    Contact John {"John Doe", addr};        // Constructor does a shallow copy, pointer is copied in stead of value
    Contact Jane {"jane Doe", addr};

    // we want to keep using the prefilled adress, but we want to update the attributes
    John.work_address->suite = 100;
    Jane.work_address->suite = 123;

    std::cout << John << std::endl;
    std::cout << Jane << std::endl;
    EXPECT_EQ(123, John.work_address->suite);    // pointer is copied in stead of value
    EXPECT_EQ(123, Jane.work_address->suite);

    // What do not want John and Jane to share the copy of address.
    // With default copy constructers we have the same problem.
    // default constructors do a shallow copy.
    // Jill copied from Jane will point to the same address.
    Contact Jill {Jane};                            // Copy does a shallow copy, pointer is copied in stead of value
    Jill.work_address->suite = 456;

    std::cout << John << std::endl;
    std::cout << Jane << std::endl;
    std::cout << Jill << std::endl;
    EXPECT_EQ(456, John.work_address->suite);     // pointer is copied in stead of value
    EXPECT_EQ(456, Jane.work_address->suite);
    EXPECT_EQ(456, Jill.work_address->suite);

    delete addr;
}

TEST(prototype_pattern, simple_prototype_pattern) {
    Address* addr = new Address {"123 East Dr", "London"};  // prefilled address -> this is the prototype!
    Contact2 John {"John Doe", addr};               // Constructor does a DEEP copy, values are copied in stead of pointer
    Contact2 Jane {"jane Doe", addr};

    John.work_address->suite = 100;
    Jane.work_address->suite = 123;

    EXPECT_EQ(100, John.work_address->suite);
    EXPECT_EQ(123, Jane.work_address->suite);

    Contact2 Jill {Jane};                            // Copy does a DEEP copy, values are copied in stead of pointer
    Jill.work_address->suite = 456;
    EXPECT_EQ(100, John.work_address->suite);
    EXPECT_EQ(123, Jane.work_address->suite);
    EXPECT_EQ(456, Jill.work_address->suite);

    delete addr;
}

TEST(prototype_pattern, prototype_of_employee) {
    // Prototype Pattern
    // Here we create a prototype Employee that prefills the Contact Objects
    Contact2 employee = {"", new Address {"123 East Dr", "London", 0}};

    // new employees can use the prototype to prefill the object attributes,
    // using the deep copy constructor that Contact2 privides.
    Contact2 John {employee};
    John.name = "John";
    John.work_address->suite = 100;
    Contact2 Jane {employee};
    Jane.name = "Jane";
    Jane.work_address->suite = 123;

    EXPECT_EQ("John", John.name);
    EXPECT_EQ(100, John.work_address->suite);
    EXPECT_EQ("London", John.work_address->city);
    EXPECT_EQ("123 East Dr", John.work_address->street);

    EXPECT_EQ("Jane", Jane.name);
    EXPECT_EQ(123, Jane.work_address->suite);
    EXPECT_EQ("London", Jane.work_address->city);
    EXPECT_EQ("123 East Dr", Jane.work_address->street);
}

// who owns the prototype employee? here we use a Prototype Factory that own it.
struct EmployeeFactory {
    static Contact2 main, aux;

    static std::unique_ptr<Contact2> NewMainOfficeEmployee(std::string name, int suite) {
        return NewEmployee(name, suite, main);
    }
    static std::unique_ptr<Contact2> NewAuxOfficeEmployee(std::string name, int suite) {
        return NewEmployee(name, suite, aux);
    }
private:
    static std::unique_ptr<Contact2> NewEmployee (std::string name, int suite, Contact2& proto) {
        auto result = std::make_unique<Contact2>(proto);
        result->name = name;
        result->work_address->suite = suite;
        return result;
    }
};
Contact2 EmployeeFactory::main {"", new Address {"123 East Dr", "London", 0}};  // static variables initialization is done outside of the class
Contact2 EmployeeFactory::aux {"", new Address {"123B East Dr", "London", 0}};

TEST(prototype_pattern, prototype_factory) {
    // Prototype Pattern
    // Here we create a prototype Employee that prefills the Contact Objects
    Contact2 employee = {"", new Address {"123 East Dr", "London", 0}};    // -> problem: who is responsible of creating and managing this???
    // solution: prototype factory

    auto John = EmployeeFactory::NewMainOfficeEmployee("John", 123);
    EXPECT_EQ("John", John->name);
    EXPECT_EQ(123, John->work_address->suite);
    EXPECT_EQ("London", John->work_address->city);
    EXPECT_EQ("123 East Dr", John->work_address->street);

    auto Jane = EmployeeFactory::NewAuxOfficeEmployee("Jane", 234);
    EXPECT_EQ("Jane", Jane->name);
    EXPECT_EQ(234, Jane->work_address->suite);
    EXPECT_EQ("London", Jane->work_address->city);
    EXPECT_EQ("123B East Dr", Jane->work_address->street);
}

#include <string>
#include <iostream>
#include <memory>
#include <functional>
#include <sstream>
using namespace std;
#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

struct Address2 {
    std::string street;
    std::string city;
    int suite;

    friend std::ostream& operator<<(std::ostream& os, const Address2& obj) {
        return os
                << "street: " << obj.street
                << " city: " << obj.city
                << " suite: " << obj.suite;
    }

private:
    friend class boost::serialization::access;

    template<class Ar>
    void serialize(Ar& ar, const unsigned int version) {
        ar & street;
        ar & city;
        ar & suite;
    }
};

struct Contact3 {
    std::string name;
    Address2 *address;

    friend std::ostream& operator<<(std::ostream& os, const Contact3& obj) {
        return os
                << "name: " << obj.name
                << " work_address: " << obj.address;
    }

private:
    friend class boost::serialization::access;

    template <class Ar>
    void serialize(Ar& ar, const unsigned int version) {
        ar & name;
        ar & address;
    }
};

// The principle issue of the prototype pattern is you have to perform deep copy of objects.
// How can we do this in a uniform fashion.
// How do we provide a uniform interface for cloning your different objects
// - without the need for deep copy
// - using serialize and deserialize with boost archive
TEST(prototype_pattern, boost_serialization) {

    Contact3 john;
    john.name = "John Doe";
    john.address = new Address2{"123 East Dr", "London", 123};

    // define a function that performs a deep copy
    auto clone = [](Contact3 c) {
        // serialization support
        std::ostringstream oss;
        boost::archive::text_oarchive oa(oss);  // this is whats going to write the object to the output stream
        oa << c;                                // here we serializing the contact into a textual format into a output string stream

        std::string s = oss.str();
        std::cout << s << std::endl;

        // deserialization support
        Contact3 result;
        std::istringstream iss(s);
        boost::archive::text_iarchive ia(iss);
        ia >> result;

        // this results in a deep copy!!!
        return result;
    };

    Contact3 jane = clone(john);
    jane.name = "Jane";
    jane.address->street = "123B West Dr";

    std::cout << john << std::endl << jane << std::endl;
    EXPECT_EQ("Jane", jane.name);                       // modified
    EXPECT_EQ("123B West Dr", jane.address->street);
    EXPECT_EQ(123, jane.address->suite);                // for prototyp
    EXPECT_EQ("London", jane.address->city);
}
