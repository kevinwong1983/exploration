#pragma once
#include <string>
#include <vector>
#include "visitor.h"
//struct Visitor; // forwared declare.

namespace visitor {

struct Element {
    virtual ~Element() = default;
    virtual void print_bad_example(std::ostringstream& oss) const = 0;  // pure virtual
    // problem here is that for each new print type e.g. Markdown, we need to create another print (print_markdown) here.
    // we now need to modify the while hierarchy chain adding this new print_markdown function.
    // this violated the open-close principle.
    // we want separate components handling the printing.

    // double dispatch
    virtual void accept(Visitor& v) const = 0;
    // accept function can be called on a specific element. i.e. the object proxy's over to the visitor
    // visitor e.g. Markdown visitor, Html visitor, Latex visitor etc...
};

struct TextElement : Element {
    std::string text;

    explicit TextElement(const std::string &text)
            : text(text) {
    }
};

struct Paragraph : TextElement {
    explicit Paragraph(const std::string &text)
            : TextElement(text) {
    }
    void print_bad_example(std::ostringstream& oss) const override {
        oss<< "<p>" << text << "</p>" << std::endl;
    }

    void accept(Visitor& v) const override {
        v.visit(*this);
    }
};

struct ListItems : TextElement {
    explicit ListItems(const std::string &text)
            : TextElement(text) {
    }
    void print_bad_example(std::ostringstream& oss) const override {
        oss<< "<li>" << text << "</li>" << std::endl;
    }

    void accept(Visitor& v) const override {
        v.visit(*this);
    }
};

struct List : std::vector<ListItems>, Element {
    List(const ::std::initializer_list<value_type> &_Ilist)
            : std::vector<ListItems>(_Ilist) {
    }
    void print_bad_example(std::ostringstream& oss) const override {
        oss << "<ul>" << std::endl;
        for (auto li : *this){
           li.print_bad_example(oss);
        }
        oss << "</ul>" << std::endl;
    }

    void accept(Visitor& v) const override {
        v.visit(*this);
    }
};

} // namespace visitor