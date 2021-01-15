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

#include "ChatRoom.h"
#include "Person.h"

// Mediator
// A component that facilitates communication between other components withous them being aware of each other or having
// direct (referential) access to each other. e.g. if component A wants to talk to component B it does not do so directly,
// instead it goes through the mediator.

// Motivation
// Components may go in and out of a system at any time e.g. Chat room participants, players in a MMORPG.
// It makes no sense for them to have direct references to one another. These references may go dead when they leave.
// Solution: have them all refer to sine central component that facilitates communication.

using namespace mediator;

TEST(iterator, chat_room) {
    ChatRoom room;
    auto john = room.join(Person{"john"});
    auto jane = room.join(Person{"jane"});

    john->say("hi room");
    jane->say("hi john");

    auto simon = room.join(Person{"simon"});
    simon->say("hi everyone!");

    jane->pm("simon", "glad you could join us, simon");
}

#include <boost/signals2.hpp>

using namespace boost::signals2;

struct Event {
    virtual ~Event() = default;

    virtual void print() const = 0;
};

struct Player;

struct PlayerScored : Event {
    std::string player_name;
    int goals_scored_so_far;

    PlayerScored(const std::string &player_name, const int goals_scored_so_far)
            : player_name(player_name),
              goals_scored_so_far(goals_scored_so_far) {
    }

    void print() const override {
        std::cout << player_name << "has scored! (their "
                  << goals_scored_so_far << " goal)" << std::endl;
    }
};

struct Game {   // formal name: event bus/broker
    boost::signals2::signal<void(Event *)> events; // observer
};

struct Player {
    std::string name;
    int goals_scored = 0;
    Game &game;

    Player(const std::string &name, Game &game)
            : name(name),
              game(game) {
    }

    void score() {
        goals_scored++;
        PlayerScored ps{name, goals_scored};
        game.events(&ps);
    }
};

struct Coach {
    Game& game;

    explicit Coach(Game& game)
    : game(game) {
        game.events.connect([](Event* e) {
            PlayerScored* ps = dynamic_cast<PlayerScored*>(e); // note: if I used a reference here, I needed to use a try-catch because it then
                                                                // assumes I would never get a null reference in c++ (thus a bit harder to handle).
            if (ps && ps->goals_scored_so_far < 3) {
                std::cout << "coach says: well done, " << ps->player_name << std::endl;
            }
        });
    }
};

TEST(iterator, event_broker) {
    Game game;
    Player player{"Kevin", game};
    Coach coach{game};

    player.score();
    player.score();
    player.score();

    // we can make it a bit more sofisticated by giving up manual processing of events
    // and using stream processing libraries like the reactive extentions for c++;
}

// summary
// - Create a mediator and have each object in the system to refer to it (e.g. reference field).
// You have to have an constructor that initialize the reference field. Also very usefull to get people to
// use the api that you initially specify the connected objects. e.g. if you have a mediator game, you need
// the clients should have a constructor that takes in game.
// - Mediator give bi-directional communication to clients that are connected to it.
// - Libraries: Signal/Slot (boost.signals) and event processing (RxCpp) libraries make communication easier to implement.
