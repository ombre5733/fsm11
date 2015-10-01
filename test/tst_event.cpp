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

#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

using namespace fsm11;


enum Event1
{
    none,
    toB,
    toC
};

enum class Event2
{
    none,
    toB,
    toC
};

SCENARIO("integral types can be used as events", "[event]")
{
    GIVEN ("an FSM with integers as event type")
    {
        using StateMachine_t = StateMachine<EventType<int>,
                                            EventListType<std::deque<int>>>;
        using State_t = State<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        sm += a + event(-1) > b;
        sm += a + event(-2) > c;

        sm.start();

        WHEN ("the event '-1' is added")
        {
            sm.addEvent(-1);

            THEN ("the FSM transitions to state B")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
            }
        }

        WHEN ("the event '-2' is added")
        {
            sm.addEvent(-2);

            THEN ("the FSM transitions to state C")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
            }
        }
    }

    GIVEN ("an FSM with unsigned integers as event type")
    {
        using StateMachine_t = StateMachine<EventType<unsigned>,
                                            EventListType<std::deque<unsigned>>>;
        using State_t = State<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        // Note: Using an integer as events below results in a warning, i.e.
        // the implementation is quite strict about types.
        sm += a + event(1u) > b;
        sm += a + event(2u) > c;

        sm.start();

        WHEN ("the event '1' is added")
        {
            sm.addEvent(1);

            THEN ("the FSM transitions to state B")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
            }
        }

        WHEN ("the event '2' is added")
        {
            sm.addEvent(2);

            THEN ("the FSM transitions to state C")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
            }
        }
    }

    GIVEN ("an FSM with an enum as event type")
    {
        using StateMachine_t = StateMachine<EventType<Event1>,
                                            EventListType<std::deque<Event1>>>;
        using State_t = State<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        sm += a + event(toB) > b;
        sm += a + event(toC) > c;

        sm.start();

        WHEN ("the event 'toB' is added")
        {
            sm.addEvent(toB);

            THEN ("the FSM transitions to state B")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
            }
        }

        WHEN ("the event 'toC' is added")
        {
            sm.addEvent(toC);

            THEN ("the FSM transitions to state C")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
            }
        }
    }

    GIVEN ("an FSM with an enum class as event type")
    {
        using StateMachine_t = StateMachine<EventType<Event2>,
                                            EventListType<std::deque<Event2>>>;
        using State_t = State<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        sm += a + event(Event2::toB) > b;
        sm += a + event(Event2::toC) > c;

        sm.start();

        WHEN ("the event 'toB' is added")
        {
            sm.addEvent(Event2::toB);

            THEN ("the FSM transitions to state B")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
            }
        }

        WHEN ("the event 'toC' is added")
        {
            sm.addEvent(Event2::toC);

            THEN ("the FSM transitions to state C")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
            }
        }
    }
}

SCENARIO("strings are suitable as events", "[events]")
{
    GIVEN ("an FSM with const char* as event type")
    {
        using StateMachine_t = StateMachine<EventType<const char*>,
                                            EventListType<std::deque<const char*>>>;
        using State_t = State<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        sm += a + event("go to B") > b;
        sm += a + event("go to C") > c;

        sm.start();

        WHEN ("the event 'go to B' is added")
        {
            sm.addEvent("go to B");
            THEN ("the FSM transitions to state B")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
            }

            WHEN ("the event 'go to A' is added")
            {
                sm.addEvent("go to A");
                THEN ("nothing happens")
                {
                    REQUIRE(isActive(sm, {&sm, &b}));
                }
            }
        }

        WHEN ("the event 'go to B' with a distinct address is added")
        {
            sm.addEvent(std::string("go to B").c_str());
            THEN ("nothing happens")
            {
                REQUIRE(isActive(sm, {&sm, &a}));
            }
        }

        WHEN ("the event 'go to C' is added")
        {
            sm.addEvent("go to C");
            THEN ("the FSM transitions to state C")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
            }

            WHEN ("the event 'go to A' is added")
            {
                sm.addEvent("go to A");
                THEN ("nothing happens")
                {
                    REQUIRE(isActive(sm, {&sm, &c}));
                }
            }
        }

        WHEN ("the event 'go to X' is added")
        {
            sm.addEvent("go to X");
            THEN ("nothing happens")
            {
                REQUIRE(isActive(sm, {&sm, &a}));
            }
        }
    }

    GIVEN ("an FSM with std::string as event type")
    {
        using StateMachine_t = StateMachine<EventType<std::string>,
                                            EventListType<std::deque<std::string>>>;
        using State_t = State<StateMachine_t>;

        StateMachine_t sm;
        State_t a("a", &sm);
        State_t b("b", &sm);
        State_t c("c", &sm);

        sm += a + event("go to B") > b;
        sm += a + event("go to C") > c;

        sm.start();

        WHEN ("the event 'go to B' is added")
        {
            sm.addEvent("go to B");
            THEN ("the FSM transitions to state B")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
            }

            WHEN ("the event 'go to A' is added")
            {
                sm.addEvent("go to A");
                THEN ("nothing happens")
                {
                    REQUIRE(isActive(sm, {&sm, &b}));
                }
            }
        }

        WHEN ("the event 'go to C' is added")
        {
            sm.addEvent("go to C");
            THEN ("the FSM transitions to state C")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
            }

            WHEN ("the event 'go to A' is added")
            {
                sm.addEvent("go to A");
                THEN ("nothing happens")
                {
                    REQUIRE(isActive(sm, {&sm, &c}));
                }
            }
        }

        WHEN ("the event 'go to X' is added")
        {
            sm.addEvent("go to X");
            THEN ("nothing happens")
            {
                REQUIRE(isActive(sm, {&sm, &a}));
            }
        }
    }
}

#if 0
struct TrackingEvent
{
    TrackingEvent() = default;

    TrackingEvent(int value)
        : m_value(new int(value))
    {
        ++std::get<0>(m_valueToCounter[value]);
    }

    TrackingEvent(const TrackingEvent&) = delete;

    TrackingEvent(TrackingEvent&& e)
        : m_value(std::move(e.m_value))
    {
        if (m_value)
            ++std::get<1>(m_valueToCounter[*m_value]);
    }

    ~TrackingEvent()
    {
        if (m_value)
        {
            ++std::get<2>(m_valueToCounter[*m_value]);
        }
    }

    std::unique_ptr<int> m_value;

    typedef std::tuple<int, int, int> counters;
    static std::map<int, counters> m_valueToCounter;
};

std::map<int, TrackingEvent::counters> TrackingEvent::m_valueToCounter;

SCENARIO("movable types can be used as event", "[event]")
{
}
#endif
