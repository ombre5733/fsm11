/*******************************************************************************
  The MIT License (MIT)

  Copyright (c) 2015 Manuel Freiberger

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*******************************************************************************/

#include "catch.hpp"

#include "../src/statemachine.hpp"
#include "testutils.hpp"

using namespace fsm11;
using namespace std;

namespace syncSM
{
using StateMachine_t = fsm11::StateMachine<>;
using State_t = StateMachine_t::state_type;
} // namespace syncSM

namespace asyncSM
{
using StateMachine_t = StateMachine<AsynchronousEventDispatching,
                                    ConfigurationChangeCallbacksEnable<true>>;
using State_t = StateMachine_t::state_type;
} // namespace asyncSM

SCENARIO("different child modes of the root state", "[behavior]")
{
    using namespace syncSM;

    StateMachine_t sm;
    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> b("b", &sm);
    TrackingState<State_t> c("c", &sm);

    GIVEN ("an FSM with plain compound root state")
    {
        REQUIRE(sm.isCompound());

        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));

        WHEN ("the FSM is started")
        {
            sm.start();
            THEN ("exactly one child is active")
            {
                REQUIRE(sm.running());
                REQUIRE(isActive(sm, {&sm, &a}));
                REQUIRE(a == make_tuple(1, 0, 1, 0));
                REQUIRE(b == make_tuple(0, 0, 0, 0));
                REQUIRE(c == make_tuple(0, 0, 0, 0));
                REQUIRE(sm.numConfigurationChanges() == 1);
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
                    REQUIRE(a == make_tuple(Y, Y, Z, Z));
                    REQUIRE(b == make_tuple(Y, Y, Z, Z));
                    REQUIRE(c == make_tuple(Y, Y, Z, Z));
                    REQUIRE(sm.numConfigurationChanges() == 2);
                }
            }
        }
    }

    GIVEN ("an FSM with parallel root state")
    {
        sm.setChildMode(ChildMode::Parallel);
        REQUIRE(sm.isParallel());

        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));

        WHEN ("the FSM is started")
        {
            sm.start();
            THEN ("all children are active")
            {
                REQUIRE(sm.running());
                REQUIRE(isActive(sm, {&sm, &a, &b, &c}));
                REQUIRE(a == make_tuple(1, 0, 1, 0));
                REQUIRE(b == make_tuple(1, 0, 1, 0));
                REQUIRE(c == make_tuple(1, 0, 1, 0));
                REQUIRE(sm.numConfigurationChanges() == 1);
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
                    REQUIRE(a == make_tuple(Y, Y, Z, Z));
                    REQUIRE(b == make_tuple(Y, Y, Z, Z));
                    REQUIRE(c == make_tuple(Y, Y, Z, Z));
                    REQUIRE(sm.numConfigurationChanges() == 2);
                }
            }
        }
    }
}

SCENARIO("different child modes of an intermediary state", "[behavior]")
{
    using namespace syncSM;

    StateMachine_t sm;
    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> b("b", &a);
    TrackingState<State_t> c("c", &a);

    GIVEN ("a plain compound intermediary state")
    {
        REQUIRE(a.isCompound());

        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));

        WHEN ("the FSM is started")
        {
            sm.start();
            THEN ("exactly one child is active")
            {
                REQUIRE(sm.running());
                REQUIRE(isActive(sm, {&sm, &a, &b}));
                REQUIRE(a == make_tuple(1, 0, 1, 0));
                REQUIRE(b == make_tuple(1, 0, 1, 0));
                REQUIRE(c == make_tuple(0, 0, 0, 0));
                REQUIRE(sm.numConfigurationChanges() == 1);
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
                    REQUIRE(a == make_tuple(Y, Y, Z, Z));
                    REQUIRE(b == make_tuple(Y, Y, Z, Z));
                    REQUIRE(c == make_tuple(Y, Y, Z, Z));
                    REQUIRE(sm.numConfigurationChanges() == 2);
                }
            }
        }
    }

    GIVEN ("a parallel intermediary state")
    {
        a.setChildMode(ChildMode::Parallel);
        REQUIRE(a.isParallel());

        WHEN ("the FSM is started")
        {
            sm.start();
            THEN ("all children are active")
            {
                REQUIRE(sm.running());
                REQUIRE(isActive(sm, {&sm, &a, &b, &c}));
                REQUIRE(a == make_tuple(1, 0, 1, 0));
                REQUIRE(b == make_tuple(1, 0, 1, 0));
                REQUIRE(c == make_tuple(1, 0, 1, 0));
                REQUIRE(sm.numConfigurationChanges() == 1);
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
                    REQUIRE(a == make_tuple(Y, Y, Z, Z));
                    REQUIRE(b == make_tuple(Y, Y, Z, Z));
                    REQUIRE(c == make_tuple(Y, Y, Z, Z));
                    REQUIRE(sm.numConfigurationChanges() == 2);
                }
            }
        }
    }
}

SCENARIO("different child modes of a leaf state", "[behavior]")
{
    using namespace syncSM;

    StateMachine_t sm;
    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> b("b", &sm);

    GIVEN ("a plain compound leaf state")
    {
        REQUIRE(a.childMode() == ChildMode::Exclusive);
        REQUIRE(a.isAtomic());

        REQUIRE(!sm.running());
        REQUIRE(isActive(sm, {}));

        WHEN ("the FSM is started")
        {
            sm.start();
            THEN ("the leaf is active")
            {
                REQUIRE(sm.running());
                REQUIRE(isActive(sm, {&sm, &a}));
                REQUIRE(a == make_tuple(1, 0, 1, 0));
                REQUIRE(b == make_tuple(0, 0, 0, 0));
                REQUIRE(sm.numConfigurationChanges() == 1);
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
                    REQUIRE(a == make_tuple(Y, Y, Z, Z));
                    REQUIRE(b == make_tuple(Y, Y, Z, Z));
                    REQUIRE(sm.numConfigurationChanges() == 2);
                }
            }
        }
    }

    GIVEN ("a parallel leaf state")
    {
        a.setChildMode(ChildMode::Parallel);
        REQUIRE(a.childMode() == ChildMode::Parallel);
        REQUIRE(a.isAtomic());

        WHEN ("the FSM is started")
        {
            sm.start();
            THEN ("the leaf is active")
            {
                REQUIRE(sm.running());
                REQUIRE(isActive(sm, {&sm, &a}));
                REQUIRE(a == make_tuple(1, 0, 1, 0));
                REQUIRE(b == make_tuple(0, 0, 0, 0));
                REQUIRE(sm.numConfigurationChanges() == 1);
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
                    REQUIRE(a == make_tuple(Y, Y, Z, Z));
                    REQUIRE(b == make_tuple(Y, Y, Z, Z));
                    REQUIRE(sm.numConfigurationChanges() == 2);
                }
            }
        }
    }
}

// TODO
SCENARIO("simple configuration changes in synchronous statemachine",
         "[behavior]")
{
    GIVEN ("a synchronous state machine")
    {
        using namespace syncSM;
        StateMachine_t sm;

        TrackingState<State_t> a("a", &sm);
        TrackingState<State_t> aa("aa", &a);
        TrackingState<State_t> ab("ab", &a);
        TrackingState<State_t> b("b", &sm);
        TrackingState<State_t> ba("ba", &b);
        TrackingState<State_t> bb("bb", &b);

        sm += aa + event(2) > ba;
        sm += ba + event(2) > bb;
        sm += a  + event(3) > bb;
        sm += b  + event(3) > ab;
        sm += aa + event(4) > b;
        sm += ba + event(4) > a;
        sm += a  + event(5) > ab;
        sm += ab + event(6) > a;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 0);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 0);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 0);

        SECTION("from atomic to atomic")
        {
            sm.addEvent(2);
            REQUIRE(isActive(sm, {&sm, &b, &ba}));
            REQUIRE(a.entered == 1);
            REQUIRE(a.left == 1);
            REQUIRE(aa.entered == 1);
            REQUIRE(aa.left == 1);
            REQUIRE(ab.entered == 0);
            REQUIRE(b.entered == 1);
            REQUIRE(b.left == 0);
            REQUIRE(ba.entered == 1);
            REQUIRE(ba.left == 0);
            REQUIRE(bb.entered == 0);

            sm.addEvent(2);
            REQUIRE(isActive(sm, {&sm, &b, &bb}));
            REQUIRE(b.entered == 1);
            REQUIRE(b.left == 0);
            REQUIRE(ba.entered == 1);
            REQUIRE(ba.left == 1);
            REQUIRE(bb.entered == 1);
            REQUIRE(bb.left == 0);
        }

        SECTION("from compound to atomic")
        {
            sm.addEvent(3);
            REQUIRE(isActive(sm, {&sm, &b, &bb}));
            REQUIRE(a.entered == 1);
            REQUIRE(a.left == 1);
            REQUIRE(aa.entered == 1);
            REQUIRE(aa.left == 1);
            REQUIRE(ab.entered == 0);
            REQUIRE(b.entered == 1);
            REQUIRE(b.left == 0);
            REQUIRE(ba.entered == 0);
            REQUIRE(bb.entered == 1);
            REQUIRE(bb.left == 0);

            sm.addEvent(3);
            REQUIRE(isActive(sm, {&sm, &a, &ab}));
            REQUIRE(b.entered == 1);
            REQUIRE(b.left == 1);
            REQUIRE(bb.entered == 1);
            REQUIRE(bb.left == 1);
            REQUIRE(a.entered == 2);
            REQUIRE(a.left == 1);
            REQUIRE(ab.entered == 1);
            REQUIRE(ab.left == 0);
        }

        SECTION("from atomic to compound")
        {
            sm.addEvent(4);
            REQUIRE(isActive(sm, {&sm, &b, &ba}));
            REQUIRE(a.entered == 1);
            REQUIRE(a.left == 1);
            REQUIRE(aa.entered == 1);
            REQUIRE(aa.left == 1);
            REQUIRE(ab.entered == 0);
            REQUIRE(b.entered == 1);
            REQUIRE(b.left == 0);
            REQUIRE(ba.entered == 1);
            REQUIRE(ba.left == 0);
            REQUIRE(bb.entered == 0);

            sm.addEvent(4);
            REQUIRE(isActive(sm, {&sm, &a, &aa}));
            REQUIRE(b.entered == 1);
            REQUIRE(b.left == 1);
            REQUIRE(ba.entered == 1);
            REQUIRE(ba.left == 1);
            REQUIRE(a.entered == 2);
            REQUIRE(a.left == 1);
            REQUIRE(aa.entered == 2);
            REQUIRE(aa.left == 1);
            REQUIRE(ab.entered == 0);
        }

        SECTION("between ancestor and descendant")
        {
            sm.addEvent(5);
            REQUIRE(isActive(sm, {&sm, &a, &ab}));
            REQUIRE(a.entered == 2);
            REQUIRE(a.left == 1);
            REQUIRE(aa.entered == 1);
            REQUIRE(aa.left == 1);
            REQUIRE(ab.entered == 1);
            REQUIRE(ab.left == 0);
            REQUIRE(b.entered == 0);

            sm.addEvent(6);
            REQUIRE(isActive(sm, {&sm, &a, &aa}));
            REQUIRE(a.entered == 3);
            REQUIRE(a.left == 2);
            REQUIRE(aa.entered == 2);
            REQUIRE(aa.left == 1);
            REQUIRE(ab.entered == 1);
            REQUIRE(ab.left == 1);
            REQUIRE(b.entered == 0);
        }

        sm.stop();
        REQUIRE(isActive(sm, {}));
        for (auto state : {&a, &aa, &ab, &b, &ba, &bb})
            REQUIRE(state->entered == state->left);
    }
}

// TODO
TEST_CASE("simple configuration changes in asynchronous statemachine",
          "[transition]")
{
    std::future<void> result;

    std::mutex mutex;
    bool configurationChanged = false;
    std::condition_variable cv;

    auto waitForConfigurationChange = [&] {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&] { return configurationChanged; });
        configurationChanged = false;
    };

    using namespace asyncSM;
    StateMachine_t sm;

    sm.setConfigurationChangeCallback([&] {
        std::unique_lock<std::mutex> lock(mutex);
        configurationChanged = true;
        cv.notify_all();
    });

    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> aa("aa", &a);
    TrackingState<State_t> ab("ab", &a);
    TrackingState<State_t> b("b", &sm);
    TrackingState<State_t> ba("ba", &b);
    TrackingState<State_t> bb("bb", &b);

    sm += aa + event(2) > ba;
    sm += ba + event(2) > bb;
    sm += a  + event(3) > bb;
    sm += b  + event(3) > ab;
    sm += aa + event(4) > b;
    sm += ba + event(4) > a;
    sm += a  + event(5) > ab;
    sm += ab + event(6) > a;

    result = sm.startAsyncEventLoop();

    sm.start();
    waitForConfigurationChange();
    REQUIRE(isActive(sm, {&sm, &a, &aa}));
    REQUIRE(a.entered == 1);
    REQUIRE(a.left == 0);
    REQUIRE(aa.entered == 1);
    REQUIRE(aa.left == 0);
    REQUIRE(ab.entered == 0);
    REQUIRE(b.entered == 0);
    SECTION("from atomic to atomic")
    {
        sm.addEvent(2);
        waitForConfigurationChange();
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 0);
        REQUIRE(bb.entered == 0);

        sm.addEvent(2);
        waitForConfigurationChange();
        REQUIRE(isActive(sm, {&sm, &b, &bb}));
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 1);
        REQUIRE(bb.entered == 1);
        REQUIRE(bb.left == 0);
    }

    SECTION("from compound to atomic")
    {
        sm.addEvent(3);
        waitForConfigurationChange();
        REQUIRE(isActive(sm, {&sm, &b, &bb}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 0);
        REQUIRE(bb.entered == 1);
        REQUIRE(bb.left == 0);

        sm.addEvent(3);
        waitForConfigurationChange();
        REQUIRE(isActive(sm, {&sm, &a, &ab}));
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 1);
        REQUIRE(bb.entered == 1);
        REQUIRE(bb.left == 1);
        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
    }

    SECTION("from atomic to compound")
    {
        sm.addEvent(4);
        waitForConfigurationChange();
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 0);
        REQUIRE(bb.entered == 0);

        sm.addEvent(4);
        waitForConfigurationChange();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 1);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 1);
        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 2);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
    }

    SECTION("between ancestor and descendant")
    {
        sm.addEvent(5);
        waitForConfigurationChange();
        REQUIRE(isActive(sm, {&sm, &a, &ab}));
        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
        REQUIRE(b.entered == 0);

        sm.addEvent(6);
        waitForConfigurationChange();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        REQUIRE(a.entered == 3);
        REQUIRE(a.left == 2);
        REQUIRE(aa.entered == 2);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 1);
        REQUIRE(b.entered == 0);
    }

    sm.stop();
    waitForConfigurationChange();
    REQUIRE(isActive(sm, {}));
    for (auto state : {&a, &aa, &ab, &b, &ba, &bb})
        REQUIRE(state->entered == state->left);
}

SCENARIO("no invoke in case of an eventless transition", "[behavior]")
{
    GIVEN ("an FSM")
    {
        using namespace syncSM;

        StateMachine_t sm;
        TrackingState<State_t> a("a", &sm);
        TrackingState<State_t> b("b", &sm);
        TrackingState<State_t> c("c", &sm);

        sm += a + event(1) > b;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        REQUIRE(a == make_tuple(1, 0, 1, 0));
        REQUIRE(sm.numConfigurationChanges() == 1);

        WHEN ("no event-less transition is present")
        {
            sm += b + event(1) > c;
            sm.addEvent(1);

            THEN ("the do-action is invoked immediately")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
                REQUIRE(b == make_tuple(1, 0, 1, 0));
                REQUIRE(c == make_tuple(0, 0, 0, 0));
            }

            REQUIRE(a == make_tuple(1, 1, 1, 1));
            REQUIRE(sm.numConfigurationChanges() == 2);
        }

        WHEN ("an event-less transition is present")
        {
            sm += b + noEvent  > c;
            sm.addEvent(1);

            THEN ("the do-action is only invoked after run-to-completion")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
                REQUIRE(b == make_tuple(1, 1, 0, 0));
                REQUIRE(c == make_tuple(1, 0, 1, 0));
            }

            REQUIRE(a == make_tuple(1, 1, 1, 1));
            REQUIRE(sm.numConfigurationChanges() == 2);
        }

        WHEN ("a guarded event-less transition is present")
        {
            bool enabled = true;
            sm += b + noEvent ([&] (int) { return enabled; }) > c;

            WHEN ("the guard evaluates to true")
            {
                enabled = true;
                sm.addEvent(1);
                THEN ("the do-action is only invoked after run-to-completion")
                {
                    REQUIRE(isActive(sm, {&sm, &c}));
                    REQUIRE(b == make_tuple(1, 1, 0, 0));
                    REQUIRE(c == make_tuple(1, 0, 1, 0));
                }
            }

            WHEN ("the guard evaluates to false")
            {
                enabled = false;
                sm.addEvent(1);
                THEN ("the do-action is invoked immediately")
                {
                    REQUIRE(isActive(sm, {&sm, &b}));
                    REQUIRE(b == make_tuple(1, 0, 1, 0));
                    REQUIRE(c == make_tuple(0, 0, 0, 0));
                }
            }

            REQUIRE(a == make_tuple(1, 1, 1, 1));
            REQUIRE(sm.numConfigurationChanges() == 2);
        }

        sm.stop();
        REQUIRE(sm.numConfigurationChanges() == 3);
        REQUIRE(a == make_tuple(Y, Y, Z, Z));
        REQUIRE(b == make_tuple(Y, Y, Z, Z));
        REQUIRE(c == make_tuple(Y, Y, Z, Z));
    }
}

SCENARIO("history state", "[behavior]")
{
    GIVEN ("an FSM with a history state")
    {
        using namespace syncSM;
        using HistoryState_t = State_t; // TODO: correct this

        StateMachine_t sm;
        TrackingState<HistoryState_t> s1("s1", &sm);
        TrackingState<State_t> s11("s11", &s1);
        TrackingState<State_t> s111("s111", &s11);
        TrackingState<State_t> s112("s112", &s11);
        TrackingState<State_t> s12("s12", &s1);
        TrackingState<State_t> s121("s121", &s12);
        TrackingState<State_t> s122("s122", &s12);
        TrackingState<State_t> s13("s13", &s1);
        TrackingState<State_t> s131("s131", &s13);
        TrackingState<State_t> s132("s132", &s13);
        TrackingState<State_t> s2("s2", &sm);

        WHEN ("the history state is re-entered")
        {
            sm += s11 + event(1) > s12;
            sm += s12 + event(2) > s2;
            sm += s2 + event(3) > s1;

            sm.start();
            REQUIRE(isActive(sm, {&sm, &s1, &s11, &s111}));

            sm.addEvent(1);
            REQUIRE(isActive(sm, {&sm, &s1, &s12, &s121}));

            sm.addEvent(2);
            REQUIRE(isActive(sm, {&sm, &s2}));

            sm.addEvent(3);
            THEN ("the latest active state is activated")
            {
                REQUIRE(isActive(sm, {&sm, &s1, &s12, &s121}));
            }
        }

        // TODO:
        // - initial state in history state only followed after startup
        // - if transition targets a more specific state, history has no effect
    }
}
