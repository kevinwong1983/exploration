#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>
#include <queue>

// track individual ducks quacking realtime by observer.
class QuackObservable; //forwared declare
class Observer {
public:
    virtual ~Observer() = default;
    virtual void update(std::shared_ptr<QuackObservable> duck) = 0;
};

class Quackologist : public Observer {
public:
    void update(std::shared_ptr<QuackObservable> duck) {
        std::cout << "Quackologist: " << duck.get() << " just quacked" << std::endl;
    };
};

class QuackObservable : public std::enable_shared_from_this<QuackObservable> {
public:
    virtual ~QuackObservable() = default;
    virtual void registerObserverable(std::shared_ptr<Observer> Observer) = 0;
    virtual void notifyObservers() = 0;
};

class Observable : public QuackObservable {
public:
    Observable(std::shared_ptr<QuackObservable> duck) : duck_{duck} {
    }
    void registerObserverable(std::shared_ptr<Observer> observer) override {
        observers_.push_back(observer);
    }
    void notifyObservers() override {
        for (auto& observer: observers_) {
            observer->update(duck_);
        }
    }
private:
    std::shared_ptr<QuackObservable> duck_;
    std::vector<std::shared_ptr<Observer>> observers_;
};

// 1. Create a Quackable Interface
class Quackable : public QuackObservable {
public:
    virtual ~Quackable() = default; // "Guideline #4: A base class destructor should be either public and virtual, or protected and nonvirtual."
    virtual void quack() = 0;
};

// 2. Implement some ducks
class MallardDuck : public Quackable {
public:
    MallardDuck() {
    };


    void quack() override {
        if (!observable_) {
            observable_ = std::make_shared<Observable>(std::dynamic_pointer_cast<MallardDuck>(shared_from_this()));
        }

        std::cout << "quack" << std::endl;
        notifyObservers();
    };

    void registerObserverable(std::shared_ptr<Observer> observer) override {
        observable_->registerObserverable(observer);
    };
    void notifyObservers() override {
        observable_->notifyObservers();
    };
private:
    std::shared_ptr<Observable> observable_;
};

class RedheadDuck : public Quackable {
public:
    RedheadDuck() {};
    void quack() override {
        observable_ = std::make_shared<Observable>(std::dynamic_pointer_cast<RedheadDuck>(shared_from_this()));
        std::cout << "quack" << std::endl;
        notifyObservers();
    };
    void registerObserverable(std::shared_ptr<Observer> observer) override {
        observable_->registerObserverable(observer);
    };
    void notifyObservers() override {
        observable_->notifyObservers();
    };
private:
    std::shared_ptr<Observable> observable_;
};

class DuckCall : public Quackable {
public:
    DuckCall() {};
    void quack() override {
        observable_ = std::make_shared<Observable>(std::dynamic_pointer_cast<DuckCall>(shared_from_this()));
        std::cout << "Kwak" << std::endl;
        notifyObservers();
    }
    void registerObserverable(std::shared_ptr<Observer> observer) override {
        observable_->registerObserverable(observer);
    };
    void notifyObservers() override {
        observable_->notifyObservers();
    };
private:
    std::shared_ptr<Observable> observable_;
};

class RubberDuck : public Quackable {
public:
    RubberDuck() {};
    void quack() override {
        observable_ = std::make_shared<Observable>(std::dynamic_pointer_cast<DuckCall>(shared_from_this()));
        std::cout << "Squeak" << std::endl;
        notifyObservers();
    }
    void registerObserverable(std::shared_ptr<Observer> observer) override {
        observable_->registerObserverable(observer);
    };
    void notifyObservers() override {
        observable_->notifyObservers();
    };
private:
    std::shared_ptr<Observable> observable_;
};

// 4. when ducks are around, gees can't be far
class Goose {
public:
    void honk() {
        std::cout << "Honk" << std::endl;
    }
};

//// 5. we need a adapter
class GooseAdapter : public Quackable {
public:
    GooseAdapter(std::unique_ptr<Goose> goose) :
        goose_{std::move(goose)} {};

    void quack() override {
        observable_ = std::make_shared<Observable>(std::dynamic_pointer_cast<GooseAdapter>(shared_from_this()));
        goose_->honk();
        notifyObservers();
    }
    void registerObserverable(std::shared_ptr<Observer> observer) override {
        observable_->registerObserverable(observer);
    };
    void notifyObservers() override {
        observable_->notifyObservers();
    };
private:
    std::shared_ptr<Observable> observable_;
    std::unique_ptr<Goose> goose_;
};

// Add a quack count behavior
class QuackCount : public Quackable {
public:

    QuackCount(std::shared_ptr<Quackable> duck) :
            duck_{std::move(duck)} {};

    void quack() override {
        count_ ++;
        duck_->quack();
    }

    static int getQuacks() {
        return count_;
    }

    static int count_;

    void registerObserverable(std::shared_ptr<Observer> observer) override {
        duck_->registerObserverable(observer);
    };
    void notifyObservers() override {
        duck_->notifyObservers();
    };

private:
    std::shared_ptr<Quackable> duck_;
};

int QuackCount::count_ = 0; // Initialize static member of class QuackCount!!

// Add abstract factory
class AbstractDuckFactory {
public:
    virtual ~AbstractDuckFactory() = default;
    virtual std::unique_ptr<Quackable> createMallardDuck() = 0;
    virtual std::unique_ptr<Quackable> createRedheadDuck() = 0;
    virtual std::unique_ptr<Quackable> createCallDuck() = 0;
    virtual std::unique_ptr<Quackable> createRubberDuck() = 0;
};

class DuckFactory : public AbstractDuckFactory {
public:
    std::unique_ptr<Quackable> createMallardDuck() override {
        return std::make_unique<MallardDuck>();
    };
    std::unique_ptr<Quackable> createRedheadDuck() override {
        return std::make_unique<RedheadDuck>();
    };
    std::unique_ptr<Quackable> createCallDuck() override {
        return std::make_unique<DuckCall>();
    };
    std::unique_ptr<Quackable> createRubberDuck() override {
        return std::make_unique<RubberDuck>();
    };
};

class CountingDuckFactory : public AbstractDuckFactory {
public:
    std::unique_ptr<Quackable> createMallardDuck() override {
        return std::make_unique<QuackCount>(std::make_unique<MallardDuck>());
    };
    std::unique_ptr<Quackable> createRedheadDuck() override {
        return std::make_unique<QuackCount>(std::make_unique<RedheadDuck>());
    };
    std::unique_ptr<Quackable> createCallDuck() override {
        return std::make_unique<QuackCount>(std::make_unique<DuckCall>());
    };
    std::unique_ptr<Quackable> createRubberDuck() override {
        return std::make_unique<QuackCount>(std::make_unique<RubberDuck>());
    };
};

class Flock : public Quackable {
public:
    void quack() override {
        for (auto& quacker : quackers_) {
            quacker->quack();
        }
    };
    void add(std::shared_ptr<Quackable> quacker) {
        quackers_.push_back(quacker);
    }
    void registerObserverable(std::shared_ptr<Observer> observer) override {
        for (auto& quacker : quackers_) {
            quacker->registerObserverable(observer);
        }
    };
    void notifyObservers() override {
        for (auto& quacker : quackers_) {
            quacker->notifyObservers();
        }
    };
private:
    std::vector<std::shared_ptr<Quackable>> quackers_;
};

// 3. We need a simulator
class DuckSimulator {
public:
    void simulate(std::unique_ptr<AbstractDuckFactory> duckFactory) {
        auto flockOfDucks = std::make_unique<Flock>();
        std::shared_ptr<Quackable> mallard = duckFactory->createMallardDuck();
        std::shared_ptr<Quackable> redhead = duckFactory->createRedheadDuck();
        std::shared_ptr<Quackable> call = duckFactory->createCallDuck();
        std::shared_ptr<Quackable> rubber = duckFactory->createRubberDuck();
        std::shared_ptr<Quackable> goose = std::make_unique<GooseAdapter>(std::move(std::make_unique<Goose>()));
        flockOfDucks->add(mallard);
        flockOfDucks->add(redhead);
        flockOfDucks->add(call);
        flockOfDucks->add(rubber);
        flockOfDucks->add(goose);


        auto flockOfMallards = std::make_unique<Flock>();
        std::shared_ptr<Quackable> mallard1 = duckFactory->createMallardDuck();
        std::shared_ptr<Quackable> mallard2 = duckFactory->createMallardDuck();
        std::shared_ptr<Quackable> mallard3 = duckFactory->createMallardDuck();
        std::shared_ptr<Quackable> mallard4 = duckFactory->createMallardDuck();
        std::shared_ptr<Quackable> mallard5 = duckFactory->createMallardDuck();

        auto quackoligist = std::make_shared<Quackologist>();
        mallard1->registerObserverable(quackoligist);
        simulate(mallard1);

        flockOfMallards->add(std::move(mallard1));
        flockOfMallards->add(std::move(mallard2));
        flockOfMallards->add(std::move(mallard3));
        flockOfMallards->add(std::move(mallard4));
        flockOfMallards->add(std::move(mallard5));

        simulate(std::move(flockOfDucks));
        simulate(std::move(flockOfMallards));
        std::cout << "The ducks quacked " << QuackCount::getQuacks() << " times" << std::endl;
    }

    void simulate(std::shared_ptr<Quackable> duck) {
        duck->quack();
    }
};

TEST(compound_patterns, runner) {
    auto duckFactory = std::make_unique<CountingDuckFactory>();
    auto simulator = DuckSimulator();
    simulator.simulate(std::move(duckFactory));
}