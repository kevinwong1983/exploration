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

//////////////////////////////////////////////////////////////////////////////////////////
//      ISP - Interface Segregation Principle
//      Many client-specific interfaces better than one general-purpose interface.
// -> Name of Interfaces have more to do with its the classes that use them, than with the classes that implements them.
// -> Fat classes problems:
// -> -> classes that have lots of methods and used by many other classes with each there own use cases.
// -> -> has lots of fan-in, which results in a major bottle-neck.
// -> -> coupling involved in fat classes can be so severe -> make it impossible to take the clients and put them in
//       separate components (dll, jar) and independently deploy them.
// -> When faces with a fat class, we isolate the class with its clients, by creating interfaces that those clients can use.
// -> -> since interfaces are more logically coupled to the client that calls them than to the classes that implements
//       them, we make sure that those interfaces contain only the method that the clients wishes to call.
// -> -> then we multiple inherent those interfaces into the original fat class.
// -> -> changes made in function signatures in certain clients, will not cause the other clients to change. This
//       prevent them to be re-compiled and redeployed.
// -> -> that means we can put those clients in separate components (dll, jar) and deploy them separately.
// -> Fat classes creates a weird backwards coupling that comes from to much knowledge, changing one method in its
//    interfaces needs recompile and redeployment of all its client, that uses that interface.
// -> -> goal if ISP is to prevent that backwards coupling, by insuring you don’t depend on methods that you do not call.
// -> -> generalised: don’t depend on things that you do not need. Otherwise you will create coupling that will make
//       your software rigid and fragile.
// -> Detect ISP violation: don’t depend on thing that you do not need.
// -> -> create an instance of an object and pass in constructor arguments that you have no use for? You are passing in
//       arguments that you do not need, simply to satisfy someone else his needs.
// -> -> build up a complex data structure to run a simple test.
// -> -> fire up a web server and connect to a database just to test a simple business rule.
// -> -> write a test that walks though the logging process simply to test a business rule.
// -> -> need to call a function, but before you can, you need to call two more function, that you do not know or case
//       what they did.
// -> Don’t force your "users" to depend on thing s they don’t need, whether your "users" are modules, people or tests.

struct Document;

// we create multifucntion device
// from the ISP we agree to use abscractions rather than concrete implementations
class IMachine2 {
public:
    virtual void print(std::vector<Document*> docs) = 0;
    virtual void scan(std::vector<Document*> docs) = 0;
    virtual void fax(std::vector<Document*> docs) = 0;
};

class MutifunctionalPheriperal : public IMachine2 {
public:
    void print(std::vector<Document*> docs) override {
        std::cout << "print something" << std::endl;
    };
    void scan(std::vector<Document*> docs) override {
        std::cout << "scan something" << std::endl;
    };
    void fax(std::vector<Document*> docs) override {
        std::cout << "fax something" << std::endl;
    };
    // according to ISP this is a bad idea..
    // 1. everytime you change just a part of the functionality e.g. print mechanism, without
    //      touching scan and fax mechanism, you would have to recompile because its part of
    //      a single file.
    // 2. it could be that the user just want device to print, and does not care about scan and fax
    //      or does not know how to implement scan or fax yet. Your are forcing the implementer to
    //      implement too much!

    // No client should be forced to depend on methods it does not use.
    // ISP is all about breaking up these monolithic monstrocities and doing it in a piece-wise fashion.
};

// how do we do it better?
class Printable {
public:
    virtual void print(std::vector<Document*> docs) = 0;
};
class Scannable {
public:
    virtual void scan(std::vector<Document*> docs) = 0;
};
class Faxable {
public:
    virtual void fax(std::vector<Document*> docs) = 0;
};
class IMachine : public Printable, public Scannable {};

class Printer : public Printable {
public:
    void print (std::vector<Document*> docs) override {
        std::cout << "print something" << std::endl;
    }
};
class Scanner : public Scannable {
public:
    void scan (std::vector<Document*> docs) override {
        std::cout << "scan something" << std::endl;
    }
};
class Faxer : public Faxable {
public:
    void fax (std::vector<Document*> docs) override {
        std::cout << "fax something" << std::endl;
    }
};

class MultifunctionalOfficeMachine : public IMachine {
public:
    MultifunctionalOfficeMachine(Printable &printer,
            Scannable scanner)
            : printer_{ printer }, scanner_{ scanner } {
    }
    void print (std::vector<Document*> docs) override {
        printer_.print(docs);
    }
    void scan (std::vector<Document*> docs) override {
        scanner_.scan(docs);
    }
private:
    Printable& printer_;
    Scannable& scanner_;
};
