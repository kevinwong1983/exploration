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

// Prototype: A partially or fully initialized object that you copy (clone) and make use of.
// Complicated objects (e.g. cars) aren't designed from scratch: They reiterate existing designs.
// An existing (partially constructed design) is a Prototype.
// We make a copy (clone) the prototype and customize it.
//      - requires "deep copy" support
//      - Painful without metadata!
// We make the cloning conveneit (e.g. via a Factory)

// To implement a prototype:
// 1. partially construct an object and store it somewhere.
// 2. clone that prototype and than customize the instance.
// 3. ensure deep copying!

using namespace std;

struct Address {
    string street;
    string city;
    int suite;

    //  Streaming
    //  The stream operators:
    //  - operator << output
    //  - operator >> input
    //  When you use these as stream operators (rather than binary shift)
    //  the first parameter is a stream. Since you do not have access to
    //  the stream object (its not yours to modify) these cannot be member
    //  operators they have to be external to the class. Thus they must
    //  either be friends of the class or have access to a public method
    //  that will do the streaming for you.
    //
    //  "friend" s a way to extended the public interface without breaking encapsulation
    //
    //   It is also traditional for these objects to return a reference
    //   to a stream object so you can chain stream operations together.
    friend ostream& operator<< (ostream& os, const Address& obj){
        return os << " street: " << obj.street
            << " city: " << obj.city
            << " suite: " << obj.suite;
    }
};

struct Contact {
    string name;
    Address* work_address;

    Contact(const string& name, Address* const work_address)
            : name(name),
              work_address(new Address(*work_address)) {        // deep copy
    }

    ~Contact() {
        delete work_address;
    }

    Contact(const Contact& other)
            : name(other.name),
              work_address(new Address(*other.work_address)) {      // deep copy
    } // rule of 5: when you define a copy constructor, you also need to create a constructor

    friend ostream& operator<<(ostream& os, const Contact& obj) {
        return os
                << "name: " << obj.name
                << " work_address: " << *obj.work_address;
    }
};

TEST(prototype, deep_copying_vs_shallow_copying) {
    Contact john {"John Doe", new Address{"123 East Dr", "London"}};
    EXPECT_EQ(john.name, "John Doe");
    EXPECT_EQ( john.work_address->street, "123 East Dr");
    EXPECT_EQ( john.work_address->city, "London");

    auto address = new Address{"123 East Dr", "London"};    // -> this is the prototype
    // we want to keep the address, but customize the suite.

    Contact john_1 {"John Doe", address};
    john_1.work_address->suite = 100;
    cout << "john 1" << john_1 << endl;

    Contact jane_1 {"Jane Doe", address};
    jane_1.work_address->suite = 123;
    cout << "jane 1" << jane_1 << endl;
    cout << "john 1" << john_1 << endl; // also changed for John

    // we want a deep copy of the prototype instead of sharing a pointer.

    // by default we have shallow copy constructors
    // we need to create our own deep copy constructor
    Contact jill{jane_1};
    EXPECT_EQ(jill.name, "Jane Doe");
    EXPECT_EQ( jill.work_address->street, "123 East Dr");
    EXPECT_EQ( jill.work_address->city, "London");
    EXPECT_EQ( jill.work_address->suite, 123);
    jill.work_address->suite = 234;

    EXPECT_EQ(jill.work_address->suite, 234);
    EXPECT_EQ(jane_1.work_address->suite, 123);
    EXPECT_EQ(john_1.work_address->suite, 100);

    delete address;
}

TEST(prototype, simple_prototype) {
    // this is a employee prototype, which already is partially constructed.
    Contact employee { "", new Address{"123 East Dr", "London", 0}};    // this has actually a memory leak

    Contact john {employee};
    john.name = "John";
    john.work_address->suite = 100;

    Contact jane {employee};
    jane.name = "Jane";
    jane.work_address->suite = 123;

    EXPECT_EQ(jane.work_address->suite, 123);
    EXPECT_EQ(john.work_address->suite, 100);
}

struct EmployeeFactory {
public:
    static Contact main, aux; // these are the address prototypes

    // whenever someone wants a new main office employee, it just clones the static employee prototype.
    // we clone the prototype by passing the prototype into the copy constructor.
    static unique_ptr<Contact> NewMainOfficeEmployee(string name, int suite) {
        return NewEmployee(name, suite, main);
    }

    static unique_ptr<Contact> NewAuxOfficeEmployee(string name, int suite) {
        return NewEmployee(name, suite, aux);
    }
private:
    static unique_ptr<Contact> NewEmployee (string name, int suite, Contact& proto) {
        auto result = make_unique<Contact>(proto);
        result->name = name;
        result->work_address->suite = suite;
        return result;
    }
};

// these are the contact prototypes
// static needs to be declared outside class
Contact EmployeeFactory::main{"", new Address{"123 EsatDr", "London", 0}};
Contact EmployeeFactory::aux{"", new Address{"123B EsatDr", "London", 0}};


TEST(prototype, factory_prototype) {
    // factory returning new Contact using prototypes.
    auto john = EmployeeFactory::NewMainOfficeEmployee("John", 100);
    auto jane = EmployeeFactory::NewAuxOfficeEmployee("Jane", 123);

    EXPECT_EQ(jane->work_address->suite, 123);
    EXPECT_EQ(john->work_address->suite, 100);

    cout << "jane -" << *jane << endl;
    cout << "john -" << *john << endl;
}

///////////////////////

#include <string>
#include <iostream>
#include <memory>
#include <functional>
#include <sstream>

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
