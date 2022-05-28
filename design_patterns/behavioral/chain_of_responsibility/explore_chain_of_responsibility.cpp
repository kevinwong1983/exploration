#include "boost/variant.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/system_error.hpp>
#include <functional>
#include <unordered_map>
#include <stack>

// Chain of responsibility
// A chain of components who all get a chance to process a command or query, optionally having a default
// processing implementation and an ability to terminate the processing chain.

// examples
// 1. you click on a graphical element on a form
// - button handles it, stop further processing
// - click might go further in underlying group box
// - click might go further in underlying window
// 2. collectible card game - computer game
// - creature has attack and defense values
// - those values can be boosted by other cards

// Command Query Separation
// Command = asking for an action or change (e.g. please set your attack value to 2)
// Query = asking for information (e.g. please give me your attack value)
// CQS = having separate means of sensing commands and queries: (antithetical to e.g. direct field access)

struct Creature {
    std::string name;
    int attack, defence;

    Creature(const std::string &name, const int attack, const int defence)
            : name{name}, attack{attack}, defence{defence} {
    }

    friend std::ostream &operator<<(std::ostream &os, const Creature &obj) {
        return os << "name:" << obj.name
                  << " attack:" << obj.attack
                  << " defence:" << obj.defence;
    }
};

class CreatureModifier {
    CreatureModifier* next = nullptr; // because its a chain, we have a next modifier
protected:
    Creature& creature;

public:
    explicit CreatureModifier(Creature& creature)
    : creature(creature) {
    }
    virtual ~CreatureModifier() = default;

    void add(CreatureModifier* cm) {
        if (next) {
            next->add(cm);
        } else {
            next = cm;
        }
    }

    virtual void handle() {
        if (next) {
            next -> handle();
        }
    }
};

// Modifiers that we want to have:
// 1. Double the creature's attack/
// 2. Increase defence by 1 unless power > 2.
// 3. No bonuses can be applied to this creature.

// 1
class DoubleAttackModifier : public CreatureModifier {
public:
    explicit DoubleAttackModifier(Creature& creature)
    : CreatureModifier(creature) {
    }

    void handle() override {
        creature.attack *= 2;
        CreatureModifier::handle(); // invoke next in chain
    }
};

// 2
class IncreaseDefenseModifier : public CreatureModifier {
public:
    explicit IncreaseDefenseModifier(Creature& creature)
    : CreatureModifier(creature) {
    }

    void handle() override {
        if (creature.attack <= 2){
            creature.defence ++;
        }
        CreatureModifier::handle(); // invoke next in chain
    }
};

// 3
class NoBonusModifier : public CreatureModifier {
public:
    explicit NoBonusModifier(Creature& creature)
    : CreatureModifier(creature){
    }

    void handle() override {
        // because this is empty, all other modifiers after this one will not be invoked.
    }
};

TEST(chian_of_responsibility, pointer_chain) {
    Creature goblin {"Goblin", 1,1};
    std::cout << goblin << std::endl;
    // now we want to beable to use cards to make the gobins attack more powerfull;
    // how can we do this in a oo fashion.

    CreatureModifier root {goblin};
    DoubleAttackModifier r1{goblin};
    DoubleAttackModifier r1_2{goblin};
    IncreaseDefenseModifier r2 {goblin};
//    NoBonusModifier no{goblin};

//    root.add(&no);
    root.add(&r1);
    root.add(&r1_2);
    root.add(&r2);
    root.handle();
    // we now have build a chain, we can now take a goblin and modify its attack and defence
    std::cout << goblin << std::endl;
}

// we typically want the modify the goblin temporary while the attack modifier exist. When it is out of scope you want
// it to stop changing the goblin. Our modifiers change them permanently. What we want is that something query the attack value
// and then allow the modifier to grab that query and modify it with additional bonuses. We can implement this with a
// centralised component: broker

#include <boost/signals2.hpp>
using namespace boost::signals2;

struct Query {
    std::string creature_name;
    enum Argument {attack, defense} argument;
    int result;

    Query (const std::string& creature_name, const Argument argument, const int result)
    : creature_name{creature_name}, argument{argument}, result {result}{
    }
};

struct Game{
    boost::signals2::signal<void(Query&)> queries;
};

struct Creature2 {
    Game& game;
    int attack, defense;
public:
    std::string name;

    Creature2(Game& game, const int attack, const int defense,  const std::string& name)
    : game(game), attack(attack), defense(defense), name(name){
    }

    int GetAttack () const {
        // how are we going to get not only the underlying goblins attack value.
        // but also the many bonuses gets added to it? The answer is, we are going
        // to do this by queries.
        Query q {name, Query::Argument::attack, attack};
        std::cout << __func__ << " 1 q.result = " << q.result << std::endl;
        game.queries(q);
        std::cout << __func__ << " 2 q.result = " << q.result << std::endl;
        return q.result;
    }

    friend std::ostream& operator<< (std::ostream& os, const Creature2& obj) {
         return os << " attack: " << obj.GetAttack() // HERE WE ARE USING GetAttack()
         << " defense: " << obj.defense
         << " name: " << obj.name;
    }
};

class CreatureModifier2 {
    Game& game;
    Creature2& creature;
public:
    CreatureModifier2(Game& game, Creature2& creature)
    : game {game},
    creature{creature} {
    }

    virtual ~CreatureModifier2 () = default;
};

class DoubleAttackModifier2 : public CreatureModifier2 {
    connection conn;
public:
    DoubleAttackModifier2(Game& game, Creature2& creature)
    : CreatureModifier2(game, creature){
        conn = game.queries.connect([&](Query& q){
            if (q.creature_name == creature.name &&
            q.argument == Query::Argument::attack) {
                q.result *= 2;
            }
            std::cout << __func__ << " q.result = " << q.result << std::endl;
        });
    }

    virtual ~DoubleAttackModifier2(){
        conn.disconnect();  // disconnects the signal when going out of scope.
    }
};

TEST(chian_of_responsibility, broker_chain) {
    Game game;
    Creature2 goblin {game, 2,2, "Strong Goblin"};
    std::cout << goblin << std::endl;

    {
        DoubleAttackModifier2 dam {game, goblin};
        std::cout << goblin << std::endl;
    }


    // when DoubleAttackModifier2 is out scope, it back to the old situation
    std::cout << goblin << std::endl;
}

// Summary
// Chain of responsibility can be implemented as a pointer chain or a centralized construct
// Enlist objects in the chain, possibly controlling their order
// Remove objects from chain when no longer applicable (e.g. in its own destructor)
