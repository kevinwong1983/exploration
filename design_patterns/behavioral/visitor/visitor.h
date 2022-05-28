#pragma once
#include <string>

namespace visitor {

struct Paragraph;   // forward declare.
struct ListItems;
struct List;

struct Visitor {
    virtual ~Visitor() = default;

    virtual void visit(const Paragraph &p) = 0;
    virtual void visit(const ListItems &p) = 0;
    virtual void visit(const List &p) = 0;

    virtual std::string str() const = 0;
};

} //using namespace std;