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

        const char* gotoA = "go to A";
        const char* gotoB = "go to B";
        const char* gotoC = "go to C";

        sm += a + event(gotoB) > b;
        sm += a + event(gotoC) > c;

        sm.start();

        WHEN ("the event 'go to B' is added")
        {
            sm.addEvent(gotoB);
            THEN ("the FSM transitions to state B")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
            }

            WHEN ("the event 'go to A' is added")
            {
                sm.addEvent(gotoA);
                THEN ("nothing happens")
                {
                    REQUIRE(isActive(sm, {&sm, &b}));
                }
            }
        }

        WHEN ("the event 'go to B' with a distinct address is added")
        {
            sm.addEvent(std::string(gotoB).c_str());
            THEN ("nothing happens")
            {
                REQUIRE(isActive(sm, {&sm, &a}));
            }
        }

        WHEN ("the event 'go to C' is added")
        {
            sm.addEvent(gotoC);
            THEN ("the FSM transitions to state C")
            {
                REQUIRE(isActive(sm, {&sm, &c}));
            }

            WHEN ("the event 'go to A' is added")
            {
                sm.addEvent(gotoA);
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
