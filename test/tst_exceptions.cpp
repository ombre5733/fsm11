#include "catch.hpp"

#include "../statemachine.hpp"

#include <deque>
#include <set>

using namespace fsm11;

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

template <typename T>
bool isActive(const T& sm,
              const std::set<const typename T::state_type*>& expected)
{
    for (const auto& state : sm.pre_order_subtree())
    {
        auto iter = expected.find(&state);
        if (iter != expected.end())
            REQUIRE(state.isActive());
        else
            REQUIRE(!state.isActive());
    }
    return true;
}

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
        bool caught = false;
        try
        {
            sm.addEvent(1);
        }
        catch (ListException&)
        {
            caught = true;
        }

        REQUIRE(caught);
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
    }

    SECTION("transit then throw")
    {
        sm.addEvent(0);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));

        bool caught = false;
        try
        {
            sm.addEvent(1);
        }
        catch (ListException&)
        {
            caught = true;
        }

        REQUIRE(caught);
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
        bool caught = false;
        try
        {
            sm.addEvent(3);
        }
        catch (GuardException&)
        {
            caught = true;
        }

        REQUIRE(caught);
    }

    SECTION("transit then throw")
    {
        sm.addEvent(0);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));

        bool caught = false;
        try
        {
            sm.addEvent(3);
        }
        catch (GuardException&)
        {
            caught = true;
        }

        REQUIRE(caught);
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
            throw GuardException();
    };

    sm += aa + event(0) / action == ba;
    sm += aa + event(3) / action == ba;
    sm += ba + event(3) / action == bb;

    REQUIRE(isActive(sm, {}));
    sm.start();
    REQUIRE(isActive(sm, {&sm, &a, &aa}));

    SECTION("throw")
    {
        bool caught = false;
        try
        {
            sm.addEvent(3);
        }
        catch (GuardException&)
        {
            caught = true;
        }

        REQUIRE(caught);
    }

    SECTION("transit then throw")
    {
        sm.addEvent(0);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));

        bool caught = false;
        try
        {
            sm.addEvent(3);
        }
        catch (GuardException&)
        {
            caught = true;
        }

        REQUIRE(caught);
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
