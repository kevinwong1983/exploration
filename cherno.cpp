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
std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

int GetRValue() {
    return 10;
}

// returns an integer reference to a lvalue
int &GetLValue() {
    static int value = 0; // I need to provide a storage for this value;
    return value;
}

void SetValue(int value) {
}

// this only takes lvalues!
void SetLValue(int &value) {
}

// this only both lvalues and rvalue due to the const ref!
void SetLRValue(const int &value) {
}

TEST(cherno_lvalues_and_rvalues, integer) {
    int i = 10; // i = lvalue, 10 = rvalue:
    // most of the time lvalue is on the left side of the equal sign.
    // most of the time rvalue is on the right side of the equal sign.
    // however this does not always apply.

    // lvalue i is an actual variable with a location in memory.
    // rvalue 10 is simply a value, just a numeric literal, just 10. It has no storage and no location until it is asinged to an lvalue.
    // e.g. you cannot do this: 10 = i; because 10 does not have a location, we cannot store data in 10.

    int a = i; // both a and i are lvalues. (thus left and right to equal sign does not always make sense)

    // rvalue does not always have to be the result of a assignment on the right of a numeric literal like above.
    // it can also be a result of a function.
    i = GetRValue();
    // in this case GetRValue() returns a rvalue, it is returning a temperary value. eventhough it is returning
    // an integer, it has not location, it has no storage.

    // GetValue = 10; // error: expression is not assignable
    // here it gets interesting: if function would return a lvalue than assignment like above is possible.
    GetLValue() = 10;

    // to expand on this: if I had a SetValue(int value) function, i can call it with both an lvalue and a rvalue.
    SetValue(i); // here I am calling it with an lvalue;
    SetValue(10); // here I am calling it with an rvalue;

    // rule: you can not take an lvalue reference of an rvalue.
    // you can only have an lvalue reference of an lvalue.
    // int& b = 10 //error:
    // SetLValue(10); //error: candidate function not viable: expects an l-value for 1st argument void SetLValue(int& value)

    // even though I cannot have lvalue ref of an rvalue, I can have a const lvalue ref of an rvalue.
    const int &b = 10;  // const lvalue ref of rvalue
    // this is a kind of work around. Compiler will create a temporary storage for you.
    int temporary = 10;
    const int &c = temporary;

    // using this I can make my Set function to accept both rvalue and lvalue.
    SetLRValue(i);
    SetLRValue(10);
}

// accepts both lvalue and rvalue
std::string PrintName(const std::string &name) {
    std::cout << name << std::endl;
    return "L";
}

// accepts only rvalue
std::string PrintRValueName(std::string &&name) { // uses two '&' indicate this function only accepts rvalues
    std::cout << name << std::endl;
    return "R";
}

// overload PrintName that only accepts rvalue
std::string PrintName(std::string &&name) { // uses two '&' indicate this function only accepts rvalues
    std::cout << name << std::endl;
    return "R";
}

TEST(cherno_lvalues_and_rvalues, strings) {
    // Same rules apply to strings which we will use as the next example.
    std::string s1 = "hello";   // s1 is lvalue, "hello" is rvalue
    std::string s2 = "world";    // s2 is lvalue, "world" is rvalue
    std::string s3 = s1 + s2;   // s3 is lvalue, s1 + s2 is rvalue

    // function with a const lvalue ref as argument, accepts both rvalues and lvalues
    PrintName(s1);
    PrintName(s1 + s2);

    // we can also create a function that only accepts rvalues;
    // PrintRValueName(s1); // error: candidate function not viable: no known conversion from 'std::string' to 'std::string &&' for 1st argument void PrintRValueName (std::string&& name)
    PrintRValueName(s1 + s2);

    // we clearly have a nice way to detect if something is a lvalue or an rvalue!
    // we can overload the Print function, depending on if argument is lvalue or rvalue, a different function will be called.
    EXPECT_EQ("L", PrintName(s1));
    EXPECT_EQ("R", PrintName(s1 + s2));

    // why is this usefull: optimisations in move semantics.
    // if we know we are taking in an rvalue object, its just a temporary without memory or location.
    // we can just "steal" the resouces that might be attacted to that object and use them somewhere else,
    // without having to copy or keep the value intact. We know it is temporary.
    // PrintName(const std::string& name), the argument can be used somehwere else.
    // PrintName (std::string&& name),  the argument is only used in that function!
}

class String {
public:
    String() = default;

    String(const char *string) {    // normal constructor
        std::cout << "created" << std::endl;
        size_ = strlen(string);
        data_ = new char[size_];
        memcpy(data_, string, size_);
    }

    String(const String &other) {   // copy constructor
        std::cout << "copied by constructor" << std::endl;
        size_ = other.size_;
        data_ = new char[size_];
        memcpy(data_, other.data_, size_);
    }

    String(String &&other) {    // move constructor
        std::cout << "moved by constructor" << std::endl;
        size_ = other.size_;
        data_ = other.data_; // we only take the pointer, not copying
        // this is just a shallow copy. we rewired the pointers.

        // we need to also take care of the temporay object (other), we are stealing his data.
        other.data_ = nullptr; // when this other object gets deleted, it is only deleting a null object. which does nothing.
        other.size_ = 0;
    }

    String &operator=(String &&other) {    // assignment operator
        std::cout << "moved by assingment" << std::endl;

        if (this != &other) {   // just a check if you are moving yourself to yourself.
            // note that data already exist here! We need to clean up our own data
            // befor assigning th other data to myself.
            delete[] data_;

            size_ = other.size_;
            data_ = other.data_; // we only take the pointer, not copying
            // this is just a shallow copy. we rewired the pointers.

            // we need to also take care of the temporay object (other), we are stealing his data.
            other.data_ = nullptr; // when this other object gets deleted, it is only deleting a null object. which does nothing.
            other.size_ = 0;
        }
        return *this;
    }

    void Print() {
        for (int i = 0; i < size_; i++) {
            std::cout << data_[i];
        }
        std::cout << std::endl;
    }

    ~String() {
        std::cout << "destroyed" << std::endl;
        delete data_;
    }

private:
    char *data_;
    uint32_t size_;
};

class Entity {
public:
    Entity(const String &name) : name_(name) {}

    Entity(String &&name) : name_(
            (String &&) name) {} //this constructor takes in a rvalue, needs to explicitly cast to a rvalue
    // Entity(String&& name) : name_(std::move(name)) {} // same as above!
    void PrintName() {
        name_.Print();
    }

private:
    String name_;

};

TEST(cherno_move_semantics, simple_example) {
    // single biggest usecase of lvalues and rvalues: move semantics!
    // move semantics allow us to move objects around. This was not possible before c++11 because here it
    // introduces rvalue references, which are neccessary for move semantics

    // basic problem move semantic soves is copying data.
    // if I am passing a object to a function, that is taking ownership of that object, I have no choice but to copy it.
    // so I need to 1. create the object in my stackframe, 2. copy that object and 3. pass it to that function.
    // same if I have a function that retuns an object, I first need to create that object and then return it (and again copying that data). This does get optimise by the compiler using return value optimisations

    // if your function just takes in a couple of objects, than it is not a big deal.
    // but what if your pass a object that needs to heap allocate a chunk of memory. That becomes a heavy object to copy.
    // if we start to use move semantics, the performance becomes a lot higher.

    Entity entity(String("HelloWorld"));
    entity.PrintName();
    // this will show: created, copied, HelloWorld.
    // This indicated first that the String constructor was called (String was created)
    // then Copy Constructor was called (when String was passed to entity)

    // the fact that copy constructor needs to allocate memory in the heap is a problem.
    // depending on the amount of data can be expensive
    // the only thing we want to do is to get the string in entity... for this we need to allocate memory twice.
    // what we want is to create it once, and then move it into entity! Then we only need to allocate once.
    // we can! here is where move semantics comes in.

    // move semantics just does a rewiring of the pointers & and empty out the old object.
}

TEST(cherno_move, stadard_move) {
    String string = "Hello";
    String dest1 = string; // this does a copy.

    // if we want to move string, we need to force it to use the move-constructor, wich does the stealing of the resouces
    // we can force it to use move-constructor by making string a temporary
    String dest2 = (String &&) string;    // casting is not that nice, also sometimes you use auto and cant deduce the type.
    String dest3 = std::move(string);   // utility function that does the casting for us.
    String dest4(std::move(string));    // same

    String dest5("");             // when Object already exist:
    dest5 = std::move(
            string);          // this uses the move-assignment operator: You need to create this operator function yourself
    // same as:
    dest5.operator=(std::move(string)); // this clearly show it is using the move-assignment operator

    // special case for move operator: moving you to yourself.
    // move assignment operator needs a check fo this.
    dest5 = std::move(dest5);

    String apple = "Apple";
    String dest6("");
    apple.Print();
    dest6.Print();
    dest6 = std::move(apple);
    apple.Print();
    dest6.Print();

    // summary:
    // std::move is use to convert an object to be a temporary: if you need an existing object
    // to be an temporary, you are basically marking on object "you can steal resources from me"
    // If you have an existing variable, you need to use std::move to mark it as temporary firs
    // before you can force it to use the move-assignment operator to steal de resources.
}

#include <array>

// to be able to reuse this for differnt types and different sizes, we need to templetize this!
template<typename T, size_t S>
class Array {
public:
    Array() {
        memset(data_, 0, S * sizeof(T));    //size is S x sizeof(T)
    }

    constexpr size_t
    Size() const { return S; }   // S is not actually storing Size, it just fils in S with the number given in the template
    // constexpr indicate size_t should be know at compile time!

    // needs operator [] to access data
    T &operator[](
            size_t index) { return data_[index]; } // by returning a reference, we are able to assign data_ using operator e.g. data[2]=12;
    const T &operator[](size_t index) const { return data_[index]; } // operator [] for const data

private:
    T data_[S];    // size needs to be specified at compile time.
};

TEST(cherno_arrays, creating_our_own_array) {
    // vector vs array. vector uses heap memory, while array uses stack memory.
    // array is much faster. Using arrays in the correct places, optimizes your application

    // heap array
    int size = 5;
    int *heapArray = new int[size]; // 1. we can do this runtime
    delete[] heapArray;   // 2. needs to be deleted

    // stack array
    int array[5];   // 1. size needs to be known at runtime
    // 2. does not need to be deleted. (goes out of scope)

    // c++11 uses templates
    std::array<int, 10> a;

    // we make our own:
    Array<int, 10> data;
    static_assert(data.Size() < 20, "Size is to large");    // static_assert assert on compile time variables.

    for (int i = 0; i < data.Size(); i++) {
        std::cout << data[i] << std::endl;  // read
        EXPECT_EQ(data[i], 0);
        data[i] = 123;                      // write
        EXPECT_EQ(data[i], 123);
    }

    const Array<int, 10> const_data;
    std::cout << const_data[1] << std::endl;   // needs its own operator tht returns const reference.
    // const_data[1] = 12;  // error: cannot assign to return value because function 'operator[]' returns a const value

    Array<std::string, 12> string_data;
    for (int i = 0; i < string_data.Size(); i++) {
        std::cout << string_data[i] << std::endl;       // read
        EXPECT_EQ(string_data[i], "");
        string_data[i] = "123";                         // write
        EXPECT_EQ(string_data[i], "123");
    }
}

//
template<typename Vector>
class VectorIterator {
public:
    using ValueType = typename Vector::ValueType;
    using PointerType = ValueType *;
    using ReferenceType = ValueType &;
public:
    VectorIterator(PointerType ptr) : ptr_(ptr) {};

    // increment operators
    VectorIterator &operator++() {
        ptr_++;
        return *this;
    }

    VectorIterator operator++(int) {
        VectorIterator iterator = *this;
        ++(*this);
        return iterator;
    }

    // decrement operators
    VectorIterator &operator--() {
        ptr_--;
        return *this;
    }

    VectorIterator operator--(int) {
        VectorIterator iterator = *this;
        --(*this);
        return iterator;
    }

    // index operator
    ReferenceType operator[](int index) {
        return *(ptr_ + index);
    }

    PointerType operator->() {
        return ptr_;
    }

    // dereferencing operator
    ReferenceType operator*() {
        return *ptr_;
    }

    // comparison operator
    bool operator ==(const VectorIterator& other) const {
        return ptr_ == other.ptr_;
    }
    bool operator !=(const VectorIterator& other) const {
        return !(*this == other);
    }

private:
    PointerType ptr_;
};

template<typename T, size_t S = 2>
class Vector {
public:
    using ValueType = T;
    using Iterator = VectorIterator<Vector<T>>;
public:
    Vector() {
        Resize(S);
    }

    ~Vector() {
        // NOTE! The delete-expression will invoke the destructor for the object or the elements of the array being deleted!!
        // can can lead to memory deleted twice if handled incorrectly.
        // More specific since clear() and emplaceback() are calling destructors in data_ manually,
        // we MUST not again call delete on data_
//        delete[] data_; // here we automatically call destructor for every object in our array
        ::operator delete(data_, current_size * sizeof(T));// operator delete does not call destructors
    }

    void PushBack(const T &t) {
        std::cout << __func__ << "copy" << std::endl;
        // check if within size
        if (current_size < allocated_size) {
            data_[current_size] = t;
            current_size++;
        } else {
            allocated_size = 2 * allocated_size;
            Resize(allocated_size);
            PushBack(t);
        }
    }

    void PushBack(T &&t) {
        std::cout << __func__ << "move" << std::endl;
        // check if within size
        if (current_size < allocated_size) {
            data_[current_size] = std::move(t);
            current_size++;
        } else {
            allocated_size = 2 * allocated_size;
            Resize(allocated_size);
            PushBack(std::move(t));
        }
    }

    void PopBack() {
        if (current_size > 0) {
            current_size--;
            data_[current_size].~T();   // here we manually call destructor for specific element
        }
    }

    void clear() {
        for (size_t i = 0; i < current_size; i++) {
            data_[i].~T();  // here we manually call destructor for specific element
        }
        current_size = 0;
    }

    template<typename... Args>
    T &EmplaceBack(Args &&... args) {
        if (current_size < allocated_size) {

            new(&data_[current_size]) T(std::forward<Args>(args)...); // creates directly in our data block
            current_size++;

            return data_[current_size];
        } else {
            allocated_size = 2 * allocated_size;
            Resize(allocated_size);
            return EmplaceBack(T(std::forward<Args>(args)...));
        }
    }

    T &operator[](size_t index) {
        return data_[index];
    }

    const T &operator[](size_t index) const {
        return data_[index];
    }

    size_t Size() const {
        return current_size;
    }

    Iterator begin() {
        return Iterator(data_);
    }

    Iterator end() {
        return Iterator(data_ + current_size);
    }

private:
    void Resize(size_t new_size) {
        std::cout << __func__ << ">>>>>>>>>>" << std::endl;
        // create new array with new_size
//        T* temp_data = new T[new_size]; // new array calls constructors of all object in this array
        T *temp_data = (T *) ::operator new(new_size * sizeof(T)); // operator new does NOT call constructor
        memset(temp_data, 0, new_size * sizeof(T));

        // copy/move the data
        for (size_t i = 0; i < current_size; i++) {
            temp_data[i] = std::move(data_[i]);
        }

        // we need to manuall cal each destructor now since we are not using delete[] on data anymore
        for (size_t i = 0; i < current_size; i++) {
            data_[i].~T();  // here we manually call destructor for specific element
        }

        // delete the old data
//        delete[] data_;// delete array calls destructors of all object in this array
        ::operator delete(data_, current_size * sizeof(T));// operator delete does not call destructors

        // rewire the pointers and update administration
        allocated_size = new_size;
        data_ = temp_data;
        temp_data = nullptr;
    }

    size_t allocated_size = S;
    size_t current_size = 0;
    T *data_ = nullptr;
};

template<typename T>
void PrintVector(const Vector<T> &vector) {
    for (size_t i = 0; i < vector.Size(); i++) {
        std::cout << vector[i] << std::endl;
    }
    std::cout << "----------------------" << std::endl;
}

TEST(cherno_vector, creating_our_own_vector) {
    // vector vs array. vector uses heap memory, while array uses stack memory.
    // vectors size is dynamic, and can be changed in runtime.
    // if you do not need heap allocation, do not use them, they will slow down your program.

    Vector<std::string> vector;
    EXPECT_EQ(vector.Size(), 0);
    vector.PushBack("Hello");
    vector.PushBack("World");

    EXPECT_EQ(vector[0], "Hello");
    EXPECT_EQ(vector[1], "World");
    EXPECT_EQ(vector.Size(), 2);

    vector.PushBack("Kitty");
    EXPECT_EQ(vector[0], "Hello");
    EXPECT_EQ(vector[1], "World");
    EXPECT_EQ(vector[2], "Kitty");
    EXPECT_EQ(vector.Size(), 3);
    PrintVector(vector);
}

struct Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    // we make Vector3 more complex by adding a memory block
    int *memoryBlock_ = nullptr;

    Vector3() {
        memoryBlock_ = new int[5];      // we added a memoryblock that we need to deallocate
    }

    Vector3(float scalar)
            : x(scalar), y(scalar), z(scalar) {
        memoryBlock_ = new int[5];
    }

    Vector3(float x, float y, float z)
            : x(x), y(y), z(z) {
        memoryBlock_ = new int[5];
    }

    Vector3(const Vector3 &other)
            : x(other.x), y(other.y), z(other.z) {
        memcpy(memoryBlock_, other.memoryBlock_, 5);
        std::cout << ">>>" << "copy constructor" << std::endl;
    }

    Vector3 &operator=(const Vector3 &other) {
        x = other.x;
        y = other.y;
        z = other.z;
        memcpy(memoryBlock_, other.memoryBlock_, 5);
        std::cout << ">>>" << "copy assign" << std::endl;
        return *this;
    }

    Vector3 &operator=(Vector3 &&other) {
        // check if same object
        if (this != &other) {
            // clean our own data first
            delete[] memoryBlock_;

            // steal data by rewiring pointer
            x = std::move(other.x);
            y = std::move(other.y);
            z = std::move(other.z);
            memoryBlock_ = other.memoryBlock_;

            // take care of other
            other.memoryBlock_ = nullptr;

            std::cout << ">>>" << "move assign" << std::endl;
        }
        return *this;
    }

    Vector3(Vector3 &&other)
            : x(std::move(other.x)), y(std::move(other.y)), z(std::move(other.z)) {
        memoryBlock_ = other.memoryBlock_;
        other.memoryBlock_ = nullptr;
        std::cout << ">>>" << "move constructor" << std::endl;
    }

    ~Vector3() {
        delete[] memoryBlock_;
        std::cout << ">>>" << "destroy" << std::endl;
    }

};

#include <chrono>

class Timer {
public:
    Timer() {
        startTimepoint_ = std::chrono::high_resolution_clock::now();
    }

    ~Timer() {
        Stop();
    }

    void Stop() {
        auto endTimepoint = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(
                startTimepoint_).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
        auto duration = end - start;
        double ms = duration * 0.001;

        std::cout << ms << " ms" << std::endl;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTimepoint_;
};


TEST(cherno_vector, creating_our_own_vector2) {
    // vector vs array. vector uses heap memory, while array uses stack memory.
    // vectors size is dynamic, and can be changed in runtime.
    // if you do not need heap allocation, do not use them, they will slow down your program.

    {   // copying, if Pushback argument is lvalue, I copy
        Timer t;
        Vector<Vector3> vector;
        EXPECT_EQ(vector.Size(), 0);
        auto v1 = Vector3{1, 2, 3};
        vector.PushBack(v1);
        auto v2 = Vector3{4};
        vector.PushBack(v2);
        EXPECT_EQ(vector.Size(), 2);
        auto v3 = Vector3{1, 2, 3};
        vector.PushBack(v3);
        auto v4 = Vector3{4};
        vector.PushBack(v4);
        EXPECT_EQ(vector.Size(), 4);
    }

    {   // move, If Pushback argument is rvalue I copy
        Timer t;
        Vector<Vector3> vector;
        EXPECT_EQ(vector.Size(), 0);
        vector.PushBack(Vector3{1, 2, 3});
        vector.PushBack(Vector3{4});
        EXPECT_EQ(vector.Size(), 2);
        vector.PushBack(Vector3{1, 2, 3});
        vector.PushBack(Vector3{4});
        EXPECT_EQ(vector.Size(), 4);
    }

    {   // emplace back: instead of creating a vector3, just give me the arguments for me
        // and I will do the construction emplace in the datablock.
        Timer t;
        Vector<Vector3> vector;
        EXPECT_EQ(vector.Size(), 0);
        vector.EmplaceBack(1, 2, 3);
        vector.EmplaceBack(4);
        EXPECT_EQ(vector.Size(), 2);
        vector.EmplaceBack(1, 2, 3);
        vector.EmplaceBack(4);
        EXPECT_EQ(vector.Size(), 4);
    }
}

template<>
void PrintVector(const Vector<Vector3> &vector) {
    for (size_t i = 0; i < vector.Size(); i++) {
        std::cout << vector[i].x << "," << vector[i].y << "," << vector[i].z << std::endl;
    }
    std::cout << "----------------------" << std::endl;
}

TEST(cherno_vector, emplaceback) {
    {   // emplace back: instead of creating a vector3, just give me the arguments for me
        // and I will do the construction emplace in the datablock.
        Timer t;
        Vector<Vector3> vector;
        vector.EmplaceBack(1, 2, 3);
        vector.EmplaceBack(4);
        vector.EmplaceBack(1, 2, 3);
        vector.EmplaceBack(4);
        PrintVector(vector);
        vector.PopBack();
        vector.PopBack();
        PrintVector(vector);
        vector.clear();
        PrintVector(vector);
        vector.EmplaceBack(1, 2, 3);
        vector.EmplaceBack(4);
        PrintVector(vector);
    }
}

#include <unordered_map>

TEST(cherno_iterator, iterator) {
    // iterators are uses to travers through a dataset.
    // for vectors it seems trivial, because you can just use a for loop
    // however other data structure, like sets does not keep data in order

    std::vector<int> value = {1, 2, 3, 4, 5};
    for (int i = 0; i < value.size(); i++) {
        std::cout << value[i] << std::endl;
    }

    // cleaner way using range based for loop, this works because vector provides a
    // begin() and end() function, that returns back an iterator at a certian position.
    //
    for (int i: value) {
        std::cout << i << std::endl;
    }

    //
    for (std::vector<int>::iterator it = value.begin(); it != value.end(); it++) {
        // need to dereference iterator
        std::cout << *it << std::endl;
        // there are a few resons wy you want to use this instead of range based for loop:
        // this is when you want to manipulate the posistion of the iterator.
        // 1. you want to erase an element, while still keep iterating though the rest of the collection
        // 2. you want to insert an element somewhere in the middle.
    }

    // Vectors are easy, they exist continuously in memory.
    // we can just use forloop with an increasing index.
    // Other datasets are hard e.g. unordered map -> this is a hashmap,
    // We need to know the keys if we want to get the data out of there,
    // unless we can iterator over them!


    std::unordered_map<std::string, int> map;
    map["kevin"] = 5;
    map["c++"] = 2;

    for (std::unordered_map<std::string, int>::const_iterator it = map.begin(); it != map.end(); it++) {
        auto &key = it->first;
        auto &value = it->second;
        std::cout << "key:" << key << " value:" << value << std::endl;
    }

    // much cleaner!
    for (auto kv: map) {
        auto &key = kv.first;
        auto &value = kv.second;
        std::cout << "key:" << key << " value:" << value << std::endl;
    }

    // even cleaner! but only in c++17
//    for (auto [key, value]: map) {
//        std::cout << "key:" << key << " value:" << value << std::endl;
//    }
}

TEST(cherno_iterator, custom_iterator) {
    Vector<std::string> value;
    value.EmplaceBack("1");
    value.EmplaceBack("2");
    value.EmplaceBack("3");
    value.EmplaceBack("4");
    value.EmplaceBack("5");

    std::cout << "not using iterators" << std::endl;
    for (int i = 0; i < value.Size(); i++) {
        std::cout << value[i] << std::endl;
    }

    std::cout << "range-base for loop" << std::endl;
    for (auto& i: value) {
        std::cout << i << std::endl;
    }

    std::cout << "iterator" << std::endl;
    for (auto it = value.begin(); it != value.end(); it++) {
        std::cout << *it << std::endl;
    }

}
