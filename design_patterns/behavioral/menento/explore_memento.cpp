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

// Memento
// A token/handle representing the system state. Lets us roll back to the state when the token was generated.
// May of may not directly expose state information.

// Motivation
// An object or system goes trhough changes
// - e.g. a bank account gets deposts and witdraw
// There are different wasy of navigating thos changes.
// One ways is to record every change (Command pattern) and teach a command to 'undo' itself
// Another is to simply save snapshots of the system. (this is what the memento pattern is about)

namespace memento {

class Memento {
    int balance; // when system has more that just a integer field, you might want to serialize it into a e.g. database.
public:
    Memento(const int balance)
    : balance(balance) {
    }
    friend class BankAccount; // we only need access to balance to within bank account, we do not need to expose balance to user.
    friend class BankAccount2;

};

class BankAccount {
    int balance = 0;

public:
    explicit BankAccount(const int balance)
    : balance(balance) {
    }

    void restore(const Memento& m) {
        balance = m.balance;
    }
    Memento deposit(int amount) {
        // we want to give this system a way to make a snapshot of the system of the current state.
        // bank account only have one state variable: balance.
        balance += amount;
        return {balance}; // implicitly returns back a Memento.
    }

    friend std::ostream& operator<<(std::ostream& os, const BankAccount& obj) {
        return os << "balance: " << obj.balance;
    }
};

TEST(memento, simple_bank_account_memento) {
    BankAccount ba{100};
    auto m1 = ba.deposit(50); // 150;
    auto m2 = ba.deposit(25); // 175;
    std::cout << ba << std::endl;

    // undo to m1.
    ba.restore(m1);
    std::cout << "undo to m1: " << ba << std::endl;

    // undo to m2.
    ba.restore(m2);
    std::cout <<  "undo to m2: " << ba << std::endl;
}

// To do undo/redo we need to store every memento that there is. This means we are going to store every single snapshot
// of a deposit function.
class BankAccount2 {
    int balance = 0;
    std::vector<std::shared_ptr<Memento>> changes;
    int current; // position/index of the current "change" that we are currently in. i.e. which memento are we operating in right now.
public:
    explicit BankAccount2(const int balance)
            : balance(balance) {
        changes.emplace_back(std::make_shared<Memento>(balance));
        current = 0;
    }

    void restore(std::shared_ptr<Memento>& m) {
        if (m) {
            balance = m->balance;
            changes.push_back(m); // we also store the memento here.
            current = changes.size()-1;
        }
    }

    std::shared_ptr<Memento> deposit(int amount) {
        balance +=amount;
        auto m = std::make_shared<Memento>(balance);
        changes.push_back(m);
        current ++;
        return m;
    }

    std::shared_ptr<Memento> undo() {
        if (current>0){
            current --;
            auto m = changes[current];
            balance = m->balance;
            return m;
        }
        return {};
    }

    std::shared_ptr<Memento> redo() {
        if (current<changes.size()){
            current ++;
            auto m = changes[current];
            balance = m->balance;
            return m;
        }
        return {};
    }

    friend std::ostream& operator<<(std::ostream& os, const BankAccount2& obj) {
        return os << "balance: " << obj.balance;
    }
};


TEST(memento, redo_undo) {
    BankAccount2 ba{100};
    ba.deposit(50); // 150;
    ba.deposit(25); // 175;
    std::cout << ba << std::endl;

    // undo
    ba.undo();
    std::cout << "undo1: " << ba << std::endl;
    // undo
    ba.undo();
    std::cout << "undo2: " << ba << std::endl;
    // redo
    ba.redo();
    std::cout << "redo2: " << ba << std::endl;
}

// summary:
// Memento are used to roll back states arbitrarily
// A memento is simply a token/handle class whith (typically) no functions of its own.
// A memento is not required to expose direclty the state(s) to whcih it reverts the system.
// Can be used to implement undo/redo (however command pattern is a bit more sensible in this regard)

} // namespace memento
