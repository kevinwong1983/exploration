#pragma once
#include "design_patterns/creational/factories/dog.h"

namespace houses {
    template <class Animal>
    class AnimalHouse {
    public:
        AnimalHouse() = default;
        ~AnimalHouse() = default;
        void Enter(Animal animal);
    };

    using DogHouse = AnimalHouse<dog>;
}


