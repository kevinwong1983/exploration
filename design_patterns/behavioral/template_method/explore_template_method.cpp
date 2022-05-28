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
#include <boost/any.hpp>
#include <mutex>
#include <boost/signals2.hpp>
#include <map>
#include <iostream>
#include <sstream>
using namespace std;

// Template Method
// Allows us to define the "skeleton"of the algorithm, with concreate implementations
// defined in subclasses (instead of using members). So similar to strategy with a different
// approach.


// Motivation
// Algorithms can be decomposed into common parts + specifics
// Strategy pattern does this htrough composition
// - High level algorithms uses an interface
// - Concrete implementations implement the interface
// Template Method does the dame thing, but through inheritance.

namespace template_method {
class Game {
public:
    Game(int number_of_players)
        : number_of_players(number_of_players) {}

    // High level algorithm to play any kind of game
    void run () {
        start();
        while (!have_winner()){
            take_turn();
        }
        std::cout << "Player" << get_winner() << " wins." << std::endl;
    }
protected:
    // only exposed to its subclasses
    virtual void start() = 0;
    virtual bool have_winner() = 0;
    virtual void take_turn() = 0;
    virtual int get_winner() = 0;

    int current_player{0};
    int number_of_players;
};

class Chess: public Game {
public:
    explicit Chess() : Game{2} {
    }

protected:
    void start() override {
        std::cout << "starting a game of chess with "
            << number_of_players << " winner." << std::endl;
    }
    bool have_winner() override {
        return turns == max_turns;
    }
    void take_turn() override {
        std::cout << "Turn "<< turns << " taken by player " << current_player << std::endl;
        turns ++;
        current_player = (current_player +1) % number_of_players;
    }
    int get_winner() override {
        return current_player;
    }
private:
    int turns{0};
    int max_turns{10};
};

TEST(template, dynamic) {
    Chess chess;
    chess.run();
}

// summary:
// define an algorithms at a high level
// define constituent  parts as pure virtual functions
// Inherit the alogorithm class, providing nessesart function implementations.

} // namespace template
