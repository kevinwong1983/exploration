#pragma once


namespace mediator {
    struct Person;
}// namespace mediator

#include <string>
#include "ChatRoom.h"

namespace mediator {
struct Person {
    std::string name;
    ChatRoom *room = nullptr;
    std::vector<std::string> chat_log;

    explicit Person(const std::string &name);

    void say(const std::string &message) const;

    void receive(const std::string &origin, const std::string &message);

    void pm(const std::string& who, const std::string& message);
};
}