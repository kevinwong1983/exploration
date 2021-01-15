//
// Created by user on 09/06/2020.
//

#include "dog.h"

std::string dog::talk() {
    std::string s = "woof";
    std::cout << s << std::endl;
    return s;
}

std::string dog::walk() {
    std::string s = "with 4 legs";
    std::cout << s << std::endl;
    return s;
}