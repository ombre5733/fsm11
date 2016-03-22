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
