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

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Command pattern:
// - Single Interface named command, with a single method named execute
// - An object which represents an instruction to perform a particular action.
// Contains all information necessary for action to be taken.

// Motivation:
// - Decoupling what is done (action) , from who does it (actors)
// - Decoupling what is done (action) , from when its done
// - Ordinary C++ statement are perishable
// -- cannot undo a field assignment
// -- cannot directly serialize a sequence of actions
// - We want an object that represents an operation
// -- X should change its Y to Z
// -- X should do Q
// Uses: GUI commands, multi-level undo/redo, macro recording and more!

class BankAccount {

public:
    BankAccount()
            : balance_(0),
              overdraft_limit_(-500) {
    }

    void Deposit(int amount) {
        balance_ += amount;
        std::cout << "deposit " << amount
                  << " ,balance is now: " << balance_ << std::endl;
    }

    void Withdraw(int amount) {
        if ((balance_ - amount) >= overdraft_limit_) {
            balance_ -= amount;
            std::cout << "withdraw " << amount
                      << " ,balance is now: " << balance_ << std::endl;
        }
    }

    int balance_;
    int overdraft_limit_;
};

TEST(command, bank_account) {
    // BankAccount ba(); //why does this not just work ??
    auto ba = BankAccount();
    EXPECT_EQ(ba.balance_, 0);
    EXPECT_EQ(ba.overdraft_limit_, -500);

    ba.Deposit(100);
    ba.Withdraw(200);
    EXPECT_EQ(ba.balance_, -100);
}

// What we want is to encapsulate the process of withdrawing an depositing things into
// separate objects. Instead of calling the functions directly, we would like to have
// some kind of event bus/processing queue for processing the series of operations.

class ICommand {    // this is used for composite command.
public:
    virtual ~ICommand() = default;          // what does this default destructor do?
    virtual void Execute() const = 0;

    virtual void Undo() const = 0;
};

// encapsulate all details of an operation in a separate object
class Command : ICommand {
public:
    BankAccount &account_;
    int amount_;
    enum Action {
        deposit, withdraw
    } action_;

    Command(BankAccount &account, const Action action, int amount)
            : account_(account),
              amount_(amount),
              action_(action) {
    }

    // define instructions for applying command (either in
    // the command itself, or elsewhere)
    void Execute() const override {
        switch (action_) {
            case deposit:
                account_.Deposit(amount_);
                break;
            case withdraw:
                account_.Withdraw(amount_);
                break;
            default:
                break;
        }
    }

    void Undo() const override {
        switch (action_) {
            case withdraw:  // change deposit wiht withdraw and visa versa
                account_.Deposit(amount_);
                break;
            case deposit:
                account_.Withdraw(amount_);
                break;
            default:
                break;
        }
    }
};

TEST(command, decoupled_what_from_who_with_command_pattern) {
    // BankAccount ba(); //why does this not just work ??
    auto ba = BankAccount();
    EXPECT_EQ(ba.balance_, 0);
    EXPECT_EQ(ba.overdraft_limit_, -500);

    // we have encapsulated everything that needs to be done on the back account.
    std::vector<Command> commands; // list of commands
    commands.emplace_back(Command(ba, Command::deposit, 100));
    commands.emplace_back(Command(ba, Command::withdraw, 200));

    for (auto &c : commands) {
        c.Execute();
    }
    EXPECT_EQ(ba.balance_, -100);
}

TEST(command, undo) {
    // when we have this encapsulation it is very easy to do a undo/redo mechanism
    auto ba = BankAccount();
    EXPECT_EQ(ba.balance_, 0);
    EXPECT_EQ(ba.overdraft_limit_, -500);

    std::vector<Command> commands{Command(ba, Command::deposit, 100), Command(ba, Command::withdraw, 200)};
    for (auto &c : commands) {
        c.Execute();
    }
    EXPECT_EQ(ba.balance_, -100);

    // now we undo
    std::for_each(commands.rbegin(), commands.rend(), [](Command &cmd) {
        cmd.Undo();
    });
    EXPECT_EQ(ba.balance_, 0);
}

// Interaction between the command pattern and the composite pattern. Composite pattern states that single object and
// a multiple of objects should have the same api for using them. Which is very relevant for commands. Previously we
// used a vector of commands. But we would like to use a collection of our own.

// create composite commands a.k.a macros
class CommandList : std::vector<Command>, public ICommand {     // inherents everything from std::vector<Command>
public:
    CommandList(const ::std::initializer_list<value_type> &_Ilist)
            : vector<Command>(_Ilist) {
    }

    void Execute() const override {
        for (auto &cmd : (*this)) {         // this is pointer to vector // why do I need to dereference ???
            cmd.Execute();
        }
    }

    void Undo() const override {
        std::for_each((*this).rbegin(), (*this).rend(), [](const Command &cmd) {
            cmd.Undo();
        });
    }
};

TEST(command, composite_commands) {
    auto ba = BankAccount();
    EXPECT_EQ(ba.balance_, 0);
    EXPECT_EQ(ba.overdraft_limit_, -500);

    CommandList commands{Command(ba, Command::deposit, 100), Command(ba, Command::withdraw, 200)};
    commands.Execute();
    EXPECT_EQ(ba.balance_, -100);

    commands.Undo();
    EXPECT_EQ(ba.balance_, 0);
}


// Strategy Pattern: simple pattern that allows us to separate high level policy from a set of low level details.
// - FTP class (+sendFile) -> IPacketProtocollStrategy(+sendPackage) <|- X-model / Y-Modem / Z-Modem
// - High level policy is dedicated through an interface to derivatives that Implement the low-level stuff.
// - Simple “external” polymorphism…
// - Advantage compared to Template Method pattern: 1. More flexible (reduce coupling) 2. Change-ability: Can be hot swapped. You can change out strategy at any time you want 3. Independent deploy-ability: High level policy is independent of low level implementation, however low level implementation is strongly dependent on high level policy! However in Strategy, both are independent from each other and can be deployed separately from each other.
//
// Template Method Pattern: solves the same problem as Strategy, but does it in a different way.
// - FTP Abstract class (+sendFile +sendPackage=0) <|- X-model / Y-Modem / Z-Modem
// - Used internal polymorphism
// - Advantage compared to Strategy pattern: 1. ease of use, 2. faster and smaller,