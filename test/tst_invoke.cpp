#include "catch.hpp"

#include "../src/statemachine.hpp"
#include "testutils.hpp"

using StateMachine_t = fsm11::StateMachine<>;
using State_t = fsm11::State<StateMachine_t>;

using namespace fsm11;

class InvokeChecker : public State_t
{
public:
    using State_t::State_t;

    virtual void enterInvoke() override
    {
        invoked = true;
    }

    bool invoked = false;
};

TEST_CASE("no invoke in case of an eventless transition", "[invoke]")
{
    StateMachine_t sm;
    InvokeChecker a("a", &sm);
    InvokeChecker b("b", &sm);
    InvokeChecker c("c", &sm);

    sm += a + event(1) == b;
    sm += b + noEvent  == c;

    REQUIRE(!a.invoked);
    REQUIRE(!b.invoked);
    REQUIRE(!c.invoked);

    sm.start();
    REQUIRE(isActive(sm, {&sm, &a}));
    REQUIRE(a.invoked);

    sm.addEvent(1);
    REQUIRE(isActive(sm, {&sm, &c}));
    REQUIRE(!b.invoked);
    REQUIRE(c.invoked);
}
