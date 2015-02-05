#include "catch.hpp"

#include "../statemachine.hpp"

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

#if 0
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

    State_t* found = p.findChild("c1");
    REQUIRE(found != 0);
    REQUIRE(!std::strcmp("c1", found->name()));

    found = p.findChild("c1", "c12");
    REQUIRE(found != 0);
    REQUIRE(!std::strcmp("c12", found->name()));
}
#endif
TEST_CASE("state relationship", "[state]")
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