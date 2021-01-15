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

// Observer
// A pattern in which the object's behavior is determined by its state. An object transitions from one state to another
// (something needs to trigger the transition). A formalized construct which manages states and transitions is
// called a state machine.

// Motivation
// consider an ordinary telephone.
// What you do with it dpends on the state of the phone/line.
// - if its ringing or you want to make a call, you can pick it up.
// - phone must be occ the hootk to take/make a call.
// - if you are calling someone, and it's busy, you put the handset down.
// changes in state can be explicit or in response to events (e.g. observer)

namespace state {

enum class State {
    OffHook,
    Connecting,
    Connected,
    OnHold
};

inline std::ostream& operator << (std::ostream& os, const State& s) {
    switch (s) {
        case State::OffHook:
            os << "OffHook";
            break;
        case State::Connecting:
            os << "Connecting";
            break;
        case State::Connected:
            os << "Connected";
            break;
        case State::OnHold:
            os << "OnHold";
            break;
        default:
            os << "Unknown";
            break;
    }
    return os;
}

enum class Trigger {
    CallDialed,
    HungUp,
    CallConnected,
    PlacedOnHold,
    TakenOffHold,
    LeftMessage
};

inline std::ostream& operator << (std::ostream& os, const Trigger& t) {
    switch (t) {
        case Trigger::CallDialed:
            os << "CallDialed";
            break;
        case Trigger::HungUp:
            os << "HungUp";
            break;
        case Trigger::CallConnected:
            os << "CallConnected";
            break;
        case Trigger::PlacedOnHold:
            os << "PlacedOnHold";
            break;
        case Trigger::TakenOffHold:
            os << "TakenOffHold";
            break;
        case Trigger::LeftMessage:
            os << "LeftMessage";
            break;
        default:
            os << "Unknown";
            break;
    }
    return os;
}


void ShowPossibleStriggers(std::map<State, std::vector<std::pair<Trigger, State>>>& rules, State& s) {
    std::cout << "The Phone is currently " << s << std::endl;
    std::cout << "Select a trigger: " << std::endl;
    int i = 0;
    for (auto item: rules[s]) {
        std::cout << i++ << ". " << item.first << std::endl;
    }
}

State SetTrigger(std::map<State, std::vector<std::pair<Trigger, State>>>& rules, State& s, int input) {
    return rules[s][input].second;
}

TEST(state, state_machine) {
    std::map<State, std::vector<std::pair<Trigger, State>>> rules;
    rules[State::OffHook] = {
            {Trigger::CallDialed, State::Connecting}
    };
    rules[State::Connecting] = {
            {Trigger::HungUp, State::OffHook},
            {Trigger::CallConnected, State::Connected}
    };
    rules[State::Connected] = {
            {Trigger::LeftMessage, State::OffHook},
            {Trigger::HungUp, State::OffHook},
            {Trigger::PlacedOnHold, State::OnHold},
    };
    rules[State::OnHold] = {
            {Trigger::TakenOffHold, State::Connected},
            {Trigger::HungUp, State::OffHook}
    };
    State currentState{ State::OffHook };
    ShowPossibleStriggers(rules, currentState);
    currentState = SetTrigger(rules, currentState, 0);
    ShowPossibleStriggers(rules, currentState);
    currentState = SetTrigger(rules, currentState, 1);
    ShowPossibleStriggers(rules, currentState);
    currentState = SetTrigger(rules, currentState, 2);
    ShowPossibleStriggers(rules, currentState);
    currentState = SetTrigger(rules, currentState, 1);
    ShowPossibleStriggers(rules, currentState);
}

using namespace std;

// back-end
#include <boost/msm/back/state_machine.hpp>

// front-end
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/state_machine_def.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;
using namespace msm::front;

vector<string> state_names{"off hook"s, "connecting"s, "connected"s, "on hold"s,
                           "destroyed"s};

// trigger/events
struct CallDialed{};
struct HungUp{};
struct CallConnected{};
struct PlacedOnHold{};
struct TakenOffHold{};
struct LeftMessage{};
struct PhoneThrownIntoWall{};

struct PhoneStateMachine : state_machine_def<PhoneStateMachine> {
    struct OffHook : state<> {};
    struct Connecting : state<> {};
    struct Connected : state<> {};
    struct OnHold : state<> {};
    struct PhoneDestroyed : state<> {};

// start, event, target, action, guard
    struct transition_table : mpl::vector<
            Row<OffHook, CallDialed, Connecting>,
            Row<Connecting, CallConnected, Connected>,
            Row<Connected, PlacedOnHold, OnHold>,
            Row<OnHold, PhoneThrownIntoWall, PhoneDestroyed>> {};

//    struct transition_table : mpl::vector<
//            Row<OffHook, CallDialed, Connecting>,
//            Row<Connecting, CallConnected, Connected>,
//            Row<Connected, PlacedOnHold, OnHold>,
//            Row<OnHold, PhoneThrownIntoWall, PhoneDestroyed>
//        >{};

};




} // namespace state
