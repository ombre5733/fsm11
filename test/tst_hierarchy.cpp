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

using StateMachine_t = fsm11::StateMachine<>;
using State_t = StateMachine_t::state_type;


TEST_CASE("ancestor/descendant relationship", "[state]")
{
    State_t p("p");
    State_t c1("c1", &p);
    State_t c2("c2", &p);
    State_t c3("c3", &p);
    State_t c11("c11", &c1);
    State_t c12("c12", &c1);
    State_t c31("c31", &c3);
    State_t c32("c32", &c3);

    REQUIRE(isAncestor(&p, &p));
    REQUIRE(isDescendant(&p, &p));

    REQUIRE(isAncestor(&p, &c1));
    REQUIRE(isAncestor(&p, &c2));
    REQUIRE(isAncestor(&p, &c3));
    REQUIRE(isAncestor(&p, &c11));
    REQUIRE(isAncestor(&p, &c12));
    REQUIRE(isAncestor(&p, &c31));
    REQUIRE(isAncestor(&p, &c32));

    REQUIRE(isAncestor(&c1, &c11));
    REQUIRE(isAncestor(&c1, &c12));
    REQUIRE(!isAncestor(&c1, &c31));
    REQUIRE(!isAncestor(&c1, &c32));

    REQUIRE(!isProperAncestor(&p, &p));
    REQUIRE(!isProperAncestor(&c1, &p));
    REQUIRE(isProperAncestor(&p, &c1));
    REQUIRE(isProperAncestor(&p, &c11));
}

TEST_CASE("least common ancestor", "[state]")
{
    State_t p("p");
    State_t c1("c1", &p);
    State_t c2("c2", &p);
    State_t c3("c3", &p);
    State_t c11("c11", &c1);
    State_t c12("c12", &c1);
    State_t c31("c31", &c3);
    State_t c32("c32", &c3);
    State_t x("x");

    REQUIRE(findLeastCommonProperAncestor(&p, &p) == nullptr);
    REQUIRE(findLeastCommonProperAncestor(&c1, &c1) == &p);

    REQUIRE(findLeastCommonProperAncestor(&c1, &p) == nullptr);
    REQUIRE(findLeastCommonProperAncestor(&p, &c1) == nullptr);

    REQUIRE(findLeastCommonProperAncestor(&c11, &c12) == &c1);
    REQUIRE(findLeastCommonProperAncestor(&c12, &c11) == &c1);

    REQUIRE(findLeastCommonProperAncestor(&c11, &c1) == &p);
    REQUIRE(findLeastCommonProperAncestor(&c1, &c11) == &p);

    REQUIRE(findLeastCommonProperAncestor(&c11, &c2) == &p);
    REQUIRE(findLeastCommonProperAncestor(&c2, &c11) == &p);
    REQUIRE(findLeastCommonProperAncestor(&c32, &c11) == &p);
    REQUIRE(findLeastCommonProperAncestor(&c11, &c32) == &p);

    REQUIRE(findLeastCommonProperAncestor(&x, &c1) == nullptr);
    REQUIRE(findLeastCommonProperAncestor(&c1, &x) == nullptr);
}
