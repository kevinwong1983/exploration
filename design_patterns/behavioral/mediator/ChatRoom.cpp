//
// Created by user on 10/01/2021.
//

#include "ChatRoom.h"

namespace mediator {

void ChatRoom::broadcast(const std::string& origin, const std::string& message) {
    for (auto&p : people) {
        if (p.name != origin) {
            p.receive(origin, message);
        }
    }
}

Person* ChatRoom::PersonReference::operator->() const {
    return &people[index]; // returns a reference to a person.
}

ChatRoom::PersonReference ChatRoom::join(Person &&p){ // rvalue
    std::string join_msg = p.name + " joins the chat";
    broadcast("room", join_msg);

    p.room = this;
    people.emplace_back(p); // does a move
    return {people, (unsigned int) people.size() - 1}; // gives back a reference to vector people[people.size()-1)'
}

void ChatRoom::message(const std::string& origin, const std::string& who, const std::string& message){
     auto target = find_if(begin(people), end(people), [&](const Person& p){
         return p.name == who;
     });

     if (target!=end(people)){
         target->receive(origin, message);
     }
}

}// namespace mediator