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
#include "testutils.hpp"

#include <queue>

using namespace fsm11;


SCENARIO("default event list has FIFO semantic", "[eventlist]")
{
    using StateMachine_t = StateMachine<>;
    using State_t = State<StateMachine_t>;

    GIVEN ("a synchronous FSM")
    {
        StateMachine_t sm;
        TrackingState<State_t> a("a", &sm);
        TrackingState<State_t> b("b", &sm);
        TrackingState<State_t> c("c", &sm);

        sm += a + event(1) > b;
        sm += b + event(2) > c;

        WHEN ("events [1, 2] are added")
        {
            sm.addEvent(1);
            sm.addEvent(2);

            THEN ("nothing happens")
            {
                REQUIRE(isActive(sm, {}));
                REQUIRE(a.entered == 0);
                REQUIRE(b.entered == 0);
            }
        }

        WHEN ("events [1, 2] are added and the FSM is started")
        {
            sm.addEvent(1);
            sm.addEvent(2);
            sm.start();

            THEN ("the FSM goes via A and B to C")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
                REQUIRE(a.entered == 1);
                REQUIRE(a.left == 1);
                REQUIRE(b.entered == 1);
                REQUIRE(b.left == 1);
                REQUIRE(c.entered == 1);
                REQUIRE(c.left == 0);
            }
        }

        WHEN ("events [2, 1] are added and the FSM is started")
        {
            sm.addEvent(2);
            sm.addEvent(1);
            sm.start();

            THEN ("the FSM goes via A to B")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
                REQUIRE(a.entered == 1);
                REQUIRE(a.left == 1);
                REQUIRE(b.entered == 1);
                REQUIRE(b.left == 0);
                REQUIRE(c.entered == 0);
                REQUIRE(c.left == 0);
            }
        }
    }
}

template <typename T>
struct PriorityQueueAdapter
        : public std::priority_queue<T, std::vector<T>, std::greater<T>>
{
    typedef std::priority_queue<T, std::vector<T>, std::greater<T>> base_type;

    typename base_type::const_reference front() const
    {
        return base_type::top();
    }

    void push_back(const T& value)
    {
        base_type::push(value);
    }

    void pop_front()
    {
        base_type::pop();
    }
};

SCENARIO("a priority queue can be used as event list", "[eventlist]")
{
    using StateMachine_t = StateMachine<EventListType<PriorityQueueAdapter<int>>>;
    using State_t = State<StateMachine_t>;

    GIVEN ("a synchronous FSM")
    {
        StateMachine_t sm;
        TrackingState<State_t> a("a", &sm);
        TrackingState<State_t> b("b", &sm);
        TrackingState<State_t> c("c", &sm);

        sm += a + event(1) > b;
        sm += b + event(2) > c;

        WHEN ("events are added")
        {
            sm.addEvent(1);
            sm.addEvent(2);

            THEN ("nothing happens")
            {
                REQUIRE(isActive(sm, {}));
                REQUIRE(a.entered == 0);
                REQUIRE(b.entered == 0);
            }
        }

        WHEN ("events [1, 2] are added and the FSM is started")
        {
            sm.addEvent(1);
            sm.addEvent(2);
            sm.start();

            THEN ("the FSM goes via A and B to C")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
                REQUIRE(a.entered == 1);
                REQUIRE(a.left == 1);
                REQUIRE(b.entered == 1);
                REQUIRE(b.left == 1);
                REQUIRE(c.entered == 1);
                REQUIRE(c.left == 0);
            }
        }

        WHEN ("events [2, 1] are added and the FSM is started")
        {
            sm.addEvent(2);
            sm.addEvent(1);
            sm.start();

            THEN ("the FSM goes via A and B to C")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
                REQUIRE(a.entered == 1);
                REQUIRE(a.left == 1);
                REQUIRE(b.entered == 1);
                REQUIRE(b.left == 1);
                REQUIRE(c.entered == 1);
                REQUIRE(c.left == 0);
            }
        }
    }
}
