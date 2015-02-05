#include "catch.hpp"

#include "../statemachine.hpp"

using namespace fsm11;

using StateMachine_t = fsm11::StateMachine<>;
using State_t = StateMachine_t::state_type;

TEST_CASE("construct a statemachine", "[statemachine]")
{
    StateMachine_t sm;
    REQUIRE(!sm.running());
}

TEST_CASE("start an empty statemachine", "[statemachine]")
{
    StateMachine_t sm;
    REQUIRE(!sm.running());
    for (int cnt = 0; cnt < 2; ++cnt)
    {
        sm.start();
        REQUIRE(sm.running());
        sm.stop();
        REQUIRE(!sm.running());
    }
}
