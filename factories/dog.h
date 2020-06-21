#pragma once

#include "animal.h"

class dog : public animal {
public:
    std::string walk() override;

    std::string talk() override;
};

struct dogFactory : animalItemFactory {
    std::unique_ptr<animal> make() override {
        return std::make_unique<dog>();
    }
};
