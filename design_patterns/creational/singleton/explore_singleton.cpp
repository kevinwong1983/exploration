#include "boost/variant.hpp"
#include <iostream>
#include <fstream>
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
#include <MacTypes.h>

// Singleton: A component which is instantiated only once.

// Motivation:
// For some components (classes) it only makes sense to have one in the system
// e.g. Database, Object factory
// it is meant to guard resource in your system from multiple access points.
// e.g. the constructor call is expensive: we only do it once / we provide everyone with the same instance.

// challenge:
// We want to prevent anyone creating additional copies
// lazy instantiation and multitrheading

class SingletonDatabase {
    // 1. we do not want people to direcly instantiate:
    // e.g. SingletonDatabase db;           -> private constructors
    // 2. we dont want people copying
    // e.g. auto db2(db)                    ->
    // 3. we doe not want assignment
    // auto db3 = db;

private:
    SingletonDatabase(){
        // 1. private constructor.
        std::cout << "initializing database" << std::endl;

        std::ifstream ifs("/Users/user/CLionProjects/experimental/design_patterns/creational/singleton/capitals.txt");

        std::string s, s2;
        while (std::getline(ifs, s)) {
            getline(ifs, s2);
            int pop = boost::lexical_cast<int>(s2);
            capitals[s] = pop;
        }

        instance_count++;
    }
    std::map<std::string, int> capitals;

    static SingletonDatabase* instance;

public:
    static int instance_count;
    // 2. delete the copy constructor
    SingletonDatabase(SingletonDatabase const&) = delete;
    // 3. delete the copy assignment
    void operator=(SingletonDatabase const&) = delete;

//    // this is the naive approach! It has some issues:
//    // 1. not thread safe: you van have multiple thread doing this.
//    // 2. how do you delete this after instantiating it: there is no static destructor
//    static SingletonDatabase* get_instance() {
//        if (!instance) {                      // Unsafe Singleton
//            instance = new SingletonDatabase;
//        }
//        return instance;
//    }

    static SingletonDatabase& get() {
        // Safe Singleton: this invocation will only happen once!
        // So thread safe!
        static SingletonDatabase db;
        return db;
    }

    int get_population(const std::string& name) {
        return capitals[name];
    }
};

int SingletonDatabase::instance_count = 0;

TEST(singleton_pattern, naive_approach) {
    auto& db1 = SingletonDatabase::get();
    auto& db2 = SingletonDatabase::get();
    EXPECT_EQ(1, db1.instance_count);
    EXPECT_EQ(1, db2.instance_count);
}

struct SingletonRecordFinder {

    int total_population(std::vector<std::string> names) {
        int result = 0;
        for (auto & name: names) {

            // this is all find until we need to create a test for it..
            // very tightly coupling to the database: I want a unit-test not a integration test!
            result += SingletonDatabase::get().get_population(name); // we hardcoded the singleton into our class... NOT GOOD!
        }
        return result;
    }
};

TEST(singleton_pattern, testing_singleton) {

    // this works, however you have tight coupling between recordfinder class and the singleton database!
    // you can only create integration tests! not unit tests;
    SingletonRecordFinder rf;
    std::vector<std::string> names {"Seoul", "Mexico City"};
    int tp = rf.total_population(names);
    EXPECT_EQ(17500000 + 17400000, tp);
}

////////// what we want is test on a dummy database instead of the singleton //////////////

class Database {
public:
    virtual int get_population(const std::string& name) = 0;
};

class improved_SingletonDatabase : public Database {                // we implement Database interface!!
private:
    improved_SingletonDatabase(){                                    // 1. private constructor.
        std::cout << "initializing database" << std::endl;

        std::ifstream ifs("/Users/user/CLionProjects/experimental/design_patterns/creational/singleton/capitals.txt");

        std::string s, s2;
        while (std::getline(ifs, s)) {
            getline(ifs, s2);
            int pop = boost::lexical_cast<int>(s2);
            capitals[s] = pop;
        }

        instance_count++;
    }
    std::map<std::string, int> capitals;
    static SingletonDatabase* instance;

public:
    static int instance_count;
    improved_SingletonDatabase(improved_SingletonDatabase const&) = delete;   // 2. delete the copy constructor
    void operator=(improved_SingletonDatabase const&) = delete;      // 3. delete the copy constructor

    static improved_SingletonDatabase& get() {
        static improved_SingletonDatabase db;    // this invacation will only happen once! So thread safe!
        return db;
    }

    int get_population(const std::string& name) override {
        return capitals[name];
    }
};

class DummyDatabase : public Database {
    std::map<std::string, int> capitals;
public:
    DummyDatabase(){
        capitals["alpha"] = 1;
        capitals["beta"] = 2;
        capitals["gamma"] = 3;
    }
    int get_population(const std::string& name) override {
        return capitals[name];
    }
};

struct ConfigurableRecordFinder {

    // using dependeny injecton we can feed in a dummy database instead of the singleton database
    explicit ConfigurableRecordFinder (Database& db) : db {db} {
    }

    int total_population(std::vector<std::string> names) {
        int result = 0;
        for (auto& name: names) {
            result += db.get_population(name);
        }
        return result;
    }

    Database& db;
};

TEST(singleton_pattern, dependency_injection) {
    DummyDatabase db{};
    ConfigurableRecordFinder rf{db};
    EXPECT_EQ(4, rf.total_population(std::vector<std::string> {"alpha", "gamma"}));
}