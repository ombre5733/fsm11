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
#include <initializer_list>

using namespace fsm11;

using StateMachine_t = fsm11::StateMachine<>;
using State_t = StateMachine_t::state_type;

TEST_CASE("construct a state", "[state]")
{
    State_t s("name");

    REQUIRE(s.childMode() == ChildMode::Exclusive);
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
    REQUIRE(s2.stateMachine() == nullptr);
    REQUIRE(s3.stateMachine() == nullptr);
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

    REQUIRE(s.childMode() == ChildMode::Exclusive);
    REQUIRE(s.isCompound());
    REQUIRE(!s.isParallel());

    s.setChildMode(ChildMode::Parallel);
    REQUIRE(s.childMode() == ChildMode::Parallel);
    REQUIRE(!s.isCompound());
    REQUIRE(s.isParallel());

    s.setChildMode(ChildMode::Exclusive);
    REQUIRE(s.childMode() == ChildMode::Exclusive);
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

    found = p.findDescendant({});
    REQUIRE(found == &p);

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

SCENARIO("a state hierarchy can be constructed in parts", "[state]")
{
    GIVEN ("two distinct state trees")
    {
        StateMachine_t sm;
        State_t a("a", &sm);
        State_t a1("a1", &a);
        State_t a2("a2", &a);
        State_t a3("a3", &a1);
        State_t a4("a4", &a1);

        State_t b("b");
        State_t b1("b1", &b);
        State_t b2("b2", &b);
        State_t b3("b3", &b1);
        State_t b4("b4", &b1);

        for (State_t* state : {&a, &a1, &a2, &a3, &a4})
            REQUIRE(state->stateMachine() == &sm);
        REQUIRE(b.parent() == nullptr);

        WHEN ("a tree is added to an FSM")
        {
            b.setParent(&sm);
            REQUIRE(b.parent() == &sm);
            THEN ("the state machine of the sub-tree changes")
            {
                for (State_t* state : {&b, &b1, &b2, &b3, &b4})
                    REQUIRE(state->stateMachine() == &sm);
            }
        }

        WHEN ("a tree is added as sub-tree")
        {
            b.setParent(&a);
            REQUIRE(b.parent() == &a);
            THEN ("the state machine of the sub-tree changes")
            {
                for (State_t* state : {&b, &b1, &b2, &b3, &b4})
                    REQUIRE(state->stateMachine() == &sm);
            }
        }
    }
}
