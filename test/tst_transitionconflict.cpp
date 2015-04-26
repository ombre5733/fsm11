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

SCENARIO("transition conflicts", "[conflicts]")
{
    GIVEN ("that the transition selection stops after the first match")
    {
        using StateMachine_t = StateMachine<>;
        using State_t = State<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        sm += a + event(1) > b;
        sm += a + event(1) > c;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));

        WHEN ("a")
        {
            sm.addEvent(1);
            THEN ("b")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
            }
        }
    }

    GIVEN ("that all transitions of a state are scanned")
    {
        using StateMachine_t = StateMachine<TransitionSelectionStopsAfterFirstMatch<false>>;
        using State_t = State<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        sm += a + event(1) > b;
        sm += a + event(1) > c;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));

        WHEN ("a")
        {
            sm.addEvent(1);
            THEN ("b")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
            }
        }
    }
}
