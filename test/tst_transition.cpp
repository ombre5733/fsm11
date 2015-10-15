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

using namespace fsm11;


namespace syncSM
{
using StateMachine_t = fsm11::StateMachine<>;
using State_t = StateMachine_t::state_type;
using Transition_t = StateMachine_t::transition_type;
} // namespace syncSM

namespace asyncSM
{
using StateMachine_t = StateMachine<AsynchronousEventDispatching,
                                    ConfigurationChangeCallbacksEnable<true>>;
using State_t = StateMachine_t::state_type;
} // namespace asyncSM

TEST_CASE("create a transition", "[transition]")
{
    using namespace syncSM;

    StateMachine_t sm;
    State_t a("a", &sm);
    State_t b("b", &sm);

    SECTION("plain transition")
    {
        Transition_t* t = sm += a + event(1) > b;
        REQUIRE(t->source() == &a);
        REQUIRE(t->target() == &b);
        REQUIRE(t->event() == 1);
        REQUIRE(t->eventless() == false);
        REQUIRE(t->guard() == nullptr);
        REQUIRE(t->action() == nullptr);
        REQUIRE(t->isExternal() == true);
        REQUIRE(t->isInternal() == false);
    }

    SECTION("event-less transition")
    {
        Transition_t* t = sm += a + noEvent > b;
        REQUIRE(t->source() == &a);
        REQUIRE(t->target() == &b);
        REQUIRE(t->eventless() == true);
        REQUIRE(t->guard() == nullptr);
        REQUIRE(t->action() == nullptr);
        REQUIRE(t->isExternal() == true);
        REQUIRE(t->isInternal() == false);
    }

    SECTION("target-less transition")
    {
        Transition_t* t = sm += a + event(1) > noTarget;
        REQUIRE(t->source() == &a);
        REQUIRE(t->target() == nullptr);
        REQUIRE(t->event() == 1);
        REQUIRE(t->eventless() == false);
        REQUIRE(t->guard() == nullptr);
        REQUIRE(t->action() == nullptr);
        REQUIRE(t->isExternal() == true);
        REQUIRE(t->isInternal() == false);
    }

    SECTION("transition with guard and action turned off explicitly")
    {
        Transition_t* t1 = sm += a + event(1) (nullptr) / nullptr > b;
        REQUIRE(t1->source() == &a);
        REQUIRE(t1->target() == &b);
        REQUIRE(t1->event() == 1);
        REQUIRE(t1->eventless() == false);
        REQUIRE(t1->guard() == nullptr);
        REQUIRE(t1->action() == nullptr);
        REQUIRE(t1->isExternal() == true);
        REQUIRE(t1->isInternal() == false);

        Transition_t* t2 = sm += a + event(1) [nullptr] / nullptr > b;
        REQUIRE(t2->source() == &a);
        REQUIRE(t2->target() == &b);
        REQUIRE(t2->event() == 1);
        REQUIRE(t2->eventless() == false);
        REQUIRE(t2->guard() == nullptr);
        REQUIRE(t2->action() == nullptr);
        REQUIRE(t2->isExternal() == true);
        REQUIRE(t2->isInternal() == false);
    }

    SECTION("transition with guard")
    {
        auto guard = [](int){ return true; };
        Transition_t* t1 = sm += a + noEvent [guard] > noTarget;
        REQUIRE(t1->source() == &a);
        REQUIRE(t1->target() == nullptr);
        REQUIRE(t1->eventless() == true);
        REQUIRE(t1->guard() != nullptr);
        REQUIRE(t1->action() == nullptr);
        REQUIRE(t1->isExternal() == true);
        REQUIRE(t1->isInternal() == false);

        Transition_t* t2 = sm += a + noEvent (guard) > noTarget;
        REQUIRE(t2->source() == &a);
        REQUIRE(t2->target() == nullptr);
        REQUIRE(t2->eventless() == true);
        REQUIRE(t2->guard() != nullptr);
        REQUIRE(t2->action() == nullptr);
        REQUIRE(t2->isExternal() == true);
        REQUIRE(t2->isInternal() == false);
    }

    SECTION("transition with action")
    {
        auto action = [](int){};
        Transition_t* t = sm += a + noEvent / action > noTarget;
        REQUIRE(t->source() == &a);
        REQUIRE(t->target() == nullptr);
        REQUIRE(t->eventless() == true);
        REQUIRE(t->guard() == nullptr);
        REQUIRE(t->action() != nullptr);
        REQUIRE(t->isExternal() == true);
        REQUIRE(t->isInternal() == false);
    }

    SECTION("transition with guard and action")
    {
        auto guard = [](int){ return true; };
        auto action = [](int){};
        Transition_t* t1 = sm += a + noEvent [guard] / action > noTarget;
        REQUIRE(t1->source() == &a);
        REQUIRE(t1->target() == nullptr);
        REQUIRE(t1->eventless() == true);
        REQUIRE(t1->guard() != nullptr);
        REQUIRE(t1->action() != nullptr);
        REQUIRE(t1->isExternal() == true);
        REQUIRE(t1->isInternal() == false);

        Transition_t* t2 = sm += a + noEvent (guard) / action > noTarget;
        REQUIRE(t2->source() == &a);
        REQUIRE(t2->target() == nullptr);
        REQUIRE(t2->eventless() == true);
        REQUIRE(t2->guard() != nullptr);
        REQUIRE(t2->action() != nullptr);
        REQUIRE(t2->isExternal() == true);
        REQUIRE(t2->isInternal() == false);
    }

    SECTION("external and internal transition")
    {
        Transition_t* t1 = sm += external> a + noEvent > b;
        REQUIRE(t1->source() == &a);
        REQUIRE(t1->target() == &b);
        REQUIRE(t1->eventless() == true);
        REQUIRE(t1->guard() == nullptr);
        REQUIRE(t1->action() == nullptr);
        REQUIRE(t1->isExternal() == true);
        REQUIRE(t1->isInternal() == false);

        Transition_t* t2 = sm += internal> a + noEvent > b;
        REQUIRE(t2->source() == &a);
        REQUIRE(t2->target() == &b);
        REQUIRE(t2->eventless() == true);
        REQUIRE(t2->guard() == nullptr);
        REQUIRE(t2->action() == nullptr);
        REQUIRE(t2->isExternal() == false);
        REQUIRE(t2->isInternal() == true);

        Transition_t* t3 = sm += external> a + event(1) > b;
        REQUIRE(t3->source() == &a);
        REQUIRE(t3->target() == &b);
        REQUIRE(t3->event() == 1);
        REQUIRE(t3->eventless() == false);
        REQUIRE(t3->guard() == nullptr);
        REQUIRE(t3->action() == nullptr);
        REQUIRE(t3->isExternal() == true);
        REQUIRE(t3->isInternal() == false);

        Transition_t* t4 = sm += internal> a + event(1) > b;
        REQUIRE(t4->source() == &a);
        REQUIRE(t4->target() == &b);
        REQUIRE(t4->event() == 1);
        REQUIRE(t4->eventless() == false);
        REQUIRE(t4->guard() == nullptr);
        REQUIRE(t4->action() == nullptr);
        REQUIRE(t4->isExternal() == false);
        REQUIRE(t4->isInternal() == true);
    }
}

TEST_CASE("targetless transitions block an event", "[transition]")
{
    using namespace syncSM;
    StateMachine_t sm;

    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> aa("aa", &a);
    TrackingState<State_t> aaa("aaa", &aa);
    TrackingState<State_t> aab("aab", &aa);
    TrackingState<State_t> ab("ab", &a);
    TrackingState<State_t> aba("aba", &ab);
    TrackingState<State_t> abb("abb", &ab);
    TrackingState<State_t> b("b", &sm);
    TrackingState<State_t> ba("ba", &b);
    TrackingState<State_t> bb("bb", &b);

    sm += aa + event(1) > ab;

    sm.start();
    REQUIRE(isActive(sm, {&sm, &a, &aa, &aaa}));
    REQUIRE(a.entered == 1);
    REQUIRE(a.left == 0);
    REQUIRE(aa.entered == 1);
    REQUIRE(aa.left == 0);
    REQUIRE(aaa.entered == 1);
    REQUIRE(aaa.left == 0);
    REQUIRE(ab.entered == 0);
    REQUIRE(b.entered == 0);

    SECTION("without targetless transition")
    {
        sm.addEvent(1);

        REQUIRE(isActive(sm, {&sm, &a, &ab, &aba}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 0);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(aaa.entered == 1);
        REQUIRE(aaa.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
        REQUIRE(aba.entered == 1);
        REQUIRE(aba.left == 0);
        REQUIRE(b.entered == 0);
    }

    SECTION("with targetless transition")
    {
        sm += aaa + event(1) > noTarget;

        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &a, &aa, &aaa}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 0);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 0);
        REQUIRE(aaa.entered == 1);
        REQUIRE(aaa.left == 0);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 0);
    }
}

SCENARIO("blocking an event with a targetless transition", "[transition]")
{
    GIVEN ("a FSM")
    {
        using namespace syncSM;
        StateMachine_t sm;

        TrackingState<State_t> a("a", &sm);
        TrackingState<State_t> b("b", &sm);

        WHEN ("no targetless transition is present")
        {
            sm += a + event(1) > b;
            sm.start();

            sm.addEvent(1);
            THEN ("a state change is performed")
            {
                REQUIRE(isActive(sm, {&sm, &b}));
                REQUIRE(a.entered == 1);
                REQUIRE(a.left == 1);
                REQUIRE(b.entered == 1);
                REQUIRE(b.left == 0);
            }
        }

        WHEN ("a targetless transition is present")
        {
            sm += a + event(1) > noTarget;
            sm += a + event(1) > b;
            sm.start();

            sm.addEvent(1);
            THEN ("the event is blocked")
            {
                REQUIRE(isActive(sm, {&sm, &a}));
                REQUIRE(a.entered == 1);
                REQUIRE(a.left == 0);
                REQUIRE(b.entered == 0);
                REQUIRE(b.left == 0);
            }
        }
    }
}

TEST_CASE("initial states are activated after start", "[transition]")
{
    using namespace syncSM;
    StateMachine_t sm;

    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> aa("aa", &a);
    TrackingState<State_t> aaa("aaa", &aa);
    TrackingState<State_t> aab("aab", &aa);
    TrackingState<State_t> ab("ab", &a);
    TrackingState<State_t> aba("aba", &ab);
    TrackingState<State_t> abb("abb", &ab);
    TrackingState<State_t> b("b", &sm);
    TrackingState<State_t> ba("ba", &b);
    TrackingState<State_t> bb("bb", &b);

    SECTION("the first child is the default initial state")
    {
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa, &aaa}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 0);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 0);
        REQUIRE(aaa.entered == 1);
        REQUIRE(aaa.left == 0);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 0);
    }

    SECTION("initial state is a child")
    {
        a.setInitialState(&ab);

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &ab, &aba}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 0);
        REQUIRE(aa.entered == 0);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
        REQUIRE(aba.entered == 1);
        REQUIRE(aba.left == 0);
        REQUIRE(b.entered == 0);
    }

    SECTION("initial state is a descendant of the first child")
    {
        a.setInitialState(&aab);

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa, &aab}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 0);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 0);
        REQUIRE(aab.entered == 1);
        REQUIRE(aab.left == 0);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 0);
    }

    SECTION("initial state is a descendant of another child")
    {
        a.setInitialState(&aba);

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &ab, &aba}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 0);
        REQUIRE(aa.entered == 0);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
        REQUIRE(aba.entered == 1);
        REQUIRE(aba.left == 0);
        REQUIRE(b.entered == 0);
    }
}

// TODO
#if 0
TEST_CASE("transition actions are executed before state entries",
          "[transition]")
{
    //REQUIRE(false);
}
#endif

TEST_CASE("initial states during configuration change", "[transition]")
{
    using namespace syncSM;
    StateMachine_t sm;

    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> b("b", &sm);
    TrackingState<State_t> ba("ba", &b);
    TrackingState<State_t> baa("baa", &ba);
    TrackingState<State_t> bb("bb", &b);

    SECTION("without initial state")
    {
        sm += a + event(1) > b;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &b, &ba, &baa}));

        REQUIRE(a == std::make_tuple(1, 1));
        REQUIRE(b == std::make_tuple(1, 0));
        REQUIRE(ba == std::make_tuple(1, 0));
        REQUIRE(baa == std::make_tuple(1, 0));
        REQUIRE(bb == std::make_tuple(0, 0));
    }

    SECTION("with initial state")
    {
        sm += a + event(1) > b;
        b.setInitialState(&bb);

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &b, &bb}));

        REQUIRE(a == std::make_tuple(1, 1));
        REQUIRE(b == std::make_tuple(1, 0));
        REQUIRE(ba == std::make_tuple(0, 0));
        REQUIRE(baa == std::make_tuple(0, 0));
        REQUIRE(bb == std::make_tuple(1, 0));
    }

    SECTION("initial state is ignored if the target is a sibling")
    {
        sm += a + event(1) > ba;
        b.setInitialState(&bb);

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &b, &ba, &baa}));

        REQUIRE(a == std::make_tuple(1, 1));
        REQUIRE(b == std::make_tuple(1, 0));
        REQUIRE(ba == std::make_tuple(1, 0));
        REQUIRE(baa == std::make_tuple(1, 0));
        REQUIRE(bb == std::make_tuple(0, 0));
    }

    SECTION("initial state is ignored if the target is a descendant")
    {
        sm += a + event(1) > baa;
        b.setInitialState(&bb);

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &b, &ba, &baa}));

        REQUIRE(a == std::make_tuple(1, 1));
        REQUIRE(b == std::make_tuple(1, 0));
        REQUIRE(ba == std::make_tuple(1, 0));
        REQUIRE(baa == std::make_tuple(1, 0));
        REQUIRE(bb == std::make_tuple(0, 0));
    }
}

TEST_CASE("internal and external transitions from compound state", "[transition]")
{
    using namespace syncSM;
    StateMachine_t sm;

    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> aa("aa", &a);
    TrackingState<State_t> ab("ab", &a);

    sm += external> a + event(1) > ab;
    sm += internal> a + event(2) > ab;
    sm +=           a + event(3) > ab;

    SECTION("external transition leaves the source state")
    {
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &a, &ab}));

        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
    }

    SECTION("internal transition does not leave the source state")
    {
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        sm.addEvent(2);
        REQUIRE(isActive(sm, {&sm, &a, &ab}));

        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 0);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
    }

    SECTION("by default a transition is an external one")
    {
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        sm.addEvent(3);
        REQUIRE(isActive(sm, {&sm, &a, &ab}));

        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
    }
}

TEST_CASE("internal and external transitions from parallel state", "[transition]")
{
    using namespace syncSM;
    StateMachine_t sm;

    TrackingState<State_t> a("a", &sm);
    a.setChildMode(ChildMode::Parallel);
    TrackingState<State_t> aa("aa", &a);
    TrackingState<State_t> ab("ab", &a);

    sm += external> a + event(1) > ab;
    sm += internal> a + event(2) > ab;
    sm +=           a + event(3) > ab;

    SECTION("external transition leaves the source state")
    {
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa, &ab}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &a, &aa, &ab}));

        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 2);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 2);
        REQUIRE(ab.left == 1);
    }

    SECTION("internal transition behaves like an external one")
    {
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a, &aa, &ab}));
        sm.addEvent(2);
        REQUIRE(isActive(sm, {&sm, &a, &aa, &ab}));

        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 2);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 2);
        REQUIRE(ab.left == 1);
    }
}

TEST_CASE("internal and external transitions from atomic state", "[transition]")
{
    using namespace syncSM;
    StateMachine_t sm;

    TrackingState<State_t> a("a", &sm);

    sm += external> a + event(1) > a;
    sm += internal> a + event(2) > a;

    SECTION("external transition leaves the source state")
    {
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &a}));

        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
    }

    SECTION("internal transition behaves like an external one")
    {
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(2);
        REQUIRE(isActive(sm, {&sm, &a}));

        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
    }
}

// An allocator to keep track of the number of transitions.
template <typename T>
class TrackingTransitionAllocator : public std::allocator<T>
{
    using base = std::allocator<T>;

public:
    template <typename U>
    struct rebind
    {
        using other = TrackingTransitionAllocator<U>;
    };

    using pointer = typename base::pointer;
    using size_type = typename base::size_type;

    TrackingTransitionAllocator(int& counter)
        : m_numTransitions(counter)
    {
        m_numTransitions = 0;
    }

    template <typename U>
    TrackingTransitionAllocator(const TrackingTransitionAllocator<U>& other)
        : m_numTransitions(other.m_numTransitions)
    {
    }

    pointer allocate(size_type n)
    {
        ++m_numTransitions;
        return base::allocate(n);
    }

    void deallocate(pointer p, size_type s)
    {
        --m_numTransitions;
        base::deallocate(p, s);
    }

private:
    int& m_numTransitions;

    template <typename U>
    friend class TrackingTransitionAllocator;
};

TEST_CASE("transition allocator by copy-construction", "[transition]")
{
    using StateMachine_t = fsm11::StateMachine<Allocator<TrackingTransitionAllocator<Transition<void>>>>;
    using State_t = StateMachine_t::state_type;

    int numTransitions = 0;

    {
        StateMachine_t sm{std::allocator_arg,
                          TrackingTransitionAllocator<void>(numTransitions)};

        TrackingState<State_t> a("a", &sm);
        TrackingState<State_t> b("b", &sm);

        sm += a + event(1) > b;
        REQUIRE(numTransitions == 1);
        sm += a + event(2) > b;
        REQUIRE(numTransitions == 2);
        sm += a + event(3) > b;
        REQUIRE(numTransitions == 3);
    }

    REQUIRE(numTransitions == 0);
}

TEST_CASE("an event matches a guarded eventless transition", "[transition]")
{
    using namespace syncSM;
    StateMachine_t sm;

    State_t a("a", &sm);
    State_t b("b", &sm);

    SECTION("guard is independent")
    {
        bool guard = false;
        sm += a + noEvent ([&] (int) { return guard; }) > b;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(2);
        REQUIRE(isActive(sm, {&sm, &a}));
        guard = true;
        sm.addEvent(2);
        REQUIRE(isActive(sm, {&sm, &b}));
    }

    SECTION("guard depends on event")
    {
        sm += a + noEvent ([&] (int event) { return event == 3; }) > b;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(2);
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(3);
        REQUIRE(isActive(sm, {&sm, &b}));
    }
}
