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
#include <boost/any.hpp>
#include <mutex>
#include <boost/signals2.hpp>

// Observer
// An observer is an object that wishes to be informed about events happening in the system, typically by providing
// a callback function to call when events occur. Then entity generating the event is sometimes called observerable.

// Motivation
// We need to be informed when certrain things happen e.g. Object property changes, Object oes something, Some external event occur.
// We need to listen to events and be notified when they occur.
// No built-in event functionality in c++.

namespace observer {

struct Person;  // forward declaration

struct PersonListener
{
    virtual ~PersonListener() = default;
    virtual void PersonChanged(Person& p,
            const std::string& property_name,
            const boost::any new_value) = 0;
};

// prevent concurency issues
static std::mutex mtx;

struct Person {
    explicit Person(const int& age)
        : age_{age}{}

    virtual int GetAge(){
        return age_;
    }
    virtual void SetAge(const int age){
        if (age_ == age){
            return;
        }

        // we need to find a delta in can vote.
        auto old_can_vote = GetCanVote();

        age_ = age;
        notify("age", age_);

        //
        auto new_can_vote = GetCanVote();
        if (old_can_vote != new_can_vote) {
            // this code van escalate in complexity very fast.
            notify("can_vote", new_can_vote);
        }
    }
    //problems can arise:
    bool GetCanVote() const {
        return age_ >= 16;
    }

    void subscribe(PersonListener* pl){
        std::lock_guard<std::mutex> guard{mtx}; //prevent concurency issues
        if (std::find(begin(listners), end(listners), pl) == end(listners)) {
            listners.push_back(pl);
        }
    }
    void unsubscribe(PersonListener* pl){
        std::lock_guard<std::mutex> guard{mtx}; //prevent concurency issues
        for (auto it = listners.begin(); it != listners.end(); it++) {
            if (*it == pl) {
                *it = nullptr;
            }
        }
    }

    void notify(const std::string& property_name, const boost::any new_value) {
        std::lock_guard<std::mutex> guard{mtx}; //prevent concurency issues
        for (const auto& listner: listners){
            if (listner) {
                listner->PersonChanged(*this, property_name, new_value);
            }
        }

        // erase the stuff that where unsubscribed and set to nullptr..
        listners.erase(remove(listners.begin(), listners.end(), nullptr), listners.end());
    }
private:
    int age_;   // observerable
    std::vector<PersonListener*> listners;
};

struct ConsoleListener : PersonListener { //outputs all changes to the command line.
    void PersonChanged(Person& p,
                       const std::string& property_name,
                       const boost::any new_value) {
        std::cout << "person's " << property_name << "has been changed to ";
        if (property_name == "age"){
            std::cout << boost::any_cast<int>(new_value);
        }
        else if (property_name == "can_vote"){
            std::cout << boost::any_cast<bool>(new_value);
        }
        std::cout << std::endl;
    }
};

// concurrency problem 2. re-entry
struct BadListner : PersonListener { //outputs all changes to the command line.
    void PersonChanged(Person& p,
                       const std::string& property_name,
                       const boost::any new_value) {
        p.unsubscribe(this); // re-entry! my lock was already taken. now I have a deadlock!!
    }
};

TEST(observer, observer) {
    Person p{14};
    ConsoleListener cl;
    p.subscribe(&cl);

    p.SetAge(15);
    p.SetAge(16);
}

TEST(observer, thread_safety) {
    Person p{14};
    ConsoleListener cl;
    // subscribe twice
    p.subscribe(&cl);
    p.subscribe(&cl);

    p.SetAge(15);
    p.SetAge(16);

    p.unsubscribe(&cl);

    p.SetAge(17);
}

TEST(observer, re_entry) {
    Person p{14};
    BadListner cl;
    p.subscribe(&cl);
    // p.SetAge(15); // this creates a deadlock due to reentry!!

    // there is no water tight solution to these kind of issues.
    // normally solved with a social contract for no re-entry.
}

template <typename T>
struct INotifyPropertyChanged {
    virtual ~INotifyPropertyChanged()  = default;
    boost::signals2::signal<void(T&, const std::string&)> PropertyChanged;
};

struct Person2 : INotifyPropertyChanged<Person2> {
    explicit Person2(const int& age)
            : age_{age}{}

    virtual int GetAge() const{
        return age_;
    }
    virtual void SetAge(const int age){
        if (age_ == age) {
            return;
        }

        age_ = age;
        PropertyChanged(*this, "age");
    }

private:
    int age_;   // observerable
};

TEST(observer, signal) {
    Person2 p{123};
    p.PropertyChanged.connect([](const Person2& person, const std::string& property_name){
        std::cout<< property_name << " has been changed to " << person.GetAge() << std::endl;
    });

    p.SetAge(4);
}

// summary:
// - implementation of observer is an intrusive approach: an observer must provide (un) subscribe functions
// and must have explicit notification code.
// - Special care must be taken to prevent issues in multithreaded scenario's
// -- use locks
// -- Re-entracy in listners is very difficult to deal with
// Libraries uch as boost.signals2 provide a usable implemenation of observer.


} // namespace observer
