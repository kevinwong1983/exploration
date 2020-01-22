#include "boost/variant.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

struct Hen
{
    char const * m_name;

    Hen() : m_name("") {
        std::cout<< "default constructor" << std::endl;
    }

    explicit Hen(char const * name) : m_name(name) {
        std::cout<< "explicit constuctor" << std::endl;
    }

    Hen(Hen const& other) : m_name(other.m_name) {
        std::cout<< "copy constuctor with name " << m_name << std::endl;
    }

    Hen(Hen && other) : m_name(other.m_name) {
        other.m_name = "null";
        std::cout<< "move constuctor with name " << m_name << std::endl;
    }

    ~Hen() {
        std::cout<< "destuctor with name " << m_name  << " " << id_ << " " << eggs_ << std::endl;
    }

    auto operator = (Hen const & other) -> Hen& {
        m_name = other.m_name;
        std::cout<< "copy assingment " << m_name << std::endl;
        return *this;
    }

    auto operator = (Hen && other) -> Hen& {
        m_name = other.m_name;
        other.m_name = "null";
        std::cout << "move assignment " << m_name << std::endl;
        return *this;
    }

    void swap (Hen & other){
        std::swap(m_name, other.m_name);
    }

    Hen(unsigned id, float eggs):
            id_(id), eggs_(eggs), m_name("") {
        std::cout << "custom constructor " << id_ << " " << eggs_ << std::endl;
    }

    unsigned id_;
    float eggs_;
};

class Cell {
public:
    Cell():i_(0){
        std::cout << "default constructor" << std::endl;
    };

    Cell(int i):i_(i){
        std::cout << "constructor" << std::endl;
    };

    ~Cell() {
        std::cout << "destructor" << std::endl;
    }

    int GetI() {
        return i_;
    }

    auto operator==(const Cell& rhs) -> bool {
        return i_ == rhs.i_;
    }

    bool operator==(const Hen& rhs){
        return i_ == rhs.id_;
    }

private:
    int i_;
    bool alive_;
};

TEST(algorithms, count) {
    // Use auto && for the ability to modify and discard values of the sequence within the loop. (That is, unless the container provides a read-only view, such as std::initializer_list, in which case it will be effectively an auto const &.)
    // Use auto & to modify the values of the sequence in a meaningful way.
    // Use auto const & for read-only access.
    // Use auto to work with (modifiable) copies.
    auto v = std::vector<int> {1,2,3,4,5,6,7,8,9};
    auto s = std::vector<Cell> ();
    for(auto&& i : v) {
        s.emplace_back(Cell(i));
    }

    // count number of elements contain the integer 2
    auto c = std::count(begin(v), end(v), 2);
    EXPECT_EQ(1, c);
    std::cout << "count: " << c << std::endl;

    auto target = Hen("mar");
    target.id_ = 2;
    auto d = std::count_if(begin(s), end(s), [&target] (auto&& local_s) {
        return local_s == target;
    });

    EXPECT_EQ(1, d);
    std::cout << "count: " << d << std::endl;
}

TEST(algorithms, count_none_any_all) {
    auto v = std::vector<int> {1,2,3,4,5,6,7,8,9};
    auto all = std::all_of(begin(v),end(v),[](auto&& v_local){
        return v_local == 2;
    });
    auto any = std::any_of(begin(v),end(v),[](auto&& v_local){
        return v_local == 2;
    });
    auto none = std::none_of(begin(v),end(v),[](auto&& v_local){
        return v_local == 2;
    });
    EXPECT_FALSE(all);
    EXPECT_TRUE(any);
    EXPECT_FALSE(none);
}

TEST(algorithms, find) {
    auto v = std::vector<int> {2,4,6,6,1,3,-2,0,11,2,3,2,4,4,2,4};

    //find first zero in collection
    auto result = std::find(begin(v), end(v), 0);
    EXPECT_EQ(0, *result); // result is an iterator, de-reference the result to get the value;

    // find the first 2 after that zero
    result = std::find (result, end(v), 2);
    if (result != end(v)) {         // find will give back iterator after end
        EXPECT_EQ(2, *result);
    }

    // find first odd value
    result = std::find_if(begin(v), end(v), [](auto&& v_local) {return v_local%2 != 0;});
    EXPECT_EQ(1, *result);
    result = std::find_if_not(begin(v), end(v), [](auto&& v_local) {return v_local%2 != 0;});
    EXPECT_EQ(2, *result);

    auto primes = std::vector<int> {1,2,3,5,7,11,13};
    result = std::find_first_of(begin(v), end(v), begin(primes), end(primes));
    EXPECT_EQ(2, *result);

    // Searches the range [first1,last1) for the first occurrence of the sequence
    // defined by [first2,last2), and returns an iterator to its first element,
    // or last1 if no occurrences are found
    auto subsequence = std::vector<int> {2 ,4};
    result = std::search(begin(v), end(v), begin(subsequence), end(subsequence));

    result = std::search_n(begin(v), end(v), 2, 4); // two times a 4
    result = std::adjacent_find(begin(v), end(v));
}

TEST(algorithms, find_all_elements) {
    auto v = std::vector<int> {2,4,6,6,1,3,-2,0,11,2,3,2,4,4,2,4};

    auto odd = std::vector<int> ();
    auto it = v.begin();
    while((it = std::find_if(it, v.end(), [](int i) {return i%2;})) != v.end()) {
        odd.emplace_back(*it);
        it ++;
    }
    EXPECT_EQ(odd.size(), 4);
    EXPECT_EQ(odd[0], 1);
    EXPECT_EQ(odd[1], 3);
    EXPECT_EQ(odd[2], 11);
    EXPECT_EQ(odd[3], 3);
}

TEST(algorithms, simple_sort) {
    auto v = std::vector<int> {4,1,0,-2,3,7,-6,2,0,0,-9,9};
    auto v2 = v;
    std::sort(begin(v2), end(v2));
    std::sort(begin(v2), end(v2), [](int a, int b) {return a > b;});
    std::sort(begin(v2), end(v2), [](int a, int b) {return std::abs(a) > std::abs(b);});
}

class Employee {
public:
    Employee(std::string first, std::string last, int sal): first_(first), last_(last), sal_(sal) {};
    int getSalary() const {return sal_;};
    std::string getSortingName() const {return last_ + ", " + first_;};
    bool operator<(const Employee that) const {return sal_ < that.sal_;};       // needs < operator to use std::sort
private:
    std::string first_;
    std::string last_;
    int sal_;
};

TEST(algorithms, simple_sort2) {
    auto staff = std::vector<Employee> {
            {"Kate", "Greg", 1000},
            {"Ob", "Art", 2000},
            {"Fake", "Name", 1000},
            {"Alan", "Turing", 2000},
            {"Frace", "Hopper", 2000},
            {"Anita", "Borg", 2000},
    };

    //std::sort(begin(staff), end(staff)); // deze maakt gebruikt van de operator < van Employee. Als deze er niet is, dan geeft het een compile error
    std::sort(begin(staff), end(staff), [](auto&& a, auto&& b) {return a<b;});

    // als je will dat de gesorteerd salaris ook de naam is gesorteerd -> do eerst een sort op naam, dan stable_sort op salaris
    std::sort(begin(staff), end(staff), [](auto&& a, auto&& b) {return a.getSortingName() <b.getSortingName();});
    std::stable_sort(begin(staff), end(staff), [](auto&& a, auto&& b) {return a<b;}); // andere variable workden in zelfde volgorde gelaten
}

TEST(algorithms, find_largest) {
    auto v = std::vector<int>{2, 4, 6, 6, 1, 3, -2, 0, 11, 2, 3, 2, 4, 4, 2, 4};
    auto max = *std::max_element(begin(v), end(v)); // * dereference to value int
    EXPECT_EQ(11, max);
    auto min = *std::min_element(begin(v), end(v));
    EXPECT_EQ(-2, min);

    auto positive = *std::upper_bound(begin(v), end(v), 0);
}

TEST(algorithms, comparing) {
    auto a = std::vector<int>{1,2,3,4,5};
    auto b = std::vector<int>{1,2,0,4,5};

    auto same = std::equal(begin(a), end(a), begin(b), end(b)); // for own classe you need to make an "operator==" for comparing classes
    EXPECT_FALSE(same);

    // mismatch returns a pair of iterators!
    auto first_change = std::mismatch(begin(a), end(a), begin(b));
    int first_change_in_a = *(first_change.first);
    int first_change_in_b = *(first_change.second);
    EXPECT_EQ(3, first_change_in_a);
    EXPECT_EQ(0, first_change_in_b);
}

TEST(algorithms, loop_in_disquise) {
    auto v = std::vector<int>{2, 4, 6, 6, 1, 3, -2, 0, 11, 2, 3, 2, 4, 4, 2, 4};
    // manier 1
    for (auto it = begin(v); it != end(v); it++) {
        *it = 0;
    }
    // mamier 2
    for (auto& i: v) { // de ref zorgt ervoor dat de waardes van vector v aangepast worden!
        i = 1;
    }
    // manier 3
    for_each(begin(v), end(v), [](auto& v_local) { v_local = 2;}); // de ref zorgt ervoor dat de waardes van vector v aangepast worden!

    // Use auto && for the ability to modify and discard values of the sequence within the loop. (That is, unless the container provides a read-only view, such as std::initializer_list, in which case it will be effectively an auto const &.)
    // Use auto & to modify the values of the sequence.
    // Use auto const & for read-only access.
    // Use auto to work with (modifiable) copies.
    for (auto& i: v) { // de ref zorgt ervoor dat de waardes van vector v aangepast worden!
        i = 1;
    }
    // of
    for_each(begin(v), end(v), [](auto&& v_local) { v_local = 2;}); // de ref zorgt ervoor dat de waardes van vector v aangepast worden!

    for (auto&& i: v) { // de ref ref zorgt ervoor dat de waardes van vector v aangepast worden EN verwijderd kan worden !
        v.pop_back();
    }
    EXPECT_EQ(v.size(),0);

    v = std::vector<int>{2, 4, 6, 6, 1, 3, -2, 0, 11, 2, 3, 2, 4, 4, 2, 4};
    for (auto& i: v) { // Use auto & to modify the values of the sequence. ??????
        v.pop_back();
    }
    EXPECT_EQ(v.size(),0);
}

TEST(algorithms, copy) {
    auto v = std::vector<int> {3,6,1,0,-2,5};
    auto v2 =  std::vector<int>(v.size());
    EXPECT_EQ(v2.size(), v.size());

    std::copy(begin(v), end(v), begin(v2));
    EXPECT_TRUE(std::equal(begin(v), end(v), begin(v2), end(v2)));

    auto v3 = v;                                // dit is het zelfde als voorgaande statement
    auto v4 = std::vector<int>(v.size());
    auto position = find(begin(v), end(v), 1);  // position is een iterator
    std::copy(begin(v), position, begin(v4));   // position is end -> wordt niet meegerekend in loop
    EXPECT_EQ(v4[0], 3);
    EXPECT_EQ(v4[1], 6);
    EXPECT_EQ(v4[2], 0);    // o is default value

    auto v5 = std::vector<int> (v.size());
    std::copy_if(begin(v), end(v), begin(v5), [](auto&& e) {
        return e%2 == 0;
    });
    EXPECT_EQ(v5[0], 6);
    EXPECT_EQ(v5[1], 0);
    EXPECT_EQ(v5[2], -2);
    EXPECT_EQ(v5[3], 0);    // o is default value

    auto v6 = std::vector<int> (v.size());
    copy_n(begin(v5), 3, begin(v6));
    EXPECT_EQ(v6[0], 6);
    EXPECT_EQ(v6[1], 0);
    EXPECT_EQ(v6[2], -2);

    // als je naar de zelfde vector copieerd, en er is overlap, dan moet je copy_backwards doen
    // std::copy(begin(v), end(v)-1, end(v)+1); // dit geeft problemen
    std::copy_backward(begin(v), end(v)-1, end(v)+1);
}

TEST(algorithms, remove) {
    auto v = std::vector<int> {3,6,1,0,-2,5};
    auto newEnd = std::remove(begin(v), end(v), 3);
    EXPECT_EQ(6, v.size()); // still size is 6, SIZE OF VECTOR HAS NOT CHANGED!
    int logicalSize = newEnd - begin(v);// logical size is 5
    v.erase(newEnd, end(v)); // here we erase it
    EXPECT_EQ(5, v.size());

    // one liner removes and erases from vector c
    v.erase(std::remove(begin(v), end(v), 1), end(v));
}

class Resource {

public:
    Resource() {
        r_object_count ++;
        std::cout << "constuctor " << r_object_count << std::endl;
    }
    Resource(const Resource& r) { // copy constructor
        i = r.i;
        r_object_count++;
        std::cout << "copy constuctor " << r_object_count << std::endl;
    }
    Resource& operator=(const Resource& r) { // assign operator
        std::cout << "assign " << r_object_count << std::endl;
        i = r.i;
        return *this;

    }
    ~Resource(){
        r_object_count--;
        std::cout << "destuctor " << r_object_count << std::endl;
    }
    void setValue(int ii) {i = ii;}
    int getValue() {return i;}

    static int r_object_count;// SIMPLE REFERENCING COUNTING
private:
    int i = 1;
};

int Resource::r_object_count = 0;

TEST(algorithms, remove2) {
    auto v = std::vector<Resource> (2);
    v[0].setValue(8);
    v[1].setValue(9);
    EXPECT_EQ(v[0].getValue(), 8);
    EXPECT_EQ(v[1].getValue(), 9);

    auto newEndIterator = std::remove_if(begin(v), end(v), [](auto&& e) {  // gives back iterator of new end
        return e.getValue() == 8;
    });
    EXPECT_EQ(v.size(), 2); // SIZE OF VECTOR HAS NOT CHANGED!
    EXPECT_EQ(v[0].getValue(), 9);  // shifted, but not erased
    EXPECT_EQ(v[1].getValue(), 9);

    v.erase(newEndIterator, end(v));  // now really erased
    EXPECT_EQ(v.size(), 1);
}

TEST(algorithms, remove3) {
    auto v = std::vector<int> {1,2,3,4,5,6,7,8,9};
    EXPECT_EQ(v.size(), 9);
    auto it = std::remove_if(v.begin(), v.end(), [](int i){  // remove all the odd numbers
        return (i%2);
    });
    // it contains all iterators of odd values
    v.erase(it, v.end());
    EXPECT_EQ(v.size(), 4);
    for (auto a: v){
        std::cout<< a << std::endl;
    }

    v = std::vector<int> {1,2,3,4,5,6,7,8,9};                // one liner
    v.erase(std::remove_if(v.begin(), v.end(), [](int i){
        return (i%2);
    }), v.end());
    EXPECT_EQ(v.size(), 4);
}

#include <numeric> // needed for iota
TEST(algorithms, fill) {
    // vector::begin() VS std::begin()
    // std::begin() was added in C++11 to make it easier to write generic code (e.g. in templates).
    // The most obvious reason for it is that plain C-style arrays do not have methods, hence no .begin().
    // So you can use std::begin() with C-style arrays, as well as STL-style containers having their own
    // begin() and end().
    //
    // If you're writing code which is not a template, you can ignore std::begin(); your fellow
    // programmers would probably find it odd if you suddenly started using it everywhere just
    // because it's new.

    auto v = std::vector<int> (10);
    std::fill(begin(v), end(v), 1);  // used copy
    EXPECT_EQ(v[0], 1);

    std::fill_n(begin(v), 6, 2);
    EXPECT_EQ(v[5], 2);
    EXPECT_EQ(v[6], 1);

    // Fills the range [first, last) with sequentially increasing values, starting with value and repetitively evaluating ++value.
    std::iota(begin(v), end(v), 1);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);

    int i = 1;
    std::generate(begin(v), end(v), [&i]() {return (i *= 2);});
    EXPECT_EQ(v[0], 2);
    EXPECT_EQ(v[1], 4);
    EXPECT_EQ(v[2], 8);
    EXPECT_EQ(v[3], 16);

    //////////
    i = 1;
    auto c = std::vector<std::unique_ptr<Cell>>(10 /*, std::make_shared<Cell>(Cell(++i))*/);
    std::generate(begin(c), end(c), [&i](){ return std::make_unique<Cell>(Cell(++i));});
}

TEST(algorithms, replace) {
    auto v = std::vector<int> {2,3,4,5,3,1,4,65,3,4,-3,-2};
    std::replace(begin(v), end(v), 3, 100);  // used copy
    EXPECT_EQ(v[1], 100);
    EXPECT_EQ(v[4], 100);
    EXPECT_EQ(v[8], 100);

    std::replace_if(begin(v), end(v), [](auto&& e) {
        return e < 0;
    }, 0);
    EXPECT_EQ(v[10], 0);
    EXPECT_EQ(v[11], 0);
}

struct Han {
    std::string name;
    int id;
    int GetId() const {
        return id;
    }
    Han(): name(""), id(0) {}
    Han(std::string name_, int id_): name(name_), id(id_) {}
};

TEST(algorithms, transform) {
    auto v = std::vector<int> {2,3,4,5,3,1,4,65,3,4,-3,-2};
    std::transform(begin(v), end(v), begin(v), [](auto&& e) {return e*2;});  // used copy
    EXPECT_EQ(v[0], 4);
    EXPECT_EQ(v[1], 6);
    EXPECT_EQ(v[2], 8);

    auto v1 = std::vector<int> {2,3,4,5,3,1,4,65,3,4,-3,-2};
    auto v2 = std::vector<int> {2,3,4,5,3,1,4,65,3,4,-3,-2};
    auto v3 = std::vector<int> (v.size());
    std::transform(begin(v1), end(v1), begin(v2), begin(v3), [](auto&& e1, auto&& e2) {return e1+e2;});  // used copy
    EXPECT_EQ(v3[0], 4);
    EXPECT_EQ(v3[1], 6);
    EXPECT_EQ(v3[2], 8);

    // transforms vector of Hans to vector of Cells
    auto hans = std::vector<Han> (10);
    int j = 1;
    std::generate(begin(hans), end(hans), [&j](){
        return Han("", j++);
    });
    auto cells = std::vector<Cell> (10);
    std::transform(begin(hans), end(hans), begin(cells), [](auto&& h) {return Cell(h.GetId());});
}

TEST(algorithms, back_inserter) {
    // with back_inserter iterator you dont have to pre-allocate memory.
    // memory will be added when needed
    auto v1 = std::vector<int>();
    std::fill_n(back_inserter(v1), 10, 2);
    for(auto&& e : v1){
        EXPECT_EQ(e, 2);
    }
    EXPECT_EQ(v1.size(), 10);

    // [n=0] is alias capturing -> zelde als 'n' buiten declarene met [&n].
    // mutable zegt dat mijn member variabelen van mijn lambda aan te passen zijn door de code van mijn lambda.
    std::generate_n(back_inserter(v1), 10, [n=0]()mutable{return n++;});
    // IPV
    // auto hans = std::vector<int> (10);
    // std::generate(begin(v1), end(v1), [n=0]()mutable{ return n++; });
    EXPECT_EQ(v1.size(), 20);
    EXPECT_EQ(v1[10], 0);
    EXPECT_EQ(v1[11], 1);
    EXPECT_EQ(v1[12], 2);

    auto cells = std::vector<std::unique_ptr<Cell>>();
    std::generate_n(back_inserter(cells), 10, [n=0]()mutable{
        return std::make_unique<Cell>(Cell(n++));
    });

    // back_inserter werkt met de volgende argorithms
    // fill_n, generate_n, transform, copy, copy_if, unique_copy, reverse_copy
    auto v2 = std::vector<int>();
    std::transform(begin(v1), end(v1), back_inserter(v2), [](auto&& e) {return e*2;});
    // IPV
    // auto v2 = std::vector<int>(v1.size());
    // std::transform(begin(v1), end(v1), begin(v2), [](auto&& e) {...});
    EXPECT_EQ(v2.size(), 20);
    EXPECT_EQ(v2[0], 4);
    EXPECT_EQ(v2[1], 4);
    EXPECT_EQ(v2[2], 4);
    // ...
    EXPECT_EQ(v2[10], 0);
    EXPECT_EQ(v2[11], 2);
    EXPECT_EQ(v2[12], 4);

    auto v3 = std::vector<int> {3,6,1,0,-2,5,-2};
    auto v4 = std::vector<int> ();
    std::copy_if(begin(v3), end(v3), back_inserter(v4), [](auto&& e) {
        return e != 3;
    });
    // IPV
    // auto v5 = std::vector<int> (v.size());
    // std::copy_if(begin(v), end(v), begin(v5), [](auto&& e) {...});
    EXPECT_EQ(v4.size(), v3.size()-1);
    EXPECT_EQ(v4[0], 6);
    EXPECT_EQ(v4[1], 1);

    auto v5 = std::vector<int>();
    std::unique_copy(begin(v3), end(v3), back_inserter(v5));
    EXPECT_EQ(v5.size(), v3.size()-1);
}

TEST(algorithms, reverse_iterators) {
    auto v1 = std::vector<int>(10);
    std::iota(begin(v1), end(v1), 1);
    EXPECT_EQ(v1[0], 1);
    EXPECT_EQ(v1[1], 2);
    EXPECT_EQ(v1[2], 3);

    // reverse copy
    auto v2 = std::vector<int>();
    copy(rbegin(v1), rend(v1), back_inserter(v2));
    EXPECT_EQ(v2[0], 10);
    EXPECT_EQ(v2[1], 9);
    EXPECT_EQ(v2[2], 8);
}

TEST(algorithms, swap) {
    auto even = std::vector<int> {2,4,6,8,10};
    auto odd = std::vector<int> {1,3,5,7,9};

    auto v1 = even;
    std::swap(v1[0], v1[1]);
    EXPECT_EQ(v1[0],4);
    EXPECT_EQ(v1[1],2);

    auto v2 = odd;
    std::swap(v1[0], v2[0]);
    EXPECT_EQ(v1[0],1);
    EXPECT_EQ(v2[0],4);

    v1 = even;
    v2 = odd;
    std::iter_swap(begin(v1), find(begin(v2), end(v2), 5)); // handy with functions that give back itterators
    EXPECT_EQ(v1[0],5);
    EXPECT_EQ(v2[2],2);

    v1 = even;
    v2 = odd;
    std::swap_ranges(begin(v1), find(begin(v1),end(v1),6), begin(v2));
    EXPECT_EQ(v1[0],1);
    EXPECT_EQ(v1[1],3);
    EXPECT_EQ(v2[0],2);
    EXPECT_EQ(v2[1],4);
}

TEST(algorithms, partial_sort_rotate_stable_partition) {
    // handige algoritmes

    // partial_sort_copy     => top_n
    auto v = std::vector<int> {7,2,5,4,3,6,1};
    auto v2 = std::vector<int> (v.size());
    std::partial_sort_copy(begin(v), end(v), begin(v2), end(v2));
    EXPECT_EQ(v2[0],1);
    EXPECT_EQ(v2[1],2);
    EXPECT_EQ(v2[2],3);
    EXPECT_EQ(v2[3],4);
    EXPECT_EQ(v2[4],5);
    EXPECT_EQ(v2[5],6);
    EXPECT_EQ(v2[6],7);

    // rotate                => move_range_within_collection
    auto v3 = std::vector<int> (6);
    std::iota(begin(v3), end(v3), 1);
    EXPECT_EQ(v3[0],1);
    EXPECT_EQ(v3[1],2); // <-
    EXPECT_EQ(v2[2],3); //
    EXPECT_EQ(v3[3],4); // ->
    EXPECT_EQ(v2[4],5);

    auto start_iter = find(begin(v3), end(v3), 2);
    auto end_iter = find(begin(v3), end(v3), 4);
    std::rotate(start_iter, end_iter, end_iter+1); // end_iter+1, so 4 will be includen
    EXPECT_EQ(v3[0],1);
    EXPECT_EQ(v3[1],4); // <-
    EXPECT_EQ(v3[2],2); // rotate
    EXPECT_EQ(v3[3],3); // ->
    EXPECT_EQ(v3[4],5);

    // stable_partition      => gather
    // 1 ->     1 <-    2
    // 2        3       1 <-
    // 3 ->     5       3
    // 4        7 ->    5
    // 5 ->     2       7 ->
    // 6        4       4
    // 7 ->     6       6
    // 8        8       8
    auto v4 = std::vector<int> (8);
    std::iota(begin(v4), end(v4), 1);
    auto selected = std::stable_partition(begin(v4), end(v4), [](auto&& e) {return e%2!=0;}); // gives back an iterator between the border of selected and unselected
    EXPECT_EQ(v4[0],1); // -> partitioned to the top
    EXPECT_EQ(v4[1],3);
    EXPECT_EQ(v4[2],5);
    EXPECT_EQ(v4[3],7); // -> partitioned to the top
    EXPECT_EQ(v4[4],2);
    EXPECT_EQ(v4[5],4); // stable partition will keep this in order
    EXPECT_EQ(v4[6],6);
    EXPECT_EQ(v4[7],8);
    auto four = std::find(begin(v4), end(v4), 4);
    std::rotate(begin(v4), selected, four);     // not include 4! rotate(first, newfirst, last)
    EXPECT_EQ(v4[0],2);
    EXPECT_EQ(v4[1],1); // <-
    EXPECT_EQ(v4[2],3);
    EXPECT_EQ(v4[3],5);
    EXPECT_EQ(v4[4],7); // ->
    EXPECT_EQ(v4[5],4);
    EXPECT_EQ(v4[6],6);
    EXPECT_EQ(v4[7],8);
}

// Iterator Parameter
// normal two iterators to define ranges to e.g find, generate etc
// -> end(v) is always 1 past the last of vector
// iterator and a number
// three iterator
// -> start, end, starting of destination
// -> uitzondering is rotate deze is rotate(first, newfirst, last)
// -> uitzondering is partial_sort => partial_sort(first, midle, last)
// four iterator
// -> start and end, define range of start sequence you are looking for
// -> uitzondering is rotate_copy => rotate_copy(first, newfirst, last, destfirst)
//
// Integer Parameter
// if function has _n than second parameter is size (number of elements)
//
// Perdicate Last = lambda
// meestal overloading. sort(first, last)  of  sort(first, last, predicate)
// -> uitzondering replace_if en replace_copy_if. have predicate in 3rd parameter, and preplace value in 4rth parameter

// Ends with _if
// -> add a predicate or, if original takes value, replace with predicate
// Ends with _n
// -> Replace fist, last  with  first, size
// Starts with is_
// -> returns bool
// -> uitzondering is_X_until; returns iterator
// Starts with stable_
// -> elements keep their relative order if the function has no reason to swap them
