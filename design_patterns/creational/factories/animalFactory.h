#pragma once

#include "map"
#include "animal.h"

class animalFactory {
public:
    static constexpr const auto kDog = "dog";
    static constexpr const auto kApe = "ape";

    static std::unique_ptr<animal> make(const std::string &name) {
        auto drink = factories_[name]->make();
        return drink;
    }

    static void registers(const std::string &name, std::unique_ptr<animalItemFactory> factory) {
        factories_[name] = std::move(factory);
    }

private:
    static std::map<std::string, std::unique_ptr<animalItemFactory>> factories_;
};
