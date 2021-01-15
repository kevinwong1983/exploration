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
// a. begin() points to first element in the containter, if empty, is qual to end();
// b. end() ppints to the element immediatly after the last element.
// c. facilitates use of standard algorithms
// d. allow the use of range-based for loop e.g. for (auto& x: my_container)
// e. different names for different iterators

// 2. Iterators operators
// a. operator != must return false if two iterators point to the same element.
// b. operator * (dereferencing) must return a reference to (or a copy of) the dtat the iterator points to.
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
            : value(value),
              left(left),
              right(right) {
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
struct BinaryTree {
    Node<T> *root = nullptr;

    template<typename U>
    struct BinaryTreeIterator {
        Node<U> *current;

        explicit BinaryTreeIterator(Node<U> *const current)
                : current(current) {
        }

        bool operator!=(const BinaryTreeIterator<U> &other) {
            return current != other.current;
        }

        Node<U> &operator*() {
            return *current;
        }

        BinaryTreeIterator<U> &operator++() {
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

// for non trivial datastructure we can make use of the boost interator facade:

struct Node2 {
    std::string value;
    Node2* next = nullptr;

    explicit Node2(const std::string& value)
    : value(value) {
    }

    Node2(const std::string& value, Node2* const parent)
            : value(value) {
        parent->next = this;
    }
};

struct ListIterator : boost::iterator_facade<ListIterator, Node2, boost::forward_traversal_tag> {
    Node2* current = nullptr;

    ListIterator(){
    }

    explicit ListIterator(Node2* const current)
    : current(current) {
    }

private:
    friend class boost::iterator_core_access;

    void increment() {
        current = current->next;
    }

    bool equal(const ListIterator& other) const {
        return other.current == current;
    }

    Node2& dereference() const {
        return *current;
    }
};

TEST(iterator, boost_iterator_facade) {
    Node2 alpha("alpha");
    Node2 beta("bata");
    Node2 gamma("gamma");

    std::for_each(ListIterator{&alpha}, ListIterator{}, [&](const Node2& n){
        std::cout << n.value << std::endl;
    });
}

// voordeel van de boost iterator fade is dat je niet meer de moeilijk leesbare template nodig hebt.
// nadeel, configuratie is lastig.
