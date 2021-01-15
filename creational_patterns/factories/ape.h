#pragma once

#include "animal.h"

class ape : public animal {
public:
    std::string walk() override;

    std::string talk() override;
};

struct apeFactory : animalItemFactory {
    std::unique_ptr<animal> make() override {
        return std::make_unique<ape>();
    }
};
