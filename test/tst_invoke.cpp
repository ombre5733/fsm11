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

using StateMachine_t = fsm11::StateMachine<>;
using State_t = fsm11::State<StateMachine_t>;

using namespace fsm11;

class InvokeChecker : public State_t
{
public:
    using State_t::State_t;

    virtual void enterInvoke() override
    {
        invoked = true;
    }

    bool invoked = false;
};

TEST_CASE("no invoke in case of an eventless transition", "[invoke]")
{
    StateMachine_t sm;
    InvokeChecker a("a", &sm);
    InvokeChecker b("b", &sm);
    InvokeChecker c("c", &sm);

    sm += a + event(1) > b;
    sm += b + noEvent  > c;

    REQUIRE(!a.invoked);
    REQUIRE(!b.invoked);
    REQUIRE(!c.invoked);

    sm.start();
    REQUIRE(isActive(sm, {&sm, &a}));
    REQUIRE(a.invoked);

    sm.addEvent(1);
    REQUIRE(isActive(sm, {&sm, &c}));
    REQUIRE(!b.invoked);
    REQUIRE(c.invoked);
}
