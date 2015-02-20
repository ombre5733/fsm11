#include "catch.hpp"

#include "../src/statemachine.hpp"
#include "testutils.hpp"

using namespace fsm11;


namespace sync
{
using StateMachine_t = fsm11::StateMachine<>;
using State_t = StateMachine_t::state_type;
} // namespace sync

namespace async
{
using StateMachine_t = StateMachine<AsynchronousEventDispatching>;
using State_t = StateMachine_t::state_type;
} // namespace async

TEST_CASE("simple configuration changes in synchronous statemachine",
          "[transition]")
{
    using namespace sync;
    StateMachine_t sm;

    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> aa("aa", &a);
    TrackingState<State_t> ab("ab", &a);
    TrackingState<State_t> b("b", &sm);
    TrackingState<State_t> ba("ba", &b);
    TrackingState<State_t> bb("bb", &b);

    sm += aa + event(2) == ba;
    sm += ba + event(2) == bb;
    sm += a  + event(3) == bb;
    sm += b  + event(3) == ab;
    sm += aa + event(4) == b;
    sm += ba + event(4) == a;
    sm += a  + event(5) == ab;
    sm += ab + event(6) == a;

    sm.start();
    REQUIRE(isActive(sm, {&sm, &a, &aa}));
    REQUIRE(a.entered == 1);
    REQUIRE(a.left == 0);
    REQUIRE(aa.entered == 1);
    REQUIRE(aa.left == 0);
    REQUIRE(ab.entered == 0);
    REQUIRE(b.entered == 0);

    SECTION("from atomic to atomic")
    {
        sm.addEvent(2);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 0);
        REQUIRE(bb.entered == 0);

        sm.addEvent(2);
        REQUIRE(isActive(sm, {&sm, &b, &bb}));
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 1);
        REQUIRE(bb.entered == 1);
        REQUIRE(bb.left == 0);
    }

    SECTION("from compound to atomic")
    {
        sm.addEvent(3);
        REQUIRE(isActive(sm, {&sm, &b, &bb}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 0);
        REQUIRE(bb.entered == 1);
        REQUIRE(bb.left == 0);

        sm.addEvent(3);
        REQUIRE(isActive(sm, {&sm, &a, &ab}));
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 1);
        REQUIRE(bb.entered == 1);
        REQUIRE(bb.left == 1);
        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
    }

    SECTION("from atomic to compound")
    {
        sm.addEvent(4);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 0);
        REQUIRE(bb.entered == 0);

        sm.addEvent(4);
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 1);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 1);
        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 2);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
    }

    SECTION("between ancestor and descendant")
    {
        sm.addEvent(5);
        REQUIRE(isActive(sm, {&sm, &a, &ab}));
        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
        REQUIRE(b.entered == 0);

        sm.addEvent(6);
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        REQUIRE(a.entered == 3);
        REQUIRE(a.left == 2);
        REQUIRE(aa.entered == 2);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 1);
        REQUIRE(b.entered == 0);
    }

    sm.stop();
    REQUIRE(isActive(sm, {}));
    for (auto state : {&a, &aa, &ab, &b, &ba, &bb})
        REQUIRE(state->entered == state->left);
}

#if 0
TEST_CASE("simple configuration changes in asynchronous statemachine",
          "[transition]")
{
    using namespace async;
    StateMachine_t sm;

    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> aa("aa", &a);
    TrackingState<State_t> ab("ab", &a);
    TrackingState<State_t> b("b", &sm);
    TrackingState<State_t> ba("ba", &b);
    TrackingState<State_t> bb("bb", &b);

    sm += aa + event(2) == ba;
    sm += ba + event(2) == bb;
    sm += a  + event(3) == bb;
    sm += b  + event(3) == ab;
    sm += aa + event(4) == b;
    sm += ba + event(4) == a;
    sm += a  + event(5) == ab;
    sm += ab + event(6) == a;

    sm.start();
    REQUIRE(isActive(sm, {&sm, &a, &aa}));
    REQUIRE(a.entered == 1);
    REQUIRE(a.left == 0);
    REQUIRE(aa.entered == 1);
    REQUIRE(aa.left == 0);
    REQUIRE(ab.entered == 0);
    REQUIRE(b.entered == 0);

    SECTION("from atomic to atomic")
    {
        sm.addEvent(2);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 0);
        REQUIRE(bb.entered == 0);

        sm.addEvent(2);
        REQUIRE(isActive(sm, {&sm, &b, &bb}));
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 1);
        REQUIRE(bb.entered == 1);
        REQUIRE(bb.left == 0);
    }

    SECTION("from compound to atomic")
    {
        sm.addEvent(3);
        REQUIRE(isActive(sm, {&sm, &b, &bb}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 0);
        REQUIRE(bb.entered == 1);
        REQUIRE(bb.left == 0);

        sm.addEvent(3);
        REQUIRE(isActive(sm, {&sm, &a, &ab}));
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 1);
        REQUIRE(bb.entered == 1);
        REQUIRE(bb.left == 1);
        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
    }

    SECTION("from atomic to compound")
    {
        sm.addEvent(4);
        REQUIRE(isActive(sm, {&sm, &b, &ba}));
        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 0);
        REQUIRE(bb.entered == 0);

        sm.addEvent(4);
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 1);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 1);
        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 2);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 0);
    }

    SECTION("between ancestor and descendant")
    {
        sm.addEvent(5);
        REQUIRE(isActive(sm, {&sm, &a, &ab}));
        REQUIRE(a.entered == 2);
        REQUIRE(a.left == 1);
        REQUIRE(aa.entered == 1);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 0);
        REQUIRE(b.entered == 0);

        sm.addEvent(6);
        REQUIRE(isActive(sm, {&sm, &a, &aa}));
        REQUIRE(a.entered == 3);
        REQUIRE(a.left == 2);
        REQUIRE(aa.entered == 2);
        REQUIRE(aa.left == 1);
        REQUIRE(ab.entered == 1);
        REQUIRE(ab.left == 1);
        REQUIRE(b.entered == 0);
    }

    sm.stop();
    REQUIRE(isActive(sm, {}));
    for (auto state : {&a, &aa, &ab, &b, &ba, &bb})
        REQUIRE(state->entered == state->left);
}
#endif

TEST_CASE("targetless transitions block an event", "[transition]")
{
    using namespace sync;
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

    sm += aa + event(1) == ab;

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
        sm += aaa + event(1) == noTarget;

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

TEST_CASE("initial states are activated after start", "[transition]")
{
    using namespace sync;
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

TEST_CASE("transition actions are executed before state entries",
          "[transition]")
{
    //REQUIRE(false);
}

TEST_CASE("initial states during configuration change", "[transition]")
{
    using namespace sync;
    StateMachine_t sm;

    TrackingState<State_t> a("a", &sm);
    TrackingState<State_t> b("b", &sm);
    TrackingState<State_t> ba("ba", &b);
    TrackingState<State_t> baa("baa", &ba);
    TrackingState<State_t> bb("bb", &b);

    SECTION("without initial state")
    {
        sm += a + event(1) == b;

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &b, &ba, &baa}));

        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 0);
        REQUIRE(baa.entered == 1);
        REQUIRE(baa.left == 0);
        REQUIRE(bb.entered == 0);
    }

    SECTION("with initial state")
    {
        sm += a + event(1) == b;
        b.setInitialState(&bb);

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &b, &bb}));

        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 0);
        REQUIRE(bb.entered == 1);
        REQUIRE(bb.left == 0);
    }

    SECTION("initial state is ignored if the target is a sibling")
    {
        sm += a + event(1) == ba;
        b.setInitialState(&bb);

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &b, &ba, &baa}));

        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 0);
        REQUIRE(baa.entered == 1);
        REQUIRE(baa.left == 0);
        REQUIRE(bb.entered == 0);
    }

    SECTION("initial state is ignored if the target is a descendant")
    {
        sm += a + event(1) == baa;
        b.setInitialState(&bb);

        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
        sm.addEvent(1);
        REQUIRE(isActive(sm, {&sm, &b, &ba, &baa}));

        REQUIRE(a.entered == 1);
        REQUIRE(a.left == 1);
        REQUIRE(b.entered == 1);
        REQUIRE(b.left == 0);
        REQUIRE(ba.entered == 1);
        REQUIRE(ba.left == 0);
        REQUIRE(baa.entered == 1);
        REQUIRE(baa.left == 0);
        REQUIRE(bb.entered == 0);
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
    using StateMachine_t = fsm11::StateMachine<TransitionAllocator<TrackingTransitionAllocator<Transition<void>>>>;
    using State_t = StateMachine_t::state_type;

    int numTransitions = 0;

    {
        StateMachine_t sm{TrackingTransitionAllocator<void>(numTransitions)};

        TrackingState<State_t> a("a", &sm);
        TrackingState<State_t> b("b", &sm);

        sm += a + event(1) == b;
        REQUIRE(numTransitions == 1);
        sm += a + event(2) == b;
        REQUIRE(numTransitions == 2);
        sm += a + event(3) == b;
        REQUIRE(numTransitions == 3);
    }

    REQUIRE(numTransitions == 0);
}
