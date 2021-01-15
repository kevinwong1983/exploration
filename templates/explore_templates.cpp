#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>
#include "animal_house.h"

using namespace houses;

TEST(dummy, template) {
    EXPECT_EQ(true, true);
}

TEST(dummy, animal_house) {
    DogHouse dogHouse;
    dog a;
    dogHouse.Enter(a);
}