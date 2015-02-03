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
TEST_CASE("hierarchy properties", "[state]")
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
}
