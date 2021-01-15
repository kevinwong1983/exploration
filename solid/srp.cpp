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
//      SRP - Single Responsibility Principle
//      A class should only have a single responsibility
// -> We gather the things that are going to change for the same reason, and we separate
//    the things that are going to change for different reasons.
// -> When we keep our responsibility separate, we can change them with minimum impact.
// -> Definition of a responsibility: a family of function that serves a specific actor.
// -> Classes have responsibilities to their users. These users can be classified as actors
//    by the role that they play.
// -> A responsibility is a family of functions that serves one particular actor.
// -> There are two values of software.
// -> -> primary value: Keep it flexible so it can continue to meet its requirements
//      throughout its lifetime.
// -> -> Secondary value: software should serve the users need.
// -> Carefully allocating responsibility to classes and modules is one of the ways to
//    keep the primary value of software high.
// -> When modules have more than one responsibility, the system become fragile due to unintended
//    interactions between the responsibilities.
// -> Separate ways to solve violations of the SRP.
// -> -> Separations
// -> -> Facades
// -> -> Interface segregation
// -> Non of the ways are perfect: welcome to software engineering

#include <fstream>

//class Todo {
//public:
//    Todo (const std::string& title):title_(title) {
//    }
//    void addTask (const std::string& task) {
//        tasks_.push_back(task);
//    }
//    void save(std::string file_name){
//        std::ofstream stream(file_name);
//        stream << title_ << std::endl;
//        for (auto& e: tasks_) {
//            stream << e << std::endl;
//        }
//    }
//private:
//    std::string title_;
//    std::vector<std::string> tasks_;
//};

class Todo {
public:
    Todo (const std::string& title):title_(title) {
    }
    void addTask (const std::string& task) {
        tasks_.push_back(task);
    }
    std::string getTitle() const {
        return title_;
    }
    std::vector<std::string> getTasks() const {
        return tasks_;
    }
private:
    std::string title_;
    std::vector<std::string> tasks_;
};

class PersistenceManager {
public:
    void save(const Todo& todo, const std::string& file_name){
        std::ofstream stream(file_name);
        stream << todo.getTitle() << std::endl;
        for (auto& e: todo.getTasks()) {
            stream << e << std::endl;
        }
    }
};

TEST(srp, SRP) {
    Todo td("wedding");
    td.addTask("order wedding cake");
    td.addTask("send out inviatations");
    td.addTask("plan rehersal");

    auto pm = PersistenceManager();
    pm.save(td, "wedding plan2.txt" );
}

struct Journal {
    std::string title_;
    std::vector<std::string> entries_;

    explicit Journal(const std::string& title):title_(title){
    }
    void add(const std::string& entry) {
        entries_.push_back(entry);
    }
    // it should not be the responsibility for Journal to save itself. should be handled by separate class.
    // -> class should only have one reason to change:
    // -> -> e.g. in case the persistence implementation changes fram file to database,
    // -> -> e.g. format in which the journal should be saved
    // Journal class should not be changed.
//    void save(const std::string& filename) {
//        std::ofstream ofs(filename);
//        for (auto& e: entries_) {
//            ofs << e << std::endl;
//        }
//    }
};

// add a persistence manager to take the "save" responsibility
//struct PersistenceManager {
//    void save(const Journal& j, const std::string& filename) {
//        std::ofstream ofs(filename);
//        for (auto& e: j.entries_) {
//            ofs << e << std::endl;
//        }
//    }
//};
