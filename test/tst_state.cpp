/*******************************************************************************
  fsm11 - A C++11-compliant framework for finite state machines

  Copyright (c) 2015, Manuel Freiberger
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

TEST_CASE("set an initial state", "[state][exception]")
{
    GIVEN ("a set of states")
    {
        State_t s1("s1");
        State_t s2("s2", &s1);
        State_t s3("s3", &s2);
        State_t s4("s4");
        REQUIRE(s1.initialState() == nullptr);

        WHEN ("the initial state is set to a child")
        {
            s1.setInitialState(&s2);
            THEN ("the initial state is changed")
            {
                REQUIRE(s1.initialState() == &s2);
            }
        }

        WHEN ("the initial state is set to a grand-child")
        {
            s1.setInitialState(&s3);
            THEN ("the initial state is changed")
            {
                REQUIRE(s1.initialState() == &s3);
            }
        }

        WHEN ("the initial state is set to an unrelated state")
        {
            THEN ("an exception is thrown")
            {
                try
                {
                    s1.setInitialState(&s4);
                    REQUIRE(false);
                }
                catch (FsmError& error)
                {
                    REQUIRE(error.code() == FsmErrorCode::InvalidStateRelationship);
                }
                catch (...)
                {
                    REQUIRE(false);
                }
            }
        }
    }
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
