#include "catch.hpp"

#include "../statemachine.hpp"

using StateMachine_t = fsm11::StateMachine<fsm11::EnableStateCallbacks<true>>;
using State_t = fsm11::State<StateMachine_t>;

TEST_CASE("create state machine with state callbacks", "[callbacks]")
{
    StateMachine_t sm;
}

TEST_CASE("entry and exit callbacks are invoked", "[callbacks]")
{
    StateMachine_t sm;
    State_t a("a", &sm);
    State_t aa("aa", &a);
    State_t b("b", &sm);

    std::set<State_t*> states;
    SECTION("entry callback")
    {
        sm.setStateEntryCallback([&](State_t* s){ states.insert(s); });
        sm.start();
        REQUIRE(states.size() == 3);
        sm.stop();
        REQUIRE(states.size() == 3);
    }

    SECTION("exit callback")
    {
        sm.setStateExitCallback([&](State_t* s){ states.insert(s); });
        sm.start();
        REQUIRE(states.size() == 0);
        sm.stop();
        REQUIRE(states.size() == 3);
    }

    auto contains = [&](State_t* s){ return states.find(s) != states.end(); };
    REQUIRE(contains(&sm));
    REQUIRE(contains(&a));
    REQUIRE(contains(&aa));
}
