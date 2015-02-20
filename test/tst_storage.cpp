#include "catch.hpp"

#include "../src/statemachine.hpp"
#include "testutils.hpp"

using namespace fsm11;

TEST_CASE("empty storage", "[storage]")
{
    using StateMachine_t = fsm11::StateMachine<Storage<>>;
    StateMachine_t sm;
}

TEST_CASE("storage with one built-in type", "[storage]")
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

TEST_CASE("storage with two built-in types", "[storage]")
{
    using StateMachine_t = fsm11::StateMachine<Storage<double, int>>;
    StateMachine_t sm;

    sm.store<0>(3.14);
    sm.store<1>(42);

    REQUIRE(sm.load<0>() == 3.14);
    REQUIRE(sm.load<1>() == 42);
}

enum Color
{
    Red,
    Green,
    Yellow
};

TEST_CASE("storage with user-defined enum", "[storage]")
{
    using StateMachine_t = fsm11::StateMachine<Storage<Color>>;
    StateMachine_t sm;

    sm.store<0>(Red);
    REQUIRE(sm.load<0>() == Red);

    sm.store<0>(Green);
    REQUIRE(sm.load<0>() == Green);
}

TEST_CASE("access storage in guard", "[storage]")
{
    using StateMachine_t = fsm11::StateMachine<Storage<int>>;
    using State_t = StateMachine_t::state_type;

    StateMachine_t sm;
    State_t a("a", &sm);
    State_t b("b", &sm);
    State_t c("c", &sm);

    sm += a + noEvent ([&](unsigned){ return sm.load<0>() == 1; }) == b;
    sm += a + noEvent ([&](unsigned){ return sm.load<0>() == 2; }) == c;

    SECTION("set to 0")
    {
        sm.store<0>(0);
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
    }

    SECTION("set to 1")
    {
        sm.store<0>(1);
        sm.start();
        REQUIRE(isActive(sm, {&sm, &b}));
    }

    SECTION("set to 2")
    {
        sm.store<0>(2);
        sm.start();
        REQUIRE(isActive(sm, {&sm, &c}));
    }
}
