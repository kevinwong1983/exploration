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

// Iterator
// An object that facilitates the traversal of a data structure. It incorporates the rules of how you want to travers
// the data structure, by having some intimate knowledge of the data structure and how to go around it.

// motivation
// iteration (traversal) is a core functinality of varisous datastructures.

// An iterator is a class that facilitates the traversal
// - Keeps pointer to an element
// - Knows how to move to a different element.

// Iterator types"
// Forward (e.g. on a list)
// Bidirectional (e.g. on a doubly linked list)
// Random Access (e.g. on a vector)

TEST(iterator, std_library) {
    // iterators
    std::vector<std::string> names{"john", "jane", "jill", "jack"};
    std::vector<std::string>::iterator it = names.begin(); // same as begin(names)
    std::cout << "first name is" << *it << std::endl;

    ++it;
    it->append(" goodall");
    std::cout << "second name is " << *it << std::endl;

    while (++it != names.end()) {   // end() gives pointer after the last element!
        std::cout << "another name:" << *it << std::endl;
    }

    // reverse iterators
    for (auto ri = rbegin(names); ri != rend(names); ri++) { // here ++ is overloaded to give the "previous" element
        std::cout << *ri;
        if (ri + 1 != rend(names)) {
            std::cout << ", ";
        }
    }

    // constant iterators: you are not allowed to change the element the iterator is pointing to
    std::vector<std::string>::const_reverse_iterator jack = crbegin(names);
    //*jack != " reacher"; connot do this...

    // the range based for loop also uses the begin() and end() mechanics.
    // so if you class supports the begin and end functions, you are able to use the range based forloop.
}

// Iterator Requirements
// 1. Container member functions
// a. begin() points to first element in the container, if empty, is equal to end();
// b. end() points to the element immediately after the last element.

// 2. Iterators operators
// a. operator != must return false if two iterators point to the same element.
// b. operator * (dereferencing) must return a reference to (or a copy of) the data the iterator points to.
// c. operators ++ gets the iterator to point to the next element.
// d. Additional operators as required (e.g. operator --, arithmetic, etc)

template<typename T>
struct BinaryTree;

template<typename T>
struct Node {
    T value = T();
    Node<T> *left = nullptr;
    Node<T> *right = nullptr;
    Node<T> *parent = nullptr;
    BinaryTree<T> *tree = nullptr;

    explicit Node(const T &value)
        : value(value) {
    }

    ~Node() {
        if (left) {
            delete left;
        }
        if (right) {
            delete right;
        }
    }

    Node(const T &value, Node<T> *const left, Node<T> *const right)
        : value(value), left(left), right(right) {
        this->left->tree = this->right->tree = tree;
        this->left->parent = this->right->parent = this;
    }

    void set_tree(BinaryTree<T> *t) {
        tree = t;
        if (left) {
            left->set_tree(t);
        }
        if (right) {
            right->set_tree(t);
        }
    }
};

template<typename T>
struct BinaryTreeIterator {
    Node<T> *current;

    explicit BinaryTreeIterator(Node<T> *const current)
            : current(current) {
    }

    bool operator!=(const BinaryTreeIterator<T> &other) {
        return current != other.current;
    }

    Node<T> &operator*() {
        return *current;
    }

    BinaryTreeIterator<T> &operator++() {
        if (current->right) {
            current = current->right;
            while (current->left) {
                current = current->left;
            }
        } else {
            Node<T> *p = current->parent;
            while (p && current == p->right) {
                current = p;
                p = p->parent;
            }
            current = p;
        }
        return *this;
    }
};

template<typename T>
struct BinaryTree {
    Node<T> *root = nullptr;

    typedef BinaryTreeIterator<T> iterator;

    explicit BinaryTree(Node<T> *const root)
            : root(root) {
        root->set_tree(this);
    }

    ~BinaryTree() {
        if (root) {
            delete root;
        }
    }

    iterator end() {
        return iterator{nullptr};
    }

    iterator begin() {
        Node<T> *n = root;
        if (n) {
            while (n->left) {
                n = n->left;
            }
        }
        return iterator{n};
    }
};

TEST(iterator, tree) {
    BinaryTree<std::string> family{
        new Node<std::string>{"me",
            new Node<std::string>{"mother",
                new Node<std::string>{"grandma"},
                new Node<std::string>{"grandpa"}},
            new Node<std::string>{"father"}
        }
    };

    for (auto it = family.begin(); it != family.end(); ++it) {
        std::cout << (*it).value << std::endl;
    }
}

// for non trivial data structure we can make use of the boost interator facade:

struct Item {
    std::string value;
    Item *next = nullptr;
    Item *previous = nullptr;

    explicit Item(const std::string &value)
            : value(value) {
    }
};

struct PancakeHouseIterator : boost::iterator_facade<PancakeHouseIterator, Item, boost::forward_traversal_tag> {
    Item *current = nullptr;

    PancakeHouseIterator() = default;

    explicit PancakeHouseIterator(Item *const current)
            : current(current) {
    }

private:
    friend class boost::iterator_core_access;

    void increment() {
        current = current->next;
    }

    bool equal(const PancakeHouseIterator &other) const {
        return other.current == current;
    }

    Item &dereference() const {
        return *current;
    }
};

template<typename T>
struct Menu {
    virtual void PushBack(T *const n) = 0;

    virtual void PushFront(T *const n) = 0;

    virtual PancakeHouseIterator begin() = 0;

    virtual PancakeHouseIterator end() = 0;;
};

struct PancakeCakeHouseMenu : public Menu<Item> {
    Item *index_;

    PancakeCakeHouseMenu(Item *const n)
            : index_(n) {
    }

    void PushBack(Item *const n) {
        auto lastNode = last();
        lastNode->next = n;
        n->previous = lastNode;
    }

    void PushFront(Item *const n) {
        auto firstNode = first();
        firstNode->previous = n;
        n->next = firstNode;
    }

    PancakeHouseIterator begin() {
        return PancakeHouseIterator(first());
    }

    PancakeHouseIterator end() {
        return PancakeHouseIterator();
    }

private:
    Item *last() {
        Item *r = index_;
        while (r->next != nullptr) {
            r = r->next;
        }
        return r;
    }

    Item *first() {
        Item *r = index_;
        while (r->previous != nullptr) {
            r = r->previous;
        }
        return r;
    }
};

TEST(iterator, boost_iterator_facade) {
    Item fruit("Fruit Pancake");
    Item cheese("Cheese Pancake");
    Item bacon("Bacon Pancake");

    auto l = PancakeCakeHouseMenu(&fruit);
    l.PushBack(&cheese);
    l.PushBack(&bacon);

    std::for_each(l.begin(), l.end(), [&](const Item &n) {
        std::cout << n.value << std::endl;
    });
}

// voordeel van de boost iterator fade is dat je niet meer de moeilijk leesbare template nodig hebt.
// nadeel, configuratie is lastig.

