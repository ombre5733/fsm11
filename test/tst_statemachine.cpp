#include "catch.hpp"

#include "../src/statemachine.hpp"
#include <future>

using namespace fsm11;


namespace sync
{
using StateMachine_t = StateMachine<>;
using State_t = StateMachine_t::state_type;
} // namespace sync

namespace async
{
using StateMachine_t = StateMachine<AsynchronousEventDispatching,
                                    EnableConfigurationChangeCallbacks<true>>;
using State_t = StateMachine_t::state_type;
} // namespace async


TEST_CASE("construct a synchronous statemachine", "[statemachine]")
{
    using namespace sync;
    StateMachine_t sm;
    REQUIRE(!sm.running());
    REQUIRE(sm.numConfigurationChanges() == 0);
}

TEST_CASE("construct an asynchronous statemachine", "[statemachine]")
{
    using namespace async;
    StateMachine_t sm;
    REQUIRE(!sm.running());
    REQUIRE(sm.numConfigurationChanges() == 0);
}

TEST_CASE("the eventloop of an asynchronous statemachine is left upon destruction",
          "[statemachine]")
{
    std::future<void> result;

    {
        using namespace async;
        StateMachine_t sm;
        REQUIRE(!sm.running());
        REQUIRE(sm.numConfigurationChanges() == 0);

        result = std::async(std::launch::async, &StateMachine_t::eventLoop, &sm);
    }
}

TEST_CASE("start an empty synchronous statemachine", "[statemachine]")
{
    using namespace sync;
    StateMachine_t sm;
    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    for (int cnt = 0; cnt < 2; ++cnt)
    {
        sm.start();
        REQUIRE(sm.running());
        REQUIRE(sm.isActive());
        REQUIRE(sm.numConfigurationChanges() == 2 * cnt + 1);
        sm.stop();
        REQUIRE(!sm.running());
        REQUIRE(!sm.isActive());
        REQUIRE(sm.numConfigurationChanges() == 2 * cnt + 2);
    }
}

TEST_CASE("start an empty asynchronous statemachine", "[statemachine]")
{
    using namespace async;
    StateMachine_t sm;

    std::mutex mutex;
    bool configurationChanged = false;
    std::condition_variable cv;

    sm.setConfigurationChangeCallback([&] {
        std::unique_lock<std::mutex> lock(mutex);
        configurationChanged = true;
        cv.notify_all();
    });

    auto waitForConfigurationChange = [&] {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&] { return configurationChanged; });
        configurationChanged = false;
    };

    REQUIRE(!sm.running());
    for (int cnt = 0; cnt < 2; ++cnt)
    {
        auto result = std::async(std::launch::async, &StateMachine_t::eventLoop, &sm);

        sm.start();
        waitForConfigurationChange();
        REQUIRE(sm.running());
        REQUIRE(sm.isActive());
        REQUIRE(sm.numConfigurationChanges() == 2 * cnt + 1);
        sm.stop();
        waitForConfigurationChange();
        REQUIRE(!sm.running());
        REQUIRE(!sm.isActive());
        REQUIRE(sm.numConfigurationChanges() == 2 * cnt + 2);
    }
}

TEST_CASE("an asynchronous statemachine is stopped upon destruction",
          "[statemachine]")
{
    using namespace async;
    std::future<void> result;

    SECTION("without starting")
    {
        StateMachine_t sm;
        result = std::async(std::launch::async, &StateMachine_t::eventLoop, &sm);
    }

    SECTION("with starting")
    {
        StateMachine_t sm;
        result = std::async(std::launch::async, &StateMachine_t::eventLoop, &sm);
        sm.start();
    }
}
