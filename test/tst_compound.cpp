#include "catch.hpp"

#include "../src/statemachine.hpp"

using namespace fsm11;

using StateMachine_t = StateMachine<>;
using State_t = StateMachine_t::state_type;

TEST_CASE("start fsm with compound root state", "[compound]")
{
    StateMachine_t sm;

    State_t a("a", &sm);
    State_t b("b", &sm);
    State_t c("c", &sm);

    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(sm.isCompound());

    sm.start();
    REQUIRE(sm.running());
    REQUIRE(sm.isActive());
    REQUIRE(a.isActive());
    REQUIRE(!b.isActive());
    REQUIRE(!c.isActive());

    sm.stop();
    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(!a.isActive());
}

TEST_CASE("start fsm with compound top-level state", "[compound]")
{
    StateMachine_t sm;

    State_t a("a", &sm);
    State_t b("b", &sm);
    State_t c("c", &a);
    State_t d("d", &a);

    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(a.isCompound());

    sm.start();
    REQUIRE(sm.running());
    REQUIRE(sm.isActive());
    REQUIRE(a.isActive());
    REQUIRE(!b.isActive());
    REQUIRE(c.isActive());
    REQUIRE(!d.isActive());

    sm.stop();
    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(!a.isActive());
    REQUIRE(!c.isActive());
}

TEST_CASE("start fsm with parallel root state", "[parallel]")
{
    StateMachine_t sm;
    sm.setChildMode(StateMachine_t::Parallel);

    State_t a("a", &sm);
    State_t b("b", &sm);
    State_t c("c", &sm);

    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(sm.isParallel());

    sm.start();
    REQUIRE(sm.running());
    REQUIRE(sm.isActive());
    REQUIRE(a.isActive());
    REQUIRE(b.isActive());
    REQUIRE(c.isActive());

    sm.stop();
    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(!a.isActive());
    REQUIRE(!b.isActive());
    REQUIRE(!c.isActive());
}

TEST_CASE("start fsm with parallel top-level state", "[parallel]")
{
    StateMachine_t sm;

    State_t a("a", &sm);
    a.setChildMode(State_t::Parallel);
    State_t b("b", &sm);
    State_t c("c", &a);
    State_t d("d", &a);

    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(a.isParallel());

    sm.start();
    REQUIRE(sm.running());
    REQUIRE(sm.isActive());
    REQUIRE(a.isActive());
    REQUIRE(!b.isActive());
    REQUIRE(c.isActive());
    REQUIRE(d.isActive());

    sm.stop();
    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(!a.isActive());
    REQUIRE(!c.isActive());
    REQUIRE(!d.isActive());
}
