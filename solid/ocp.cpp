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
//      OCP - Open close principle
//      Module should be open for extension and closed for modification
// -> Open for extentions: it should be very simple to change the behavoir of that module
// -> Closed for modification means the source code should not change
// -> How do we do this? Abstraction & Inversion
// -> When ever you have a module with behavior you want to extend without modifying it, you separate
//    the extensive behavior behind an abstract interface and then turn the dependencies around.
// -> If you design your system in conformance to the Open-Close Principle. When you modify them, you
//    can do so by adding new code. Not by changing old code.
// -> If you old code never gets modified, it will never rot…
// -> When violation of the OCP, will lead to design smell like rigidity, fragility and immobility.
// -> To create a system that perfectly conform to the OCP, one must be able to predict the future.
// -> However, by using a iterative process, with lots of feedback and refactoring, we can infect develop
//    systems that conform well enough to the OCP

enum class Color {
    Red, Green, Blue
};
enum class Size {
    Small, Medium, Large
};

class Candy {
public:
    std::string name;
    Color color;
    Size size;
};
//
//class Filter {
//    std::vector<Candy> by_color(const std::vector<Candy> &candy,
//                                Color color) {
//        std::vector<Candy> result;
//        for (auto &c: candy) {
//            if (c.color == color) {
//                result.push_back(c);
//            }
//        }
//        return result;
//    }
//};

class Filter {
public:
    std::vector<Candy> by_color(const std::vector<Candy> &candy,
                                Color color) const {
        std::vector<Candy> result;
        for (auto &c: candy) {
            if (c.color == color) {
                result.push_back(c);
            }
        }
        return result;
    }

    std::vector<Candy> by_size(const std::vector<Candy> &candy,
                               Size size) const {
        std::vector<Candy> result;
        for (auto &c: candy) {
            if (c.size == size) {
                result.push_back(c);
            }
        }
        return result;
    }
};

class ICriteria {
public:
    virtual bool is_satisfied(const Candy &candy) const = 0;
};

class ColorCriteria : public ICriteria {
public:
    ColorCriteria(const Color color) : color_(color) {
    }

    bool is_satisfied(const Candy &candy) const override {
        return candy.color == color_;
    }

private:
    Color color_;
};

class BetterFilter {
public:
    std::vector<Candy> filter(const std::vector<Candy> &candy,
                              const ICriteria &criteria) const {
        std::vector<Candy> result;
        for (auto &p : candy) {
            if (criteria.is_satisfied(p)) {
                result.push_back(p);
            }
        }
        return result;
    }
};

class SizeCriteria : public ICriteria {
public:
    SizeCriteria(const Size size) : size_(size) {
    }

    bool is_satisfied(const Candy &candy) const override {
        return candy.size == size_;
    }

private:
    Size size_;
};

class ColorAndSizeCriteria : public ICriteria {
public:
    ColorAndSizeCriteria(ColorCriteria color, SizeCriteria size)
            : color_(color), size_(size) {
    }

    bool is_satisfied(const Candy &candy) const override {
        return size_.is_satisfied(candy)
               && color_.is_satisfied(candy);
    }

private:
    ColorCriteria color_;
    SizeCriteria size_;
};

TEST(ocp, ocp1) { // never cast away const, instead use mutable
    Candy kever{"my kever", Color::Green, Size::Small};
    Candy tree{"Tree", Color::Green, Size::Large};
    Candy house{"House", Color::Blue, Size::Large};

    std::vector<Candy> all{kever, tree, house};
    BetterFilter bf;
    ColorCriteria green(Color::Green);

    auto green_things = bf.filter(all, green);
    EXPECT_EQ(green_things.size(), 2);
    for (auto &&x : green_things) {
        std::cout << x.name << " is green " << std::endl;
    }

    SizeCriteria small(Size::Small);
    auto small_candys = bf.filter(all, small);
    EXPECT_EQ(small_candys.size(), 1);
    for (auto &&x : small_candys) {
        std::cout << x.name << " is small " << std::endl;
    }

    SizeCriteria large(Size::Large);
    ColorAndSizeCriteria GreenAndLarge(green, large);
    auto green_and_large_candys = bf.filter(all, GreenAndLarge);
    EXPECT_EQ(green_and_large_candys.size(), 1);
    for (auto &&x : green_and_large_candys) {
        std::cout << x.name << " is green and large " << std::endl;
    }
}
