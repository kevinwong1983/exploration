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
#include <boost/bimap.hpp>

// Null Object
// A no-op object that satisfies the dependency requirement of same other object.

// motivation:
// When component A uses component B, if typically assumes that B is actually present.
// - you inject B, not e.g. optional<B>
// - you do not inject a pointer and then check for nullptr everywhere
// There is no option of telling A not to use an instance of B
// - Its use is hard-coded
// Thus, we build a no-op, non-functioning inheritor of B and pass that into A

struct Logger {
    virtual ~Logger() = default;

    virtual void info(const std::string &s) = 0;

    virtual void warn(const std::string &s) = 0;

};

struct ConsoleLogger : public Logger {
    void info(const std::string &s) override {
        std::cout << "INFO: " << s << std::endl;
    }

    void warn(const std::string &s) override {
        std::cout << "WARN: " << s << std::endl;
    }
};

struct BankAccount {
    std::shared_ptr<Logger> log;    // Bank account is dependent on Logger!
    std::string name;
    int balance = 0;

    BankAccount(const std::shared_ptr<Logger> &logger, const std::string &name, const int balance) :
            log{logger}, name{name}, balance{balance} {
    }

    void deposit(int amount) {
        balance += amount;
        log->info("Deposited $" + boost::lexical_cast<std::string>(amount) +
                " to " + name + ", balance is now $" + boost::lexical_cast<std::string>(balance));
    }

    void withdraw(int amount) {
        if (balance >= amount) {
            balance -= amount;
            log->info("Withdrew $" + boost::lexical_cast<std::string>(amount) +
                      " from " + name + ", $" + boost::lexical_cast<std::string>(balance) + " left.");
        } else {
            log->warn("Tried to withdraw $" + boost::lexical_cast<std::string>(amount) +
                      " from " + name + " but couldn't due to low balance.");
        }
    }

};

TEST(null_object, simple) {
    auto logger = std::make_shared<ConsoleLogger>();
    BankAccount account{logger,  "primary logger",  1000};

    account.deposit(2000);
    account.withdraw(2500);
    account.withdraw(1000);

    // if we do not want to log??
    // we cannot do std::shared_ptr<Logger> logger; and pass this to account
    // this will crash
    // std::shared_ptr<Logger> empty_logger;
    // BankAccount account2{empty_logger,  "primary logger",  1000};
    // account2.deposit(2000);
    // account2.withdraw(2500);
    // account2.withdraw(1000);
    // Process finished with exit code 139 (interrupted by signal 11: SIGSEGV)
}

struct NullLogger : Logger {
    void info(const std::string &s) override {
        // empty
    }

    void warn(const std::string &s) override {
        // empty
    }
};

TEST(null_object, null_logger) {
    auto logger = std::make_shared<NullLogger>();
    BankAccount account{logger,  "primary logger",  1000};

    account.deposit(2000);
    account.withdraw(2500);
    account.withdraw(1000);
}

// summary
// Inherit from the required object
// Implements the function with empty bodies:
// - Return default values
// - If those valies are used, you are in trouble!!!
// Supply an instance of the Null Object in line of an actual object.





