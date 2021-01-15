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
#include <boost/bimap.hpp>

// Flyweigth:
// A space optimization technique that let us use less memory by storing externally the data associated with similar objects

// Motivation:
// - avoid redundancy when storing data

typedef uint32_t key;

struct User {
    // std::string first_name, last_name;
    User(const std::string& first_name, const std::string& last_name):
        first_name{add(first_name)}, last_name{add(last_name)} {
    }
    const std::string& get_first_name() const {
        return names.left.find(first_name)->second;
    }
    const std::string& get_last_name() const {
        return names.left.find(last_name)->second;
    }
    friend std::ostream& operator << (std::ostream& os, const User& obj) {
        return os
            << "first name: " << obj.first_name << " " << obj.get_first_name()
            << " last name: " << obj.last_name << " " << obj.get_last_name();
    }

protected:
    static int seed;
    static boost::bimap<key, std::string> names;
    static key add(const std::string& s){ // the actual flyweight = the key that indexes the names of the bimap, thereby saving us memory!
        auto it = names.right.find(s);
        if (it == names.right.end()){
            key id = ++ seed;
            names.insert(boost::bimap<key, std::string>::value_type(seed, s));
            return id;
        }
        return it->second;
    }
    key first_name;
    key last_name;
};
int User::seed = 0;
boost::bimap<key, std::string> User::names{};

TEST(flyweight, simple_flyweigth) {
    User john_doe {"John", "Doe"};
    User jane_doe {"Jane", "Doe"};
    User jane_flip {"Jane", "Flip"};

    // the actual flyweight itself is the key that indexes the bimap with stored names, so it saves us memory for all the
    // users that are called John, Jane, Doe...
    std::cout << "John - " << john_doe << std::endl;
    std::cout << "Jane - " << jane_doe << std::endl;
    std::cout << "Jane - " << jane_flip << std::endl;

    EXPECT_EQ(john_doe.get_last_name(), jane_doe.get_last_name());
    EXPECT_EQ(jane_flip.get_first_name(), jane_doe.get_first_name());
}

// use boost flyweight library
#include <boost/flyweight.hpp>

struct User2 {
    boost::flyweight<std::string> first_name; // change these into flyweigh objects
    boost::flyweight<std::string> last_name;

    User2(const std::string& first_name, const std::string& last_name):
            first_name{first_name}, last_name{last_name} {
    }
    friend std::ostream& operator << (std::ostream& os, const User2& obj) {
        return os
                << "first name: " << obj.first_name
                << " last name: " << obj.last_name;
    }
};

TEST(flyweight, boost_flyweigth) {
    User2 john_doe {"John", "Doe"};
    User2 jane_doe {"Jane", "Doe"};
    User2 jane_flip {"Jane", "Flip"};

    // the actual flyweight itself is the key that indexes the bimap with stored names, so it saves us memory for all the
    // users that are called John, Jane, Doe...
    std::cout << "John - " << john_doe << std::endl;
    std::cout << "Jane - " << jane_doe << std::endl;
    std::cout << "Jane - " << jane_flip << std::endl;

    // compare the pointers of the strings to see if the flyweight works
    EXPECT_EQ(john_doe.last_name.get(), jane_doe.last_name.get());
    EXPECT_EQ(jane_flip.first_name.get(), jane_doe.first_name.get());
}

// Summary:
// Store common data externally or use a Flyweight library type






