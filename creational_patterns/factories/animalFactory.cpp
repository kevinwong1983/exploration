//
// Created by user on 09/06/2020.
//

#include "animalFactory.h"
#include "ape.h"
#include "dog.h"

static std::map<std::string, std::unique_ptr<animalItemFactory>> init() {
    std::map<std::string, std::unique_ptr<animalItemFactory>> mp;
    mp[animalFactory::kDog] = std::make_unique<dogFactory>();
    mp["ape"] = std::make_unique<apeFactory>();
    return mp;
}

std::map<std::string, std::unique_ptr<animalItemFactory>> animalFactory::factories_ = init();


