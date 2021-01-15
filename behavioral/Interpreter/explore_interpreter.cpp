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

// Interpreter
// A component that processes structured text data. Does so by turning it into separate lexical tokens (lexing) and
// then interpreting sequences of said tokens (parsing).

// Motivation
// Textual input needs to be processed, e.g. a programming language compilers, HTML/XML, regex, numerical expressions

struct Token {
    enum Type {
        integer, plus, minus, lparen, rparen
    } type;
    std::string text;

    Token(const Type type, const std::string &text)
            : type(type),
              text(text) {
    }

    friend std::ostream &operator<<(std::ostream &os, const Token &obj) {
        return os << "'" << obj.text << "'";
    }
};

struct Element {
    virtual ~Element() = default;

    virtual int eval() const = 0;
};

struct Integer : Element {
    int value;

    explicit Integer(const int value)
            : value(value) {
    }

    int eval() const override {
        return value;
    }
};

struct BinaryOperation : Element {
    enum Type {
        addition, substraction
    } type;
    std::shared_ptr<Element> lhs, rhs;

    int eval() const override {
        if (type == addition) {
            return lhs->eval() + rhs->eval();
        }
        return lhs->eval() - rhs->eval();
    }
};

std::shared_ptr<Element> parse(const std::vector<Token> &tokens) {
    auto result = std::make_shared<BinaryOperation>();
    bool have_lhs = false;

    for (int i = 0; i < tokens.size(); i++) {
        auto token = tokens[i];

        switch (token.type) {
            case Token::integer: {
                int value = boost::lexical_cast<int>(token.text);
                auto integer = std::make_shared<Integer>(value);
                if (!have_lhs) {
                    result->lhs = integer;
                    have_lhs = true;
                } else {
                    result->rhs = integer;
                }
            }
                break;
            case Token::plus:
                result->type = BinaryOperation::addition;
                break;
            case Token::minus:
                result->type = BinaryOperation::substraction;
                break;
            case Token::lparen: // here we want ot have all elements from the left parentesis until the right parentesis and evaluate them separately.
            {
                int j = i;
                for (; j < tokens.size(); j++) {
                    if (tokens[j].type == Token::rparen) {   // if I have a right parentisis I am just going to break.
                        break;
                    }
                }

                // we get a vector of the subexpressions and parse it.
                std::vector<Token> subexpression(&tokens[i + 1], &tokens[j]);
                auto element = parse(subexpression);

                if (!have_lhs) {
                    result->lhs = element;
                    have_lhs = true;
                } else {
                    result->rhs = element;
                }
                i = j;
            }
                break;
            default:
                break;
        }
    }
    return result;
}

std::vector<Token> lex(const std::string &input) {
    std::vector<Token> result;

    for (int i = 0; i < input.length(); i++) {
        switch (input[i]) {
            case '+':
                result.push_back(Token{Token::plus, "+"});
                break;
            case '-':
                result.push_back(Token{Token::minus, "-"});
                break;
            case '(':
                result.push_back(Token{Token::lparen, "("});
                break;
            case ')':
                result.push_back(Token{Token::rparen, ")"});
                break;
            default:
                std::ostringstream buffer;
                buffer << input[i];
                for (int j = i + 1; j < input.size(); j++) {
                    if (std::isdigit(input[j])) {
                        buffer << input[j];
                        i++;
                    } else {
                        result.push_back(Token{Token::integer, buffer.str()});
                        break;
                    }
                }
        }
    }

    return result;
}

TEST(interpreters, handmade_interpreter) {
    std::string input{"(13-4)-(12+1)"};
    // for interpreting this string, we devide into two phases 1. lexing, 2. parsing.
    // 1. lexing: breaking up the string into different tokens.

    auto tokens = lex(input);
    for (auto token: tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;

    // 2. parsing turns tokens into meaningfull constructs
    auto parsed = parse(tokens);
    std::cout << input << "=" << parsed->eval();
}

