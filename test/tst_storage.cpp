#include "catch.hpp"

#include "../statemachine.hpp"

using namespace fsm11;

TEST_CASE("empty storage", "[storage]")
{
    using StateMachine_t = fsm11::StateMachine<Storage<>>;
    StateMachine_t sm;
}

TEST_CASE("storage with built-in type", "[storage]")
{
    SECTION("int")
    {
        using StateMachine_t = fsm11::StateMachine<Storage<int>>;
        StateMachine_t sm;
        sm.store<0>(21);
        REQUIRE(sm.load<0>() == 21);
        sm.store<0>(6);
        REQUIRE(sm.load<0>() == 6);
    }

    SECTION("double")
    {
        using StateMachine_t = fsm11::StateMachine<Storage<double>>;
        StateMachine_t sm;
        sm.store<0>(3.14);
        REQUIRE(sm.load<0>() == 3.14);
        sm.store<0>(2.71);
        REQUIRE(sm.load<0>() == 2.71);
    }

    SECTION("const char*")
    {
        const char* greeting = "hello";
        const char* person = "Manuel";
        using StateMachine_t = fsm11::StateMachine<Storage<const char*>>;
        StateMachine_t sm;
        sm.store<0>(greeting);
        REQUIRE(sm.load<0>() == greeting);
        sm.store<0>(person);
        REQUIRE(sm.load<0>() == person);
    }
}
