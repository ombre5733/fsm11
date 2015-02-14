#include "catch.hpp"

#include "../statemachine.hpp"
#include "testutils.hpp"

#include <deque>

using namespace fsm11;

struct ActionException
{
};

struct GuardException
{
};

struct ListException
{
};

class ThrowingList
{
public:
    bool empty() const
    {
        return m_events.empty();
    }

    unsigned& front()
    {
        return m_events.front();
    }

    void pop_front()
    {
        m_events.pop_front();
    }

    void push_back(unsigned event)
    {
        if (event == 1)
            throw ListException();
        m_events.push_back(event);
    }

private:
    std::deque<unsigned> m_events;
};

using StateMachine_t = fsm11::StateMachine<EventListType<ThrowingList>>;
using State_t = StateMachine_t::state_type;

TEST_CASE("throw in addEvent", "[exception]")
{
    StateMachine_t sm;

    State_t a("a", &sm);
    State_t aa("aa", &a);
    State_t ab("ab", &a);
    State_t b("b", &sm);
    State_t ba("ba", &b);
    State_t bb("bb", &b);

    sm += aa + event(0) == ba;

    REQUIRE(isActive(sm, {}));
    sm.start();
    REQUIRE(isActive(sm, {&sm, &a, &aa}));

    SECTION("throw")
    {
        REQUIRE_THROWS_AS(sm.addEvent(1), ListException);
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
    }

    SECTION("transit then throw")
    {
        sm.addEvent(0);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        REQUIRE_THROWS_AS(sm.addEvent(1), ListException);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
    }
}

TEST_CASE("throw in guard", "[exception]")
{
    StateMachine_t sm;

    State_t a("a", &sm);
    State_t aa("aa", &a);
    State_t ab("ab", &a);
    State_t b("b", &sm);
    State_t ba("ba", &b);
    State_t bb("bb", &b);

    auto guard = [](unsigned event) {
        if (event == 3)
            throw GuardException();
        return event % 2 == 0;
    };

    sm += aa + event(0) [guard] == ba;
    sm += aa + event(3) [guard] == ba;
    sm += ba + event(3) [guard] == bb;

    REQUIRE(isActive(sm, {}));
    sm.start();
    REQUIRE(isActive(sm, {&sm, &a, &aa}));

    SECTION("throw")
    {
        REQUIRE_THROWS_AS(sm.addEvent(3), GuardException);
    }

    SECTION("transit then throw")
    {
        sm.addEvent(0);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        REQUIRE_THROWS_AS(sm.addEvent(3), GuardException);
    }

    // The state machine must have been stopped.
    REQUIRE(!sm.running());
    REQUIRE(isActive(sm, {}));

    // Restart the state machine.
    sm.start();
    REQUIRE(isActive(sm, {&sm, &a, &aa}));
    sm.addEvent(0);
    REQUIRE(isActive(sm, {&sm, &b, &ba}));
    sm.stop();

    REQUIRE(!sm.running());
    REQUIRE(isActive(sm, {}));
}

TEST_CASE("throw in action", "[exception]")
{
    StateMachine_t sm;

    State_t a("a", &sm);
    State_t aa("aa", &a);
    State_t ab("ab", &a);
    State_t b("b", &sm);
    State_t ba("ba", &b);
    State_t bb("bb", &b);

    auto action = [](unsigned event) {
        if (event == 3)
            throw ActionException();
    };

    sm += aa + event(0) / action == ba;
    sm += aa + event(3) / action == ba;
    sm += ba + event(3) / action == bb;

    REQUIRE(isActive(sm, {}));
    sm.start();
    REQUIRE(isActive(sm, {&sm, &a, &aa}));

    SECTION("throw")
    {
        REQUIRE_THROWS_AS(sm.addEvent(3), ActionException);
    }

    SECTION("transit then throw")
    {
        sm.addEvent(0);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        REQUIRE_THROWS_AS(sm.addEvent(3), ActionException);
    }

    // The state machine must have been stopped.
    REQUIRE(!sm.running());
    REQUIRE(isActive(sm, {}));

    // Restart the state machine.
    sm.start();
    REQUIRE(isActive(sm, {&sm, &a, &aa}));
    sm.addEvent(0);
    REQUIRE(isActive(sm, {&sm, &b, &ba}));
    sm.stop();

    REQUIRE(!sm.running());
    REQUIRE(isActive(sm, {}));
}
