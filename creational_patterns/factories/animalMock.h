#pragma once

#include <gmock/gmock.h>
#include "animal.h"

class animalMock : public animal {
public:
    MOCK_METHOD0(walk, std::string());
    MOCK_METHOD0(talk, std::string());
};

struct animalMockFactory : animalItemFactory {
    MOCK_METHOD0(make, std::unique_ptr<animal>());
};
