//
// Created by user on 24/06/2020.
//

#include "animal_house.h"
#include "creational_patterns/factories/ape.h"

using namespace houses;

template <class Animal>
void AnimalHouse<Animal>::Enter(Animal animal) {
    animal.walk();
    animal.talk();
}

// If you compile and (try to) link these two .cpp files, most compilers will generate linker errors.
// There are two solutions for this. The first solution is to physically move the definition of the
// template functions into the .h file, even if they are not inline functions. This solution may
// (or may not!) cause significant code bloat, meaning your executable size may increase dramatically
// (or, if your compiler is smart enough, may not; try it and see).

template class houses::AnimalHouse<dog>; // https://isocpp.org/wiki/faq/templates#separate-template-class-defn-from-decl
template class houses::AnimalHouse<ape>;