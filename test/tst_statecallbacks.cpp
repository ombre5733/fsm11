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

using StateMachine_t = fsm11::StateMachine<fsm11::StateCallbacksEnable<true>>;
using State_t = fsm11::State<StateMachine_t>;

TEST_CASE("create state machine with state callbacks", "[callbacks]")
{
    StateMachine_t sm;
}

TEST_CASE("entry and exit callbacks are invoked", "[callbacks]")
{
    StateMachine_t sm;
    State_t a("a", &sm);
    State_t aa("aa", &a);
    State_t b("b", &sm);

    std::set<State_t*> states;
    SECTION("entry callback")
    {
        sm.setStateEntryCallback([&](State_t* s){ states.insert(s); });
        sm.start();
        REQUIRE(states.size() == 3);
        sm.stop();
        REQUIRE(states.size() == 3);
    }

    SECTION("exit callback")
    {
        sm.setStateExitCallback([&](State_t* s){ states.insert(s); });
        sm.start();
        REQUIRE(states.size() == 0);
        sm.stop();
        REQUIRE(states.size() == 3);
    }

    auto contains = [&](State_t* s){ return states.find(s) != states.end(); };
    REQUIRE(contains(&sm));
    REQUIRE(contains(&a));
    REQUIRE(contains(&aa));
}
