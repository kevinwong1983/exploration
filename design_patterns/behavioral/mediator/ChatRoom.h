#pragma once

namespace mediator {
struct ChatRoom;
} // namespace mediator

#include <vector>
#include <string>
#include "Person.h"
namespace mediator {

struct ChatRoom {
    std::vector<Person> people;

    class PersonReference {
        std::vector<Person> &people;
        unsigned int index;
    public:
        PersonReference(std::vector<Person> &persons, const unsigned index)
                : people(persons),
                  index(index) {}

        Person* operator->() const; // deref operator
    };

    void broadcast(const std::string &origin, const std::string &message);

    PersonReference join(Person &&p);

    void message(const std::string& origin, const std::string& who, const std::string& message);

};

}// namespace mediator