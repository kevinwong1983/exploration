#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>

#include "animalFactory.h"
#include "animalMock.h"
#include "dog.h"
#include "ape.h"

class zoo {
    zoo() : animal_(animalFactory::make("dog")) {
        animal_->talk();
        animal_->walk();
    }

    std::unique_ptr<animal> animal_;
};

using testing::Return;

TEST(time, factory_test) {
//    auto a = animalFactory::make("ape");
//    EXPECT_TRUE(dynamic_cast<ape *>(a.get()));
//    auto d = animalFactory::make("dog");
//    EXPECT_TRUE(dynamic_cast<dog *>(d.get()));
//
//    auto amf = std::make_unique<animalMockFactory>();
//
//    //
//    std::unique_ptr<animalMock> mock = std::make_unique<animalMock>();
//    EXPECT_CALL(*mock, talk())
//            .Times(1)
//            .WillOnce(Return("hey I talk like a mock"));
//
//    amf->setReturnMock(std::move(mock));
//    animalFactory::registers("mock", std::move(amf));
//
//    auto m = animalFactory::make("mock");
//    EXPECT_TRUE(dynamic_cast<animalMock *>(m.get()));
//    EXPECT_EQ("hey I talk like a mock", m->talk());
//
//    //
//    std::unique_ptr<animalMock> mock2 = std::make_unique<animalMock>();
//    EXPECT_CALL(*mock2, talk())
//            .Times(1)
//            .WillOnce(Return("hey I talk like a mock"));
//
//    amf->setReturnMock(std::move(mock2));
//    animalFactory::registers("dog", std::move(amf));    // dog factory is now overwritten with mock factory
//
//    auto n = animalFactory::make("dog");
//    EXPECT_TRUE(dynamic_cast<animalMock *>(n.get()));
//    EXPECT_EQ("hey I talk like a mock", n->talk());
}

TEST(time, factory_test2) {
    auto animal_mock_factory = std::make_unique<animalMockFactory>();
    std::unique_ptr<animalMock> mock = std::make_unique<animalMock>();
    EXPECT_CALL(*mock, talk())
            .WillOnce(Return("hey I talk like a mock"));
    EXPECT_CALL(*animal_mock_factory, make())
            .WillOnce(Return(testing::ByMove(std::move(mock))));

    animalFactory::registers("dog",
                             std::move(animal_mock_factory));    // dog factory is now overwritten with mock factory

    auto n = animalFactory::make("dog");
    EXPECT_TRUE(dynamic_cast<animalMock *>(n.get()));
    EXPECT_EQ("hey I talk like a mock", n->talk());
}

TEST(time, polymorphism) {
    // this will not work: When you need polymorphism, you need to use either pointers or references
    // And since references can only be bound once, you cannot really use them in standard containers.
    auto d = std::make_unique<dog>();
    auto a = std::make_unique<ape>();
    auto animals = std::vector<std::unique_ptr<animal>>{};
    animals.emplace_back(std::move(d));
    animals.emplace_back(std::move(a));
    for(auto&& animal: animals) {
        animal->talk();
        animal->walk();
    }
}