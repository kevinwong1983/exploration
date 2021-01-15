//
// Created by user on 10/01/2021.
//
#include "iostream"
#include "Person.h"

namespace mediator {

Person::Person(const std::string &name)
        : name(name) {
}

void Person::say(const std::string &message) const {
    room->broadcast(name, message);
}

void Person::receive(const std::string &origin, const std::string &message) {
    std::string s{origin + ": \"" + message + "\""};
    std::cout << "[" << name << "'s chat session" << s << std::endl;
    chat_log.emplace_back(message);
}

void Person::pm(const std::string& who, const std::string& message){
    room->message(name, who, message);
}

}