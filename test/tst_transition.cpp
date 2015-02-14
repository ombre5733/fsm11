#include "catch.hpp"

#include "../statemachine.hpp"
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


template <typename TBase>
class TrackingState : public TBase
{
public:
    using event_type = typename TBase::event_type;

    template <typename T>
    explicit TrackingState(const char* name, T* parent = 0)
        : TBase(name, parent),
          entered(0),
          left(0),
          invoked(0)
    {
    }

    virtual void onEntry(event_type) override
    {
        ++entered;
    }

    virtual void onExit(event_type) override
    {
        ++left;
    }

    virtual void enterInvoke() override
    {
        REQUIRE(invoked == 0);
        ++invoked;
    }

    virtual std::exception_ptr exitInvoke() override
    {
        REQUIRE(invoked == 1);
        --invoked;
        return nullptr;
    }

    std::atomic_int entered = 0;
    std::atomic_int left = 0;
    std::atomic_int invoked = 0;
};

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
