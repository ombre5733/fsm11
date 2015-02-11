#include "catch.hpp"

#include "../threadedstate.hpp"
#include "../statemachine.hpp"

#include <mutex>
#include <thread>

using StateMachine_t = fsm11::StateMachine<>;
using State_t = fsm11::ThreadedState<StateMachine_t>;

std::mutex g_mutex;
std::thread::id g_id;

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
