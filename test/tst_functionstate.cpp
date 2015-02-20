#include "catch.hpp"

#include "../src/functionstate.hpp"
#include "../src/statemachine.hpp"

using StateMachine_t = fsm11::StateMachine<>;
using State_t = fsm11::FunctionState<StateMachine_t>;

TEST_CASE("construct function state", "[functionstate]")
{
    State_t s0("s0");
    REQUIRE(!s0.isActive());
    REQUIRE(s0.parent() == 0);
    REQUIRE(!s0.entryFunction());
    REQUIRE(!s0.exitFunction());

    State_t s1("s1", nullptr, nullptr);
    REQUIRE(!s1.isActive());
    REQUIRE(s1.parent() == 0);
    REQUIRE(!s1.entryFunction());
    REQUIRE(!s1.exitFunction());

    int entered = 0;
    int left = 0;
    auto enter = [&](unsigned) { ++entered; };
    auto leave = [&](unsigned) { ++left; };

    State_t s2("s2", enter, nullptr, &s1);
    REQUIRE(s2.parent() == &s1);

    REQUIRE(s2.entryFunction());
    REQUIRE(!s2.exitFunction());
    s2.entryFunction()(0);
    REQUIRE(entered == 1);
    REQUIRE(left == 0);

    State_t s3("s3", nullptr, leave);
    REQUIRE(!s3.entryFunction());
    REQUIRE(s3.exitFunction());
    s3.exitFunction()(0);
    REQUIRE(entered == 1);
    REQUIRE(left == 1);

    State_t s4("s4", enter, leave);
    REQUIRE(s4.entryFunction());
    REQUIRE(s4.exitFunction());
    s4.entryFunction()(0);
    s4.exitFunction()(0);
    REQUIRE(entered == 2);
    REQUIRE(left == 2);
}

TEST_CASE("set actions of function state", "[functionstate]")
{
    int entered = 0;
    int left = 0;
    auto enter = [&](unsigned ev) { entered += ev; };
    auto leave = [&](unsigned ev) { left += ev; };

    State_t s("s");
    REQUIRE(!s.entryFunction());
    REQUIRE(!s.exitFunction());

    s.setEntryFunction(enter);
    REQUIRE(s.entryFunction() != nullptr);
    REQUIRE(!s.exitFunction());

    s.entryFunction()(3);
    REQUIRE(entered == 3);
    REQUIRE(left == 0);
    REQUIRE(s.entryFunction() != nullptr);
    REQUIRE(!s.exitFunction());

    s.setExitFunction(leave);
    REQUIRE(s.entryFunction() != nullptr);
    REQUIRE(s.exitFunction() != nullptr);

    s.exitFunction()(5);
    REQUIRE(entered == 3);
    REQUIRE(left == 5);
    REQUIRE(s.entryFunction() != nullptr);
    REQUIRE(s.exitFunction() != nullptr);

    s.setEntryFunction(nullptr);
    REQUIRE(!s.entryFunction());
    REQUIRE(s.exitFunction() != nullptr);

    s.setExitFunction(nullptr);
    REQUIRE(!s.entryFunction());
    REQUIRE(!s.exitFunction());
}
