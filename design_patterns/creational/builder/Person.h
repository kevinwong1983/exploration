#pragma once

#include <string>
#include <iostream>

using namespace std;

class PersonBuilder;

class Person {
private:
    // address
    string street_address, post_code, city;

    // employmnet
    string company_name, position;
    int annual_income = 0;

    Person() {}  // constructor is hidden
public:
    // move constructors
    Person(Person &&other) : street_address(std::move(other.street_address)),
                             post_code(std::move(other.post_code)),
                             city(std::move(other.city)),
                             company_name(std::move(other.company_name)),
                             position(std::move(other.position)),
                             annual_income(other.annual_income) {
    }

    // move assingment operator
    Person &operator=(Person &&other) {
        if (this == &other) return *this;
        street_address = std::move(other.street_address);
        post_code = std::move(other.post_code);
        city = std::move(other.city);
        company_name = std::move(other.company_name);
        position = std::move(other.position);
        annual_income = other.annual_income;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream &os, const Person &obj) {
        return os << "street_address: " << obj.street_address
                  << " post_code: " << obj.post_code
                  << " city: " << obj.city
                  << " company_name: " << obj.company_name
                  << " position: " << obj.position
                  << " annual_income: " << obj.annual_income;
    }

    static PersonBuilder create();

    friend class PersonBuilder; // let your builders have access to privates
    friend class PersonAddressBuilder;
    friend class PersonJobBuilder;
};
