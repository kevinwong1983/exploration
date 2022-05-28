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

// Proxy

// motivation:
// a class that is functioning as an interface to a particular resource. That resource may be remote,
// expensive to construct, or may require logging or some other added functionality.

struct BankAccount {
    virtual ~BankAccount() = default;

    virtual void deposit(int amount) = 0;

    virtual void withdraw(int amount) = 0;
};

struct CurrentBankAccount : BankAccount {
    explicit CurrentBankAccount(const int b)
            : balance(b) {
    }

    void deposit(int amount) override {
        balance += amount;
        std::cout << "Deposited $" + boost::lexical_cast<std::string>(amount) + ", balance is now $"
                     + boost::lexical_cast<std::string>(balance) << std::endl;
    }

    void withdraw(int amount) override {
        if (balance >= amount) {
            balance -= amount;
            std::cout << "Withdrew $" + boost::lexical_cast<std::string>(amount) + ", $"
                         + boost::lexical_cast<std::string>(balance) + " left." << std::endl;
        } else {
            std::cout << "Tried to withdraw $" + boost::lexical_cast<std::string>(amount)
                         + " but couldn't due to low balance." << std::endl;
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const CurrentBankAccount &obj) {
        return os << "balance: " << obj.balance;
    }

private:
    int balance;
};

TEST(proxy, smart_pointers) {
    BankAccount *a = new CurrentBankAccount{123};
    a->deposit(321);
    delete a;

    std::shared_ptr<CurrentBankAccount> b = std::make_shared<CurrentBankAccount>(123);
    // the interface of the pointer is exactly the same as the smart pointer!
    b->deposit(321);
    // it exposes everything what the underlying pointer exposes

    CurrentBankAccount *underlying = b.get();
    std::cout << b << std::endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////

struct Image {
    virtual ~Image() = default;

    virtual void draw() = 0;
};

struct Bitmap : Image {
    Bitmap(const std::string &filename) {
        std::cout << "Loading a file from" << filename << std::endl;
    }

    void draw() override {
        std::cout << "Drawing image" << std::endl;
    }
};

// use polymorpsm by passing this function either a Bitmap or a LazyBitmap
void draw_image(Image &img) {
    std::cout << "About to draw the image" << std::endl;
    img.draw();
    std::cout << "Done drawing the image" << std::endl;
}

TEST(proxy, virtual_proxy_normal) {
    Bitmap b{"pokemon.png"}; // load the image already before we draw. This might not be what you want.
    draw_image(b);
}

struct LazyBitmap : Image {
    LazyBitmap(const std::string &filename) :
            filename{filename}, bmp{nullptr} {
    }

    ~LazyBitmap() {
    }

    void draw() override {
        if (!bmp) {
            bmp = new Bitmap{filename};
        }
        bmp->draw();
    }

private:
    Bitmap *bmp;
    std::string filename;
};

TEST(proxy, virtual_proxy_lazy) {
    LazyBitmap b{"pokemon.png"}; // load the image already before we draw. This might not be what you want.
    draw_image(b);
    std::cout << "--------" << std::endl;
    draw_image(b);
}

///////////////////////////////////////////////////////////////////////////////////////////////

struct Pingable {
    virtual ~Pingable() = default;

    virtual std::wstring ping(const std::wstring &message) = 0;
};

struct Pong : Pingable {
    std::wstring ping(const std::wstring &message) override {
        return message + L" pong";
    }
};

void tryIt(Pingable &p) {
    std::wcout << p.ping(L"ping") << "\t";
}

TEST(proxy, communication_proxy) {
    Pong pp;
    for (size_t i = 0; i < 10; i++) {
        tryIt(pp);
    }

    // Pong here is a in process service
    // However not we want to move Pong to a webserver with an restfull api that provides the same service.
}

//#include <cppreset/http_client.h>
//using namespace web;
//using namespace http;
//using namespace client;

// Remote Pong does a restfull call to http://localhost:1234/api/pingpong/<message> witch returns the ping message.
struct RemotePong: Pingable {
    std::wstring ping(const std::wstring& message) override {
//        http_client client (U{"http://localhost:914/"});
//        uri_builder builder(U("/api/pingpong/"));
//        builder.append(message);
//        pplx::task<wstring< task> = client.request(methods::GET, builder)
//                .then([=](http_response r) {
//                    return r.extreact_string();
//                });
//        task.wait)_;
//        return task.get();
        return L"";
    }
};
// API stays the same! thanks to a interface we can just substitute one of another.

// How is Proxy diffent form Decorator:
// - Proxy provides an identical interface; decorator provides an enhanced interface;
// - Decorator typecally aggregates (or has reference to) what it is decorating; proxy doesnt have to;
// - Proxy might not event be working with an materialized object;

// summary
// A proxy has the same interface as the underlying object. It can mascarade as the original object.
// To create a proxy, simply replicate the existing interface of an object.
// Add relevant functionality to the redefined member functions
// - As wel as constructor, destructor, etc
// Different proxies (communication, logging, caching, etc) have completely different behaviors. The only thing they have
// in common is that they share the same underlying interface of the underlying object.










