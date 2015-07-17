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

using StateMachine_t = StateMachine<>;
using State_t = StateMachine_t::state_type;

SCENARIO("different child modes of the root state", "[behavior]")
{
    StateMachine_t sm;
    State_t a("a", &sm);
    State_t b("b", &sm);
    State_t c("c", &sm);

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
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
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
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
                }
            }
        }
    }
}

SCENARIO("different child modes of an intermediary state", "[behavior]")
{
    StateMachine_t sm;
    State_t a("a", &sm);
    State_t b("b", &a);
    State_t c("c", &a);
    State_t d("d", &sm);
    State_t e("e", &d);
    State_t f("f", &d);

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
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
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
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
                }
            }
        }
    }
}

SCENARIO("different child modes of a leaf state", "[behavior]")
{
    StateMachine_t sm;
    State_t a("a", &sm);
    State_t b("b", &sm);

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
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
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
            }

            WHEN ("the FSM is stopped")
            {
                sm.stop();
                THEN ("no state is active")
                {
                    REQUIRE(!sm.running());
                    REQUIRE(isActive(sm, {}));
                }
            }
        }
    }
}
