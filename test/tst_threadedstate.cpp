#include "catch.hpp"

#include "../src/threadedstate.hpp"
#include "../src/statemachine.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>

using StateMachine_t = fsm11::StateMachine<>;
using State_t = fsm11::ThreadedState<StateMachine_t>;

std::mutex g_mutex;
std::thread::id g_id;
std::condition_variable g_cv;
bool g_notify;
int g_invokeState;

class TestState : public State_t
{
public:
    using State_t::State_t;

    virtual void invoke() override
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        g_id = std::this_thread::get_id();
    }
};

template <unsigned TFn>
class WaitingState : public State_t
{
public:
    using State_t::State_t;

    virtual void invoke() override
    {
        g_mutex.lock();
        g_notify = true;
        g_invokeState = 1;
        g_mutex.unlock();
        g_cv.notify_all();

        switch (TFn)
        {
        case 0: waitForExitRequest(); break;
        case 1: waitForExitRequestFor(std::chrono::milliseconds(500)); break;
        default: REQUIRE(false); break;
        }

        g_mutex.lock();
        g_notify = true;
        g_invokeState = 2;
        g_mutex.unlock();
        g_cv.notify_all();
    }
};

TEST_CASE("construct threaded state", "[threadedstate]")
{
    TestState s1("s1");
    TestState s2("s2", &s1);
    REQUIRE(s2.parent() == &s1);
}

TEST_CASE("start invoke action in threaded state", "[threadedstate]")
{
    StateMachine_t sm;
    TestState s1("s1", &sm);

    g_mutex.lock();
    g_id = std::this_thread::get_id();
    g_mutex.unlock();

    sm.start();
    sm.stop();
    REQUIRE(g_id != std::this_thread::get_id());
}

TEST_CASE("waitForExitRequest blocks an invoked action", "[threadedstate]")
{
    StateMachine_t sm;
    WaitingState<0> s1("s1", &sm);

    g_notify = false;
    g_invokeState = 0;

    sm.start();

    std::unique_lock<std::mutex> lock(g_mutex);
    g_cv.wait(lock, [&] { return g_notify; });
    REQUIRE(g_invokeState == 1);
    lock.unlock();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // The invoked action must still be blocked.
    lock.lock();
    g_cv.wait(lock, [&] { return g_notify; });
    REQUIRE(g_invokeState == 1);
    g_notify = false;
    lock.unlock();

    sm.stop();

    lock.lock();
    g_cv.wait(lock, [&] { return g_notify; });
    REQUIRE(g_invokeState == 2);
    lock.unlock();
}

TEST_CASE("waitForExitRequestFor blocks an invoked action", "[threadedstate]")
{
    StateMachine_t sm;
    WaitingState<1> s1("s1", &sm);

    g_notify = false;
    g_invokeState = 0;

    sm.start();

    std::unique_lock<std::mutex> lock(g_mutex);
    g_cv.wait(lock, [&] { return g_notify; });
    REQUIRE(g_invokeState == 1);
    g_notify = false;
    lock.unlock();

    SECTION("timeout")
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        lock.lock();
        g_cv.wait(lock, [&] { return g_notify; });
        REQUIRE(g_invokeState == 2);
        lock.unlock();

        sm.stop();
    }

    SECTION("no timeout")
    {
        sm.stop();

        lock.lock();
        g_cv.wait(lock, [&] { return g_notify; });
        REQUIRE(g_invokeState == 2);
        lock.unlock();
    }
}

TEST_CASE("invoked action is left when state machine is destructed",
          "[threadedstate]")
{
    g_notify = false;
    g_invokeState = 0;
    std::unique_lock<std::mutex> lock(g_mutex, std::defer_lock);
    std::unique_ptr<WaitingState<0>> s1;

    {
        StateMachine_t sm;
        // The state must be destructed after sm's destructor has been called.
        s1.reset(new WaitingState<0>("s1", &sm));

        sm.start();

        lock.lock();
        g_cv.wait(lock, [&] { return g_notify; });
        REQUIRE(g_invokeState == 1);
        g_notify = false;
        lock.unlock();
    }

    lock.lock();
    g_cv.wait(lock, [&] { return g_notify; });
    REQUIRE(g_invokeState == 2);
    lock.unlock();
}
