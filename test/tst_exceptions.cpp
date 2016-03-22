/*******************************************************************************
  fsm11 - A C++ library for finite state machines

  Copyright (c) 2015-2016, Manuel Freiberger
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  - Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
  - Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include "catch.hpp"

#include "../src/functionstate.hpp"
#include "../src/statemachine.hpp"

#include "testutils.hpp"

#include <deque>
#include <future>
#include <tuple>

using namespace fsm11;

struct ActionException
{
};

struct GuardException
{
};

struct InvokeException
{
};

struct ListException
{
};

struct StateException
{
};

template <typename TState>
class ThrowingInvokeState : public TState
{
public:
    ThrowingInvokeState(const char* name, TState* parent)
        : TState(name, parent)
    {
    }

    virtual void enterInvoke() override
    {
        if (throwInEnterInvoke)
            throw InvokeException();
    }

    virtual void exitInvoke() override
    {
        if (throwInExitInvoke)
            throw InvokeException();
    }

    bool throwInEnterInvoke{false};
    bool throwInExitInvoke{false};
};

class ThrowingList
{
public:
    bool empty() const
    {
        return m_events.empty();
    }

    int& front()
    {
        return m_events.front();
    }

    void pop_front()
    {
        m_events.pop_front();
    }

    void push_back(int event)
    {
        if (event == 1)
            throw ListException();
        m_events.push_back(event);
    }

private:
    std::deque<int> m_events;
};

namespace syncSM
{
using StateMachine_t = StateMachine<EventListType<ThrowingList>>;
using State_t = StateMachine_t::state_type;
using FunctionState_t = FunctionState<StateMachine_t>;
} // namespace syncSM

namespace asyncSM
{
using StateMachine_t = StateMachine<AsynchronousEventDispatching,
                                    EventListType<ThrowingList>,
                                    ConfigurationChangeCallbacksEnable<true>>;
using State_t = StateMachine_t::state_type;
using FunctionState_t = FunctionState<StateMachine_t>;
} // namespace asyncSM


SCENARIO("throw an exception from the event list", "[exception]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;
        StateMachine_t sm;

        State_t a("a", &sm);
        State_t aa("aa", &a);
        State_t ab("ab", &a);
        State_t b("b", &sm);
        State_t ba("ba", &b);
        State_t bb("bb", &b);

        sm += aa + event(0) > ba;

        REQUIRE(isActive(sm, {}));
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));

        WHEN ("an exception is thrown from the event list")
        {
            REQUIRE_THROWS_AS(sm.addEvent(1), ListException);
            THEN ("the state machine does not change its configuration")
            {
                REQUIRE(isActive(sm, {&sm, &a, &aa}));
            }
        }

        WHEN ("an exception is thrown after a transition")
        {
            sm.addEvent(0);
            REQUIRE(isActive(sm, {&sm, &b, &ba}));
            REQUIRE_THROWS_AS(sm.addEvent(1), ListException);
            THEN ("the state machine does not change its configuration")
            {
                REQUIRE(isActive(sm, {&sm, &b, &ba}));
            }
        }

        // The state machine must still be running.
        REQUIRE(sm.running());
    }

    GIVEN ("an asynchronous FSM")
    {
        using namespace asyncSM;

        std::future<void> result;
        StateMachine_t sm;
        ConfigurationChangeTracker<StateMachine_t> cct(sm);

        State_t a("a", &sm);
        State_t aa("aa", &a);
        State_t ab("ab", &a);
        State_t b("b", &sm);
        State_t ba("ba", &b);
        State_t bb("bb", &b);

        sm += aa + event(0) > ba;

        result = std::async(std::launch::async, [&] { sm.eventLoop(); });
        REQUIRE(isActive(sm, {}));
        sm.start();
        cct.wait();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));

        WHEN ("an exception is thrown from the event list")
        {
            REQUIRE_THROWS_AS(sm.addEvent(1), ListException);
            THEN ("the state machine does not change its configuration")
            {
                REQUIRE(isActive(sm, {&sm, &a, &aa}));
            }
        }

        WHEN ("an exception is thrown after a transition")
        {
            sm.addEvent(0);
            cct.wait();
            REQUIRE(isActive(sm, {&sm, &b, &ba}));
            REQUIRE_THROWS_AS(sm.addEvent(1), ListException);
            THEN ("the state machine does not change its configuration")
            {
                REQUIRE(isActive(sm, {&sm, &b, &ba}));
            }
        }

        // The state machine must still be running.
        REQUIRE(sm.running());
    }
}

SCENARIO("throw an exception in a transition guard", "[exception]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;
        StateMachine_t sm;

        State_t a("a", &sm);
        State_t aa("aa", &a);
        State_t ab("ab", &a);
        State_t b("b", &sm);
        State_t ba("ba", &b);
        State_t bb("bb", &b);

        auto guard = [](int event) {
            if (event == 3)
                throw GuardException();
            return event % 2 == 0;
        };

        int actionEvent = -1;
        auto action = [&](int event) {
            actionEvent = event;
        };

        sm += aa + event(0) [guard] / action > ba;
        sm += aa + event(3) [guard] / action > ba;
        sm += ba + event(3) [guard] / action > bb;

        REQUIRE(isActive(sm, {}));
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));

        WHEN ("the guard throws an exception")
        {
            REQUIRE_THROWS_AS(sm.addEvent(3), GuardException);
            THEN ("the action is not called")
            {
                REQUIRE(actionEvent == -1);
            }
        }

        WHEN ("the guard throws after a transition")
        {
            sm.addEvent(0);
            REQUIRE(isActive(sm, {&sm, &b, &ba}));
            REQUIRE_THROWS_AS(sm.addEvent(3), GuardException);
            THEN ("the guard is not called")
            {
                REQUIRE(actionEvent == 0);
            }
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

    GIVEN ("an asynchronous FSM")
    {
        using namespace asyncSM;

        std::future<void> result;
        StateMachine_t sm;
        ConfigurationChangeTracker<StateMachine_t> cct(sm);

        State_t a("a", &sm);
        State_t aa("aa", &a);
        State_t ab("ab", &a);
        State_t b("b", &sm);
        State_t ba("ba", &b);
        State_t bb("bb", &b);

        auto guard = [](int event) {
            if (event == 3)
                throw GuardException();
            return event % 2 == 0;
        };

        int actionEvent = -1;
        auto action = [&](int event) {
            actionEvent = event;
        };

        sm += aa + event(0) [guard] / action > ba;
        sm += aa + event(3) [guard] / action > ba;
        sm += ba + event(3) [guard] / action > bb;

        result = std::async(std::launch::async, [&] { sm.eventLoop(); });
        REQUIRE(isActive(sm, {}));
        sm.start();
        cct.wait();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));

        WHEN ("the guard throws an exception")
        {
            sm.addEvent(3);
            cct.wait();
            REQUIRE_THROWS_AS(result.get(), GuardException);
            THEN ("the action is not called")
            {
                REQUIRE(actionEvent == -1);
            }
        }

        WHEN ("the guard throws after a transition")
        {
            sm.addEvent(0);
            cct.wait();
            REQUIRE(isActive(sm, {&sm, &b, &ba}));
            sm.addEvent(3);
            cct.wait();
            REQUIRE_THROWS_AS(result.get(), GuardException);
            THEN ("the action is not called")
            {
                REQUIRE(actionEvent == 0);
            }
        }

        // The state machine must have been stopped.
        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));

        // Restart the state machine.
        result = std::async(std::launch::async, [&] { sm.eventLoop(); });
        sm.start();
        cct.wait();

        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        sm.addEvent(0);
        cct.wait();
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        sm.stop();
        cct.wait();

        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));
    }
}

SCENARIO("throw an exception in a transition action", "[exception]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;
        StateMachine_t sm;

        State_t a("a", &sm);
        State_t aa("aa", &a);
        State_t ab("ab", &a);
        State_t b("b", &sm);
        State_t ba("ba", &b);
        State_t bb("bb", &b);

        auto action = [](int event) {
            if (event == 3)
                throw ActionException();
        };

        sm += aa + event(0) / action > ba;
        sm += aa + event(3) / action > ba;
        sm += ba + event(3) / action > bb;

        REQUIRE(isActive(sm, {}));
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));

        WHEN ("an event is added")
        {
            THEN ("the transition action throws")
            {
                REQUIRE_THROWS_AS(sm.addEvent(3), ActionException);
            }
        }

        WHEN ("an event is added after a transition")
        {
            sm.addEvent(0);
            REQUIRE(isActive(sm, {&sm, &b, &ba}));
            THEN ("the transition action throws")
            {
                REQUIRE_THROWS_AS(sm.addEvent(3), ActionException);
            }
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

    GIVEN ("an asynchronous FSM")
    {
        using namespace asyncSM;

        std::future<void> result;
        StateMachine_t sm;
        ConfigurationChangeTracker<StateMachine_t> cct(sm);

        State_t a("a", &sm);
        State_t aa("aa", &a);
        State_t ab("ab", &a);
        State_t b("b", &sm);
        State_t ba("ba", &b);
        State_t bb("bb", &b);

        auto action = [](int event) {
            if (event == 3)
                throw ActionException();
        };

        sm += aa + event(0) / action > ba;
        sm += aa + event(3) / action > ba;
        sm += ba + event(3) / action > bb;

        result = std::async(std::launch::async, [&] { sm.eventLoop(); });
        REQUIRE(isActive(sm, {}));
        sm.start();
        cct.wait();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));

        WHEN ("an event is added")
        {
            sm.addEvent(3);
            cct.wait();
            THEN ("the transition action throws")
            {
                REQUIRE_THROWS_AS(result.get(), ActionException);
            }
        }

        WHEN ("an event is added after a transition")
        {
            sm.addEvent(0);
            cct.wait();
            REQUIRE(isActive(sm, {&sm, &b, &ba}));
            sm.addEvent(3);
            cct.wait();
            THEN ("the transition action throws")
            {
                REQUIRE_THROWS_AS(result.get(), ActionException);
            }
        }

        // The state machine must have been stopped.
        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));

        // Restart the state machine.
        result = std::async(std::launch::async, [&] { sm.eventLoop(); });
        sm.start();
        cct.wait();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        sm.addEvent(0);
        cct.wait();
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        sm.stop();
        cct.wait();

        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));
    }
}

SCENARIO("throw an exception in a state entry function", "[exception]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;

        StateMachine_t sm;

        using TrackingState_t = TrackingState<FunctionState_t>;
        TrackingState_t a("a", &sm);
        TrackingState_t aa("aa", &a);
        State_t ab("ab", &a);
        State_t b("b", &sm);
        State_t ba("ba", &b);
        State_t bb("bb", &b);

        WHEN ("an ancestor throws upon entry")
        {
            a.setEntryFunction([](int) { throw StateException(); });

            REQUIRE_THROWS_AS(sm.start(), StateException);

            THEN ("the child is not entered")
            {
                REQUIRE(a == std::make_tuple(1, 0, 0, 0));
                REQUIRE(aa == std::make_tuple(0, 0, 0, 0));
            }
        }

        WHEN ("a leaf state throws upon entry")
        {
            aa.setEntryFunction([](int) { throw StateException(); });

            REQUIRE_THROWS_AS(sm.start(), StateException);

            THEN ("the parent is entered and left")
            {
                REQUIRE(a == std::make_tuple(1, 1, 0, 0));
                REQUIRE(aa == std::make_tuple(1, 0, 0, 0));
            }
        }

        // The state machine must have been stopped.
        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));
    }

    GIVEN ("an asynchronous FSM")
    {
        using namespace asyncSM;

        std::future<void> result;
        StateMachine_t sm;
        ConfigurationChangeTracker<StateMachine_t> cct(sm);

        using TrackingState_t = TrackingState<FunctionState_t>;
        TrackingState_t a("a", &sm);
        TrackingState_t aa("aa", &a);
        State_t ab("ab", &a);
        State_t b("b", &sm);
        State_t ba("ba", &b);
        State_t bb("bb", &b);

        WHEN ("an ancestor throws upon entry")
        {
            a.setEntryFunction([](int) { throw StateException(); });

            result = std::async(std::launch::async, [&] { sm.eventLoop(); });
            sm.start();
            cct.wait();
            REQUIRE_THROWS_AS(result.get(), StateException);

            THEN ("the child is not entered")
            {
                REQUIRE(a == std::make_tuple(1, 0, 0, 0));
                REQUIRE(aa == std::make_tuple(0, 0, 0, 0));
            }
        }

        WHEN ("a leaf state throws upon entry")
        {
            aa.setEntryFunction([](int) { throw StateException(); });

            result = std::async(std::launch::async, [&] { sm.eventLoop(); });
            sm.start();
            cct.wait();
            REQUIRE_THROWS_AS(result.get(), StateException);

            THEN ("the parent is entered and left")
            {
                REQUIRE(a == std::make_tuple(1, 1, 0, 0));
                REQUIRE(aa == std::make_tuple(1, 0, 0, 0));
            }
        }

        // The state machine must have been stopped.
        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));
    }
}

SCENARIO("throw an exception in a state exit function", "[exception]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;

        StateMachine_t sm;

        using TrackingState_t = TrackingState<FunctionState_t>;
        TrackingState_t a("a", &sm);
        TrackingState_t aa("aa", &a);
        State_t ab("ab", &a);
        State_t b("b", &sm);
        State_t ba("ba", &b);
        State_t bb("bb", &b);

        WHEN ("a leaf state throws upon exit")
        {
            aa.setExitFunction([](int) { throw StateException(); });

            sm.start();
            REQUIRE_THROWS_AS(sm.stop(), StateException);
            THEN ("the leaf and the parent are exited")
            {
                REQUIRE(a == std::make_tuple(1, 1, 1, 1));
                REQUIRE(aa == std::make_tuple(1, 1, 1, 1));
            }
        }

        // The state machine must have been stopped.
        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));
    }

    GIVEN ("an asynchronous FSM")
    {
        using namespace asyncSM;

        std::future<void> result;
        StateMachine_t sm;
        ConfigurationChangeTracker<StateMachine_t> cct(sm);

        using TrackingState_t = TrackingState<FunctionState_t>;
        TrackingState_t a("a", &sm);
        TrackingState_t aa("aa", &a);
        State_t ab("ab", &a);
        State_t b("b", &sm);
        State_t ba("ba", &b);
        State_t bb("bb", &b);

        WHEN ("a leaf state throws upon exit")
        {
            aa.setExitFunction([](int) { throw StateException(); });

            result = std::async(std::launch::async, [&] { sm.eventLoop(); });
            sm.start();
            cct.wait();
            sm.stop();
            cct.wait();
            REQUIRE_THROWS_AS(result.get(), StateException);
            REQUIRE(a == std::make_tuple(1, 1, 1, 1));
            REQUIRE(aa == std::make_tuple(1, 1, 1, 1));
        }

        // The state machine must have been stopped.
        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));
    }
}

SCENARIO ("throw an exception when entering the invoke action", "[exception]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;

        StateMachine_t sm;

        using TrackingState_t = TrackingState<ThrowingInvokeState<State_t>>;
        TrackingState_t a("a", &sm);
        TrackingState_t aa("aa", &a);

        WHEN ("an ancestor throws upon entering the invoke action")
        {
            a.throwInEnterInvoke = true;

            THEN ("the leaf and the parent are exited")
            {
                REQUIRE_THROWS_AS(sm.start(), InvokeException);
                REQUIRE(a == std::make_tuple(1, 1, 1, 0));
                REQUIRE(aa == std::make_tuple(1, 1, Y, Y));
            }
        }

        WHEN ("a leaf state throws upon entering the invoke action")
        {
            aa.throwInEnterInvoke = true;

            THEN ("the leaf and the parent are exited")
            {
                REQUIRE_THROWS_AS(sm.start(), InvokeException);
                REQUIRE(a == std::make_tuple(1, 1, Y, Y));
                REQUIRE(aa == std::make_tuple(1, 1, 1, 0));
            }
        }

        // The state machine must have been stopped.
        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));
    }

    GIVEN ("an asynchronous FSM")
    {
        using namespace asyncSM;

        std::future<void> result;
        StateMachine_t sm;
        ConfigurationChangeTracker<StateMachine_t> cct(sm);

        using TrackingState_t = TrackingState<ThrowingInvokeState<State_t>>;
        TrackingState_t a("a", &sm);
        TrackingState_t aa("aa", &a);

        WHEN ("an ancestor throws upon entering the invoke action")
        {
            a.throwInEnterInvoke = true;
            result = std::async(std::launch::async, [&] { sm.eventLoop(); });
            sm.start();
            cct.wait();

            THEN ("the leaf and the parent are exited")
            {
                REQUIRE_THROWS_AS(result.get(), InvokeException);
                REQUIRE(a == std::make_tuple(1, 1, 1, 0));
                REQUIRE(aa == std::make_tuple(1, 1, Y, Y));
            }
        }

        WHEN ("a leaf state throws upon entering the invoke action")
        {
            aa.throwInEnterInvoke = true;

            result = std::async(std::launch::async, [&] { sm.eventLoop(); });
            sm.start();
            cct.wait();

            THEN ("the leaf and the parent are exited")
            {
                REQUIRE_THROWS_AS(result.get(), InvokeException);
                REQUIRE(a == std::make_tuple(1, 1, Y, Y));
                REQUIRE(aa == std::make_tuple(1, 1, 1, 0));
            }
        }

        // The state machine must have been stopped.
        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));
    }
}

SCENARIO ("throw an exception when leaving the invoke action", "[exception]")
{
    GIVEN ("a synchronous FSM")
    {
        using namespace syncSM;

        StateMachine_t sm;

        using TrackingState_t = TrackingState<ThrowingInvokeState<State_t>>;
        TrackingState_t a("a", &sm);
        TrackingState_t aa("aa", &a);

        WHEN ("an ancestor throws upon leaving the invoke action")
        {
            a.throwInExitInvoke = true;
            sm.start();

            THEN ("the leaf and the parent are exited")
            {
                REQUIRE_THROWS_AS(sm.stop(), InvokeException);
                REQUIRE(a == std::make_tuple(1, 1, 1, 1));
                REQUIRE(aa == std::make_tuple(1, 1, Y, Y));
            }
        }

        WHEN ("a leaf state throws upon leaving the invoke action")
        {
            aa.throwInExitInvoke = true;
            sm.start();

            THEN ("the leaf and the parent are exited")
            {
                REQUIRE_THROWS_AS(sm.stop(), InvokeException);
                REQUIRE(a == std::make_tuple(1, 1, Y, Y));
                REQUIRE(aa == std::make_tuple(1, 1, 1, 1));
            }
        }

        // The state machine must have been stopped.
        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));
    }

    GIVEN ("an asynchronous FSM")
    {
        using namespace asyncSM;

        std::future<void> result;
        StateMachine_t sm;
        ConfigurationChangeTracker<StateMachine_t> cct(sm);

        using TrackingState_t = TrackingState<ThrowingInvokeState<State_t>>;
        TrackingState_t a("a", &sm);
        TrackingState_t aa("aa", &a);

        WHEN ("an ancestor throws upon leaving the invoke action")
        {
            a.throwInExitInvoke = true;
            result = std::async(std::launch::async, [&] { sm.eventLoop(); });
            sm.start();
            cct.wait();
            sm.stop();
            cct.wait();

            THEN ("the leaf and the parent are exited")
            {
                REQUIRE_THROWS_AS(result.get(), InvokeException);
                REQUIRE(a == std::make_tuple(1, 1, 1, 1));
                REQUIRE(aa == std::make_tuple(1, 1, Y, Y));
            }
        }

        WHEN ("a leaf state throws upon leaving the invoke action")
        {
            aa.throwInExitInvoke = true;

            result = std::async(std::launch::async, [&] { sm.eventLoop(); });
            sm.start();
            cct.wait();
            sm.stop();
            cct.wait();

            THEN ("the leaf and the parent are exited")
            {
                REQUIRE_THROWS_AS(result.get(), InvokeException);
                REQUIRE(a == std::make_tuple(1, 1, Y, Y));
                REQUIRE(aa == std::make_tuple(1, 1, 1, 1));
            }
        }

        // The state machine must have been stopped.
        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));
    }
}
