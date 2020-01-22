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

TEST(modern_cpp, unique_pointers) {

    auto hen1 = make_unique<Hen>(1, 2.2f);
    EXPECT_TRUE(hen1);

    auto hen2 = move(hen1); // move ownership of hen1 to hen2
    EXPECT_FALSE(hen1);     //
    EXPECT_TRUE(hen2);

    Hen& hen_ref1 = *hen1;
    //Hen hen_copy1 = *hen1;

    Hen& hen_ref2 = *hen2;
    Hen hen_copy2 = *hen2;
    EXPECT_EQ(hen_ref2.eggs_, 2.2f);
    EXPECT_EQ(hen_ref2.id_, 1);

    hen1.reset(hen2.release());
    EXPECT_TRUE(hen1);
    EXPECT_FALSE(hen2);

    // dit blijft geldig. het referentie wijst nog steeds
    // naar geheugen plek dat geldig is.
    EXPECT_EQ(hen_ref2.eggs_, 2.2f);
    EXPECT_EQ(hen_ref2.id_, 1);

    hen1.reset();

    // WHY????
    EXPECT_EQ(hen_ref2.eggs_, 2.2f);
    EXPECT_EQ(hen_ref2.id_, 1);
}

// transfer from ownership of pointer in an out of function explicit
auto GenHen(int id, float eggs) -> std::unique_ptr<Hen>
{
    return std::make_unique<Hen> (id, eggs);// this does a MOVE!!!!
    // same as: return move(std::make_unique<Hen> (id, eggs));
};

auto UpdateHen(std::unique_ptr<Hen> hen) -> std::unique_ptr<Hen>
{
    hen->eggs_ += 1.8f;
    return hen; // dit is een move
}

TEST(modern_cpp, unique_pointers2) {
    auto hen1 = GenHen(6, 3.3f);
    // UpdateHen(hen1); // dit gaat fout!
    auto hen2 = UpdateHen(move(hen1));
    EXPECT_EQ(hen2->eggs_, 3.3f+1.8f);

    std::function<std::unique_ptr<Hen>(int, float)> egg_factory = [](int id, float eggs) {
        return std::make_unique<Hen> (id, eggs);
    };

    auto hen3 = egg_factory(123, 45.6f);
    EXPECT_EQ(hen3->eggs_, 45.6f);
    EXPECT_EQ(hen3->id_, 123);
}

TEST(modern_cpp, shared_pointers1) {
    auto sp = std::shared_ptr<int> {};

    EXPECT_EQ(nullptr, sp);
    EXPECT_EQ(0, sp.use_count());
    EXPECT_FALSE(sp.unique());

    sp = std::make_shared<int>(123);

    EXPECT_NE(nullptr, sp);
    EXPECT_EQ(1, sp.use_count());
    EXPECT_TRUE(sp.unique());
    EXPECT_EQ(*sp, 123);        // dereference retern the object pointed by the syored pointer, same as *sp.get()
    EXPECT_EQ(*sp.get(), 123);  // sp.get gives back raw pointer

    auto sp2 = sp; // this already copies the pointer and increases the ref count

    EXPECT_NE(nullptr, sp);
    EXPECT_EQ(2, sp.use_count());
    EXPECT_FALSE(sp.unique());

    EXPECT_TRUE(sp.get() == sp2.get()); // zelfde als sp == sp2
    EXPECT_TRUE(sp == sp2);
}

TEST(modern_cpp, vectors) {
    auto c = std::vector<int> {};
    EXPECT_EQ(c.empty(), true);
    EXPECT_EQ(c.size(), 0);

    c = std::vector<int> {1,2,3,4,5};
    EXPECT_EQ(c.size(), 5);

    // mogelijk om de waardes als array aan te passen
    c[0] = 9;
    c[1] = 8;

    // mogelijk om waardes toe te voegen aan vector
    c.emplace_back(6);          // add to end
    c.emplace(begin(c), 0);     // add to begin

    for (auto&& e : c) {
        std::cout << std::to_string(e) << std::endl; // e is waarde
    }
    //zelfde als
    for (auto i = begin(c); i != end(c); ++i)
    {
        std::cout << "i: " << std::to_string(*i) << std::endl; // e is iterator
    }
    // deze is krachter en flexibeler
    for (auto i = rbegin(c); i != rend(c); ++i)     //reverse
    {
        std::cout << "i: " << std::to_string(*i) << std::endl; // e is iterator
    }
}

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

TEST(gol, vector_of_vectors_unique_ptr) {
    auto number_of_columns = 5;
    auto number_of_rows = 9;
    std::vector <std::vector< std::unique_ptr<int>>> matrix;
    //matrix.resize(number_of_columns, std::vector<std::unique_ptr<int>>(number_of_rows, std::make_unique<int>()));
    matrix.resize(number_of_columns);
    for (auto&& c : matrix){
        c.reserve(number_of_rows);
        for (auto i = 0; i < number_of_rows; ++i) {
            c.emplace_back(std::make_unique<int>());
        }
    }
}

TEST(gol, vector_of_vectors) {
    auto number_of_columns = 5;
    auto number_of_rows = 9;
    std::vector <std::vector<Cell>> matrix;
    matrix.resize(number_of_columns, std::vector<Cell>(number_of_rows, Cell()));
}

TEST(gol, vector_of_vectors2) {
    auto number_of_columns = 5;
    auto number_of_rows = 9;
    std::vector <std::vector< std::shared_ptr<Cell>>> matrix;
    matrix.resize(number_of_columns, std::vector<std::shared_ptr<Cell>>(number_of_rows, std::make_shared<Cell>()));
}

TEST(gol, vector_of_vectors3) {
    auto a = std::vector<int> {1,2,3,4,5};
    auto b = std::vector<int> ();

    b.insert(b.end(), a.begin(), a.end());

    EXPECT_EQ(b[0], 1);
    EXPECT_EQ(b[1], 2);
    EXPECT_EQ(b[2], 3);
    EXPECT_EQ(b[3], 4);
    EXPECT_EQ(b[4], 5);
}

TEST(modern_cpp, list) {
    auto l = std::list<int> {1,2,3,4,5};
    EXPECT_EQ(l.size(), 5);
    EXPECT_FALSE(l.empty());

    auto m = std::list<int> (begin(l), end(l));
    EXPECT_EQ(m.size(), 5);
    EXPECT_FALSE(m.empty());

    auto n = std::list<int> (++begin(l), --end(l));
    EXPECT_EQ(n.size(), 3);
    EXPECT_FALSE(n.empty());

    auto o = std::list<int> (10, 1234); // size 10, all value 1234
    EXPECT_EQ(o.size(), 10);

    for (auto o_local : o) {            // i is integer
        EXPECT_EQ(1234, o_local);
    }

    for (auto i = begin(l); i != end(l); ++i) {     // i is iterator
        std::cout << *i << std::endl;               // need to dereference
    }

    l.emplace_back(6);
    l.emplace_front(0);
    EXPECT_EQ(l.size(), 7);
    l.pop_front();
    l.pop_back();
    EXPECT_EQ(l.size(), 5);

    l.reverse();
    // std::list does not have random access operator [] because std::list internally store elements in doubly linked list.
    EXPECT_EQ(*std::next(l.begin(), 0), 5);
    EXPECT_EQ(*std::next(l.begin(), 1), 4);
    EXPECT_EQ(*std::next(l.begin(), 2), 3);
    EXPECT_EQ(*std::next(l.begin(), 3), 2);
    EXPECT_EQ(*std::next(l.begin(), 4), 1);
    for (auto i = begin(l); i != end(l); ++i) {
        std::cout << *i << std::endl;
    }

    l.sort();
    EXPECT_EQ(*std::next(l.begin(), 0), 1);
    EXPECT_EQ(*std::next(l.begin(), 1), 2);
    EXPECT_EQ(*std::next(l.begin(), 2), 3);
    EXPECT_EQ(*std::next(l.begin(), 3), 4);
    EXPECT_EQ(*std::next(l.begin(), 4), 5);
    for (auto i = begin(l); i != end(l); ++i) {
        std::cout << *i << std::endl;
    }

    l.remove_if([](int value) {
        return true; // remove if return true
    });
    EXPECT_TRUE(l.empty());
}

TEST(modern_cpp, movement_within_containers) {
    auto b = std::list <Hen> ();
    auto a = Hen("Henrietta");
    b.push_back(a);
    // move constuctor with name Henrietta -> IF NO MOVE CONSTUCTOR IS THERE, IT WILL COPY
    // destuctor with name null

    auto c = std::list <Hen> ();
    EXPECT_EQ(c.size(), 0);
    c.emplace_back(); // this will call default constructor
    EXPECT_EQ(c.size(), 1);
    c.emplace_back("Henrietta"); // this will fill in arguments in explicit constructor
    EXPECT_EQ(c.size(), 2);
    c.clear();
}

TEST(modern_cpp, set) {
    // A set is ordered. It is guaranteed to remain
    // in a specific ordering, according to a functor
    // that you provide. No matter what elements you
    // add or remove (unless you add a duplicate, which
    // is not allowed in a set), it will always be ordered.
    auto c = std::set<int, std::less<int>> ();
    EXPECT_TRUE(c.empty());
    EXPECT_EQ(c.size(), 0);

    c = std::set<int> {1 ,3, 5, 2, 4};
    EXPECT_EQ(c.size(), 5);

    for (auto number : c) {
        std::cout << number << std::endl;
    }

    auto result = c.emplace(6); // result is a pair pair<iterator, bool>
    EXPECT_EQ(*result.first, 6);
    EXPECT_TRUE(result.second);

    result = c.emplace(6);
    EXPECT_EQ(*result.first, 6);
    EXPECT_FALSE(result.second); // not added, since integer 6 was already in set

    EXPECT_EQ(c.erase(6), 1);
    EXPECT_FALSE(c.erase(1234));

    auto exist = c.find(3);
    EXPECT_EQ(*exist, 3);

    auto does_not_exist = c.find(1234); // if not found it will give back iterator to the end of set
    EXPECT_EQ(does_not_exist, end(c));
}

TEST(modern_cpp, map) {
    auto c = std::map<int, double>();

    EXPECT_EQ(c.size(), 0);

    c = std::map<int, double> {
            {1, 1.1},
            {4, 4.1},
            {2, 2.1},
            {3, 3.1},
            {5, 1.1}
    };
    EXPECT_EQ(c.size(), 5);

    // able to use [] like an array to set and get
    // get
    EXPECT_EQ(c[4], 4.1);
    // set
    c[6] = 6.1;

    auto v = c[7];
    EXPECT_EQ(v, 0.0);

    EXPECT_EQ(c.size(), 7);

    auto result = c.emplace(8, 8.1); // returns itter and bool
    EXPECT_EQ(((*result.first).first), 8);
    EXPECT_EQ(((*result.first).second), 8.1);
    EXPECT_TRUE(result.second);

    auto i = c.find(6); // find elements by value
    EXPECT_EQ(i->first, 6); // key
    EXPECT_EQ(i->second, 6.1); //value

    for (auto&& a : c) {
        std::cout << a.first << " " << a.second << std::endl;
    }

    // in c++17
    // for (auto && [key,val]: c) {
    //     std::cout << key << " " << val << std::endl;
    // }
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

auto operator==(Han const& left, Han const& right) -> bool {
    return left.name == right.name &&
           left.id == right.id;
}

namespace std
{
    template <>
    struct hash<Han>
    {
        // hash function -> two calls with the same value must give the same results!
        auto operator()(Han const & han) const -> size_t
        {
            boost::hash<std::string> string_hash;
            auto hash = string_hash(han.name + std::to_string(han.id));
            return hash;
        }
    };
}

TEST(modern_cpp, unorderd_map) {
    auto c = std::unordered_map<Han, double>  // Han moet een Hash fuctie en een equal functie hebben
            {
                    {{"jaap", 123}, 1.2},
                    {{"geid", 124}, 3.2},
                    {{"mier", 125}, 4.2}
            };

    c[{"piet", 345}] = 6.3;

    auto k = Han("gek", 2311);
    c[k] = 8.6;

    for (auto&& h: c){
        std::cout << h.first.name << " " << h.first.id << " " << h.second << std::endl;
    }
}

TEST(modern_cpp, unorderd_map2) {
    auto c = std::unordered_map<Han, Han>  // Han moet een Hash fuctie en een equal functie hebben
            {
                    {{"jaap", 123}, {"jip", 456}},
                    {{"geit", 124}, {"jap", 567}},
                    {{"mier", 125}, {"jop", 678}}
            };
    EXPECT_EQ(c.size(), 3);

    auto k = Han("piet", 345);
    c[k] = Han("jup", 789);
    EXPECT_EQ(c.size(), 4);

    for (auto&& h: c){
        std::cout << h.first.name << " " << h.first.id << " " << h.second.name << " " << h.second.id  << std::endl;
    }
}

auto trim (std::string const & s) -> std::string {
    auto front = find_if_not(begin(s), end(s), isspace);    //standaard find algorithms voor containers werkt voor strings
    auto back = find_if_not(rbegin(s), rend(s), isspace);

    return std::string(front, back.base());
}

TEST(modern_cpp, string_basic) {
    auto s = std::string();
    // same as -> string is typedef voor basic_string<char> container
    s = std::basic_string<char>();

    s = std::string("cluck!");

    EXPECT_FALSE(s.empty());
    EXPECT_EQ(s.size(), 6);

    std::cout<< s.c_str() << std::endl;

    for (auto&& c : s){
        std::cout<< c << std::endl;
    }

    auto hen = std::string("Mathilda");
    auto pasture = std::string("Tomatoes");
    auto id = hen + "@" + pasture;              // concat string with +

    std::cout<< id.c_str() << std::endl;

    auto pos = id.find('@');                    // gives back position in string
    auto pos2 = id.find('m');
    EXPECT_EQ(pos, 8);
    EXPECT_EQ(pos2, 11);
    std::cout<< "pos " << pos << " pos2 " << pos2 << std::endl;

    //auto domain = std::string(id, pos, pos2-pos); zelfde als
    auto domain = id.substr(pos, pos2-pos);
    std::cout<< "domain:" << domain << std::endl;

    auto trimmed = trim ("      \t Matilda the hen  \r\n   ");
    std::cout << trimmed << std::endl;
    EXPECT_EQ(trimmed, "Matilda the hen");
}

// string operations
TEST(boost_lib, string_concat) {
    std::string s2 = "hallo";
    std::string s3 = "wereld";
    auto s4 = s2 + " " + s3;
    EXPECT_EQ(s4, "hallo wereld");

}


#include <regex>
#include <boost/regex.hpp>
TEST(modern_cpp, regular_expressions) {
    auto s = std::string("Call 8770-808-2321 to reach Amedlia the hen!");

    auto r = std::regex{ R"((\d{4})-(\d{3})-(\d{4}))"};

    auto m = std::smatch (); // match results of string iterators
    //m = std::match_results<std::string::const_iterator>();
    EXPECT_TRUE(m.empty());

    EXPECT_TRUE(std::regex_search(s, m, r));
    EXPECT_FALSE(m.empty());
    EXPECT_EQ(m.size(), 4);

    for (auto&& sub : m){
        std::cout << "[" << std::string(&*sub.first, sub.length()) << "]" << std::endl;
    }
}
