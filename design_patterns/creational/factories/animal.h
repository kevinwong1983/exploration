#pragma once
#include "iostream"

class animal {
public:
    virtual ~animal() = default;
    virtual std::string walk() = 0;
    virtual std::string talk() = 0;
};

struct animalItemFactory {
    virtual ~animalItemFactory() = default;
    virtual std::unique_ptr<animal> make() = 0;
};
