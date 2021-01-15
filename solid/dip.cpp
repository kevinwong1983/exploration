#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>
#include <vector>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

//////////////////////////////////////////////////////////////////////////////////////////
//      DIP - Dependency Inversion Principle
//      Dependencies should be abstract rather than concrete. In other words, dependencies on interfaces
//      and supertypes is better than dependencies on concrete types.
//
// -> What dependencies there are, what there cost are and how to arrange them into a good architecture
// -> -> Runtime dependency: whenever two modules interact at runtime
// -> -> ->  when flow of control leaves one module and enters another
// -> -> -> when one module access the variables of another
// -> -> Compile time dependency: when a name is defined in one module, but appears in another module
//       You cannot just compile one module, you need to first compile the modules that that module depend upon.
// -> -> -> e.g. Dependency of A->B->C. When compiling A, need to first compile C and B.
//          Structure design
// -> -> topdown design, starting with main, until you specify every methods
// -> -> compile time dependencies (source code dependencies) run in the same direction as the runtime dependencies
// -> -> when source code dependencies are based on this structure, then we have a difficult time of keeping those
//      dependencies of crossing team boundaries
// -> -> we need to make the runtime dependencies run different than the source code dependencies structure!
//
// -> Dependency Inversion/Inversion of Control(IoC)
// -> -> the actual process of creating abstractions and getting them to replace dependencies.
// -> -> use polymorphism
// -> - A -> B with f() (both runtime and completive dependency)
// -> - polymorphism: A -> Interface with f() <|- B with f(), A would use interface, B would implement interface
// -> - now A still has runtime dependency on B, it does not have the compile time dependencies
// -> - Both A and B have source code dependencies on the interface!
// -> - note that the source code dependencies of B upon the interface, points in the opposite direction of the runtime dependency of A upon B
// -> - dependencies are inverted whenever the source code dependencies opose the direction of the flow of control
// -> - this is the way how we create boundaries in our software modules
// -> - whenever we want boundaries to exist, we careful choose which dependencies to invert against the flow of control, so that all the dependencies point to the same direction across the boundary
// -> - boundary like this is how we create plug-ins:
// -> -plugin is a module, that is anonymously called by another module, caller has no idea who is calling
// -> - that way you create an independently deployable and developable architecture is to compose it using plugins
// -> - thus devide the system with boundaries and invert the dependencies that cross this boundaries
//        Architectural Implications: High level policies(e.g. use cases) should not depend on low level details (e.g. databases, web formatting), low level details should depend on high level policy
// -> -  a good architecture is a plugin architecture using the DIP

enum class HeaterPlateMode { On, Off };
//class ElectricHeaterPlate {
//public:
//    void SetHeaterPlate(const HeaterPlateMode mode) {
//        std::cout << "set heater plate on/off" << std::endl;
//    }
//};

enum class Color { Green, Red };
//class ColorLed {
//public:
//    void SetColor(const Color color) {
//        std::cout << "set led to color" << std::endl;
//    }
//};
//
//class CoffeeMaker {
//public:
//    CoffeeMaker() : heater_(ElectricHeaterPlate()),
//                    led_(ColorLed()) {
//    }
//
//    void Make() {
//        heater_.SetHeaterPlate(HeaterPlateMode::On);
//        led_.SetColor(Color::Red);
//        //..other logic..
//    }
//
//private:
//    ElectricHeaterPlate heater_;
//    ColorLed led_;
//};
//
//TEST(dip, DIP) {
//    auto c = CoffeeMaker();
//    c.Make();
//}

// abstract interfaces
class HeaterElement {
public:
    virtual void SetOn() = 0;
    virtual void SetOff() = 0;
};
class Indicator {
public:
    virtual void SetActive() = 0;
    virtual void SetInactive() = 0;
};

// implementation of the interfaces
class ElectricHeaterPlate : public HeaterElement {
public:
    void SetOn() override {
        SetSwitch(HeaterPlateMode::On);
    };

    void SetOff() override {
        SetSwitch(HeaterPlateMode::Off);
    };
private:
    void SetSwitch(const HeaterPlateMode position) {
        std::cout << "set position of switch to electricity" << std::endl;
    }
};
class ColorLed : public Indicator {
public:
    void SetActive() override {
        SetColor(Color::Red);
    };

    void SetInactive() override {
        SetColor(Color::Green);
    };
private:
    void SetColor(Color color) {
        std::cout << "set heater plate on/off" << std::endl;
    }
};

class CoffeeMaker {
public:
    CoffeeMaker(HeaterElement &heater, Indicator &indicator)
            : heater_(heater), indicator_(indicator) {
    }

    void Make() {
        heater_.SetOn();
        indicator_.SetActive();
        //..other logic..
    }

private:
    HeaterElement &heater_;
    Indicator &indicator_;
};

TEST(dip, DIP2) {
    ColorLed led;
    ElectricHeaterPlate plate;
    auto c = CoffeeMaker(plate, led);
    c.Make();
}