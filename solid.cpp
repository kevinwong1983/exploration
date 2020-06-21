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

// SRP - Single Resonsibility Principle
//- We gather the things that are going to change for the same reason, and we separate the things that are going to change for different reasons
//- When we keep our responsibility separate, we can change them with minimum impact.
//- A responsibility is a family of function that serves a specific actor
//- Classes have responsibilities to their users. These users can be classified as actors by the role that they play.
//- A responsibility is a family of functions that serves one particular actor
//- There are two values of software
//-- primary value: Keep it flexible so it can continue to meet its requirements throughout its lifetime
//-- Secondary value: software should serve the users need.
//- Carefully allocating responsibility to classes and modules is one of the ways to keep the primary value of software high
//- When modules have more than one responsibility, the system become fragile due to unintended interactions between the responsibilities
//- Separate ways to solve violations of the SRP
//-- Sepparations
//-- Facades
//-- Interface segragations
//- Non of the ways are perfect: welcome to software engineering

#include <fstream>

struct Journal {
    std::string title_;
    std::vector<std::string> entries_;

    explicit Journal(const std::string& title): title_(title) {
    }

    void Add(const std::string& entry) {            // adding entry is responsibility of Journal
        entries_.emplace_back(entry);
    }

//    void save(const std::string& file_name) {     // saving shoud not be responsiblity of Journal
//        std::ofstream ofs(file_name);             // if someone want to change hoe we save (e.g. from files to cloud
//        for(auto& s : entries_) {                 // he has to change Journal class? this is violation of SRP
//            ofs << s <<std::endl;
//        }
//    }
};

// By creating a sepparate class with only the responsiblity for persistence
// we conform the the SRP
struct persistenceManager {
    void save(const std::string& file_name, const Journal& j) {
        std::ofstream ofs(file_name);
        for(auto& s : j.entries_) {
            ofs << s <<std::endl;
        }
    }
};


