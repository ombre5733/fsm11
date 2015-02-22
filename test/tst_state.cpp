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
#include <cstring>

using namespace fsm11;

using StateMachine_t = fsm11::StateMachine<>;
using State_t = StateMachine_t::state_type;

TEST_CASE("construct a state", "[state]")
{
    State_t s("name");

    REQUIRE(s.childMode() == State_t::Exclusive);
    REQUIRE(!std::strcmp(s.name(), "name"));
    REQUIRE(s.parent() == 0);
    REQUIRE(s.isAtomic());
    REQUIRE(!s.isCompound());
    REQUIRE(!s.isParallel());
    REQUIRE(s.stateMachine() == 0);
    REQUIRE(!s.isActive());
    REQUIRE(s.initialState() == 0);

    REQUIRE(s.beginTransitions() == s.endTransitions());
    REQUIRE(s.cbeginTransitions() == s.cendTransitions());
}

TEST_CASE("set the parent of a state", "[state]")
{
    State_t p1("p1");
    State_t p2("p2");

    State_t c("c", &p1);
    REQUIRE(&p1 == c.parent());
    REQUIRE(!p1.isAtomic());
    REQUIRE(p2.isAtomic());

    c.setParent(&p2);
    REQUIRE(&p2 == c.parent());
    REQUIRE(p1.isAtomic());
    REQUIRE(!p2.isAtomic());

    c.setParent(&p1);
    REQUIRE(&p1 == c.parent());
    REQUIRE(!p1.isAtomic());
    REQUIRE(p2.isAtomic());
}

TEST_CASE("set the state machine", "[state]")
{
    StateMachine_t sm1;
    State_t s1("s1", &sm1);
    State_t s2("s2");
    State_t s3("s3", &s2);

    REQUIRE(s1.stateMachine() == &sm1);
    REQUIRE(s2.stateMachine() == 0);
    REQUIRE(s3.stateMachine() == 0);
    s2.setParent(&s1);
    REQUIRE(s1.stateMachine() == &sm1);
    REQUIRE(s2.stateMachine() == &sm1);
    REQUIRE(s3.stateMachine() == &sm1);

    State_t s4("s4", &s2);
    REQUIRE(s4.stateMachine() == &sm1);

    REQUIRE(!s1.isAtomic());

    StateMachine_t sm2;
    s2.setParent(&sm2);

    REQUIRE(s1.stateMachine() == &sm1);
    REQUIRE(s2.stateMachine() == &sm2);
    REQUIRE(s3.stateMachine() == &sm2);
    REQUIRE(s4.stateMachine() == &sm2);

    REQUIRE(s1.isAtomic());
}

TEST_CASE("change the child mode", "[state]")
{
    State_t s("s");
    State_t c("c", &s);

    REQUIRE(State_t::Exclusive == s.childMode());
    REQUIRE(s.isCompound());
    REQUIRE(!s.isParallel());

    s.setChildMode(State_t::Parallel);
    REQUIRE(State_t::Parallel == s.childMode());
    REQUIRE(!s.isCompound());
    REQUIRE(s.isParallel());

    s.setChildMode(State_t::Exclusive);
    REQUIRE(State_t::Exclusive == s.childMode());
    REQUIRE(s.isCompound());
    REQUIRE(!s.isParallel());
}

TEST_CASE("set an initial state", "[state]")
{
    StateMachine_t sm;
    State_t s1("s1", &sm);
    State_t s2("s2");
    State_t s3("s3", &s2);

    REQUIRE(s2.initialState() == 0);
    s2.setInitialState(&s3);
    REQUIRE(s2.initialState() == &s3);

    REQUIRE(sm.initialState() == 0);
    sm.setInitialState(&s1);
    REQUIRE(sm.initialState() == &s1);
}

TEST_CASE("find a child", "[state]")
{
    State_t p("p");
    State_t c1("c1", &p);
    State_t c2("c2", &p);
    State_t c3("c3", &p);
    State_t c11("c11", &c1);
    State_t c12("c12", &c1);
    State_t c31("c31", &c3);
    State_t c32("c32", &c3);

    State_t* found;

    found = p.findChild("c1");
    REQUIRE(found == &c1);

    found = c3.findChild("c32");
    REQUIRE(found == &c32);

    found = p.findChild("p");
    REQUIRE(found == nullptr);

    found = c1.findChild("p");
    REQUIRE(found == nullptr);

    found = c1.findChild("");
    REQUIRE(found == nullptr);
}

TEST_CASE("find a descendant", "[state]")
{
    State_t p("p");
    State_t c1("c1", &p);
    State_t c2("c2", &p);
    State_t c3("c3", &p);
    State_t c11("c11", &c1);
    State_t c12("c12", &c1);
    State_t c31("c31", &c3);
    State_t c32("c32", &c3);

    State_t* found;

    found = p.findDescendant({"c1"});
    REQUIRE(found == &c1);

    found = c3.findDescendant({"c32"});
    REQUIRE(found == &c32);

    found = p.findDescendant({"c3", "c32"});
    REQUIRE(found == &c32);

    found = p.findDescendant({"p"});
    REQUIRE(found == nullptr);

    found = c1.findDescendant({"p"});
    REQUIRE(found == nullptr);

    found = c1.findDescendant({""});
    REQUIRE(found == nullptr);
}

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
