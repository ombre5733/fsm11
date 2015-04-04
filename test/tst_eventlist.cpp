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
