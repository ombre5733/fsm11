#include "catch.hpp"

#include "../src/functionstate.hpp"
#include "../src/statemachine.hpp"
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

struct StateException
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

using StateMachine_t = StateMachine<EventListType<ThrowingList>>;
using State_t = StateMachine_t::state_type;
using FunctionState_t = FunctionState<StateMachine_t>;

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

TEST_CASE("throw in transition guard", "[exception]")
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

TEST_CASE("throw in transition action", "[exception]")
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

TEST_CASE("throw in onEntry", "[exception]")
{
    using TrackingState_t = TrackingState<FunctionState_t>;

    StateMachine_t sm;

    TrackingState_t a("a", &sm);
    TrackingState_t aa("aa", &a);
    State_t ab("ab", &a);
    State_t b("b", &sm);
    State_t ba("ba", &b);
    State_t bb("bb", &b);

    a.setEntryFunction([](unsigned) { throw StateException(); });

    REQUIRE_THROWS_AS(sm.start(), StateException);
    REQUIRE(a.entered == 1);
    REQUIRE(a.left == 0);

    REQUIRE(aa.entered == 0);
    REQUIRE(aa.left == 0);
}

TEST_CASE("throw in onExit", "[exception]")
{
    using TrackingState_t = TrackingState<FunctionState_t>;

    StateMachine_t sm;

    TrackingState_t a("a", &sm);
    TrackingState_t aa("aa", &a);
    State_t ab("ab", &a);
    State_t b("b", &sm);
    State_t ba("ba", &b);
    State_t bb("bb", &b);

    aa.setExitFunction([](unsigned) { throw StateException(); });

    sm.start();
    REQUIRE_THROWS_AS(sm.stop(), StateException);
    REQUIRE(a.entered == 1);
    REQUIRE(a.left == 1);

    REQUIRE(aa.entered == 1);
    REQUIRE(aa.left == 1);
}

#if 0
TEST_CASE("async sm: throw in transition guard", "[exception]")
{
    using StateMachine_t = StateMachine<AsynchronousEventDispatching,
                                        EventListType<ThrowingList>,
                                        ConfigurationChangeCallbacksEnable<true>>;
    using State_t = StateMachine_t::state_type;
    using FunctionState_t = FunctionState<StateMachine_t>;

    std::future<void> result;

    std::mutex mutex;
    bool configurationChanged = false;
    std::condition_variable cv;

    auto waitForConfigurationChange = [&] {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&] { return configurationChanged; });
        configurationChanged = false;
    };

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

    sm.setConfigurationChangeCallback([&] {
        std::unique_lock<std::mutex> lock(mutex);
        configurationChanged = true;
        cv.notify_all();
    });

    result = sm.startAsyncEventLoop();

    REQUIRE(isActive(sm, {}));
    sm.start();
    waitForConfigurationChange();
    REQUIRE(isActive(sm, {&sm, &a, &aa}));

    SECTION("throw")
    {
        REQUIRE(result.valid());
        sm.addEvent(3);
        REQUIRE_THROWS_AS(result.get(), GuardException);
    }

    SECTION("transit then throw")
    {
        sm.addEvent(0);
        waitForConfigurationChange();
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
#endif
