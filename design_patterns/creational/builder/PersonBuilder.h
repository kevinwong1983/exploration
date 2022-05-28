//
// Created by user on 20/07/2020.
//
#pragma once

#include "Person.h"

class PersonAddressBuilder;
class PersonJobBuilder;

class PersonBuilder {
    Person p;
protected:
    Person& person; // only inheritence can access theses

    explicit PersonBuilder(Person& person) : person{person} {
    }

public:
    PersonBuilder() : person {p} {}

    operator Person()
    {
        return move(person);
    }

    PersonAddressBuilder lives();
    PersonJobBuilder works();
};