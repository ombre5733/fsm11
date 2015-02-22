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

using namespace fsm11;

using StateMachine_t = StateMachine<>;
using State_t = StateMachine_t::state_type;

TEST_CASE("start fsm with compound root state", "[compound]")
{
    StateMachine_t sm;

    State_t a("a", &sm);
    State_t b("b", &sm);
    State_t c("c", &sm);

    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(sm.isCompound());

    sm.start();
    REQUIRE(sm.running());
    REQUIRE(sm.isActive());
    REQUIRE(a.isActive());
    REQUIRE(!b.isActive());
    REQUIRE(!c.isActive());

    sm.stop();
    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(!a.isActive());
}

TEST_CASE("start fsm with compound top-level state", "[compound]")
{
    StateMachine_t sm;

    State_t a("a", &sm);
    State_t b("b", &sm);
    State_t c("c", &a);
    State_t d("d", &a);

    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(a.isCompound());

    sm.start();
    REQUIRE(sm.running());
    REQUIRE(sm.isActive());
    REQUIRE(a.isActive());
    REQUIRE(!b.isActive());
    REQUIRE(c.isActive());
    REQUIRE(!d.isActive());

    sm.stop();
    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(!a.isActive());
    REQUIRE(!c.isActive());
}

TEST_CASE("start fsm with parallel root state", "[parallel]")
{
    StateMachine_t sm;
    sm.setChildMode(StateMachine_t::Parallel);

    State_t a("a", &sm);
    State_t b("b", &sm);
    State_t c("c", &sm);

    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(sm.isParallel());

    sm.start();
    REQUIRE(sm.running());
    REQUIRE(sm.isActive());
    REQUIRE(a.isActive());
    REQUIRE(b.isActive());
    REQUIRE(c.isActive());

    sm.stop();
    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(!a.isActive());
    REQUIRE(!b.isActive());
    REQUIRE(!c.isActive());
}

TEST_CASE("start fsm with parallel top-level state", "[parallel]")
{
    StateMachine_t sm;

    State_t a("a", &sm);
    a.setChildMode(State_t::Parallel);
    State_t b("b", &sm);
    State_t c("c", &a);
    State_t d("d", &a);

    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(a.isParallel());

    sm.start();
    REQUIRE(sm.running());
    REQUIRE(sm.isActive());
    REQUIRE(a.isActive());
    REQUIRE(!b.isActive());
    REQUIRE(c.isActive());
    REQUIRE(d.isActive());

    sm.stop();
    REQUIRE(!sm.running());
    REQUIRE(!sm.isActive());
    REQUIRE(!a.isActive());
    REQUIRE(!c.isActive());
    REQUIRE(!d.isActive());
}
