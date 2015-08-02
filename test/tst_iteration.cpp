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

#include <map>

using namespace fsm11;

using StateMachine_t = fsm11::StateMachine<>;
using State_t = StateMachine_t::state_type;

namespace
{

bool contains(const std::map<const State_t*, int>& set, State_t* state)
{
    return set.find(state) != set.end();
}

} // anonymous namespace

// Copied over from Bothe.
#define CREATE_TYPE_CHECKER(t)                                                 \
    template <typename T, typename U = int>                                    \
    struct has_ ## t : std::false_type {};                                     \
    template <typename T>                                                      \
    struct has_ ## t <T, decltype((void)std::declval<typename T::t>(),0)> : std::true_type {};

CREATE_TYPE_CHECKER(difference_type)
CREATE_TYPE_CHECKER(value_type)
CREATE_TYPE_CHECKER(pointer)
CREATE_TYPE_CHECKER(reference)
CREATE_TYPE_CHECKER(iterator_category)

CREATE_TYPE_CHECKER(iterator)
CREATE_TYPE_CHECKER(const_iterator)

TEST_CASE("iterator requirements", "[iteration]")
{
    // Every C++ container must have an 'iterator' and a 'const_iterator' type.
    REQUIRE(has_iterator<StateMachine_t>::value);
    REQUIRE(has_const_iterator<StateMachine_t>::value);

    using iterator = StateMachine_t::iterator;
    using const_iterator = StateMachine_t::const_iterator;

    // Make sure that the required types are present in the iterators.
    // Note: The types are implemented in the common base class and it
    // suffices to check one iterator type here.
    REQUIRE(has_difference_type<iterator>::value);
    REQUIRE(has_value_type<iterator>::value);
    REQUIRE(has_pointer<iterator>::value);
    REQUIRE(has_reference<iterator>::value);
    REQUIRE(has_iterator_category<iterator>::value);

    REQUIRE(has_difference_type<const_iterator>::value);
    REQUIRE(has_value_type<const_iterator>::value);
    REQUIRE(has_pointer<const_iterator>::value);
    REQUIRE(has_reference<const_iterator>::value);
    REQUIRE(has_iterator_category<const_iterator>::value);

    // Would have to check CopyConstructible, CopyAssignable, Destructible...
    // Podness should do for now.
    REQUIRE(std::is_pod<iterator>::value);

    // TODO: Check C++14-requirement of singularness!

    StateMachine_t sm;
    SECTION("default constructible")
    {
        iterator iter1;
        iterator iter2;
    }
    SECTION("operators")
    {
        iterator iter1;
        iterator iter2;
        iter1 = sm.begin();
        iter2 = sm.begin();
        SECTION("dereference")
        {
            *iter1;
        }
        SECTION("equality")
        {
            REQUIRE(iter1 == iter2);
            iter2 = sm.end();
            REQUIRE(iter1 != iter2);
        }
        SECTION("prefix increment")
        {
            iter2 = ++iter1;
            REQUIRE(iter1 == iter2);
            REQUIRE(iter1 == sm.end());
        }
        SECTION("postfix increment")
        {
            iter2 = iter1++;
            REQUIRE(iter1 != iter2);
            REQUIRE(iter2 == sm.begin());
            REQUIRE(iter1 == sm.end());
        }
    }
}

TEST_CASE("iterate over a single state", "[iteration]")
{
    State_t s("s");
    const State_t& cs = s;

    // The iteration starts with the state itself. A begin()-iterator is
    // never equal to an end()-iterator. If the begin()-iterator is advanced,
    // we have to reach the end()-iterator.

    SECTION("pre-order iterator")
    {
        REQUIRE(s.pre_order_begin()   != s.pre_order_end());
        REQUIRE(s.pre_order_cbegin()  != s.pre_order_cend());
        REQUIRE(cs.pre_order_begin()  != cs.pre_order_end());
        REQUIRE(cs.pre_order_cbegin() != cs.pre_order_cend());

        REQUIRE(++s.pre_order_begin()   == s.pre_order_end());
        REQUIRE(++s.pre_order_cbegin()  == s.pre_order_cend());
        REQUIRE(++cs.pre_order_begin()  == cs.pre_order_end());
        REQUIRE(++cs.pre_order_cbegin() == cs.pre_order_cend());

        REQUIRE(&s == &*s.pre_order_begin());
        REQUIRE(&s == &*s.pre_order_cbegin());
        REQUIRE(&s == &*cs.pre_order_begin());
        REQUIRE(&s == &*cs.pre_order_cbegin());
    }

    SECTION("post-order iterator")
    {
        REQUIRE(s.post_order_begin()   != s.post_order_end());
        REQUIRE(s.post_order_cbegin()  != s.post_order_cend());
        REQUIRE(cs.post_order_begin()  != cs.post_order_end());
        REQUIRE(cs.post_order_cbegin() != cs.post_order_cend());

        REQUIRE(++s.post_order_begin()   == s.post_order_end());
        REQUIRE(++s.post_order_cbegin()  == s.post_order_cend());
        REQUIRE(++cs.post_order_begin()  == cs.post_order_end());
        REQUIRE(++cs.post_order_cbegin() == cs.post_order_cend());

        REQUIRE(&s == &*s.post_order_begin());
        REQUIRE(&s == &*s.post_order_cbegin());
        REQUIRE(&s == &*cs.post_order_begin());
        REQUIRE(&s == &*cs.post_order_cbegin());
    }

    SECTION("child iterator")
    {
        REQUIRE(s.child_begin() == s.child_end());
        REQUIRE(s.child_cbegin() == s.child_cend());

        REQUIRE(cs.child_begin() == cs.child_end());
        REQUIRE(cs.child_cbegin() == cs.child_cend());
    }

    SECTION("atomic iterator")
    {
        REQUIRE(s.atomic_begin()   != s.atomic_end());
        REQUIRE(s.atomic_cbegin()  != s.atomic_cend());
        REQUIRE(cs.atomic_begin()  != cs.atomic_end());
        REQUIRE(cs.atomic_cbegin() != cs.atomic_cend());

        REQUIRE(++s.atomic_begin()   == s.atomic_end());
        REQUIRE(++s.atomic_cbegin()  == s.atomic_cend());
        REQUIRE(++cs.atomic_begin()  == cs.atomic_end());
        REQUIRE(++cs.atomic_cbegin() == cs.atomic_cend());

        REQUIRE(&s == &*s.atomic_begin());
        REQUIRE(&s == &*s.atomic_cbegin());
        REQUIRE(&s == &*cs.atomic_begin());
        REQUIRE(&s == &*cs.atomic_cbegin());
    }
}

TEST_CASE("iterate over state hierarchy", "[iteration]")
{
    State_t p("p");
    State_t c1("c1", &p);
    State_t c2("c2", &p);
    State_t c3("c3", &p);
    State_t c11("c11", &c1);
    State_t c12("c12", &c1);
    State_t c31("c31", &c3);
    State_t c32("c32", &c3);

    const State_t& cp = p;
    const State_t& cc1 = c1;

    std::vector<const State_t*> visited;

    SECTION("pre-order iterator")
    {
        SECTION("mutable iterator from pre_order_begin()/pre_order_end()")
        {
            for (auto iter = p.pre_order_begin(); iter != p.pre_order_end(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("mutable iterator from pre_order_begin()/pre_order_end() postfix")
        {
            for (auto iter = p.pre_order_begin(); iter != p.pre_order_end(); iter++)
                visited.push_back(&*iter);
        }

        SECTION("const-iterator from pre_order_begin()/pre_order_end()")
        {
            for (auto iter = cp.pre_order_begin(); iter != cp.pre_order_end(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("const-iterator from pre_order_cbegin()/pre_order_cend()")
        {
            for (auto iter = p.pre_order_cbegin(); iter != p.pre_order_cend(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("iteration via mutable pre-order-view")
        {
            for (State_t& state : p.pre_order_subtree())
                visited.push_back(&state);
        }

        SECTION("iteration via const pre-order-view")
        {
            for (const State_t& state : cp.pre_order_subtree())
                visited.push_back(&state);
        }

        REQUIRE(visited.size() == 8);

        auto visited_iter = visited.begin();
        for (const State_t* iter : {&p, &c1, &c11, &c12, &c2, &c3, &c31, &c32})
        {
            REQUIRE(*visited_iter == iter);
            ++visited_iter;
        }
    }

    SECTION("pre-order iterator over sub-tree")
    {
        SECTION("mutable iterator from pre_order_begin()/pre_order_end()")
        {
            for (auto iter = c1.pre_order_begin(); iter != c1.pre_order_end(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("const-iterator from pre_order_begin()/pre_order_end()")
        {
            for (auto iter = cc1.pre_order_begin(); iter != cc1.pre_order_end(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("const-iterator from pre_order_cbegin()/pre_order_cend()")
        {
            for (auto iter = c1.pre_order_cbegin(); iter != c1.pre_order_cend(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("iteration via mutable pre-order-view")
        {
            for (State_t& state : c1.pre_order_subtree())
                visited.push_back(&state);
        }

        SECTION("iteration via const pre-order-view")
        {
            for (const State_t& state : cc1.pre_order_subtree())
                visited.push_back(&state);
        }

        REQUIRE(visited.size() == 3);

        auto visited_iter = visited.begin();
        for (const State_t* iter : {&c1, &c11, &c12})
        {
            REQUIRE(*visited_iter == iter);
            ++visited_iter;
        }
    }

    SECTION("post-order iterator")
    {
        SECTION("mutable iterator from post_order_begin()/post_order_end()")
        {
            for (auto iter = p.post_order_begin(); iter != p.post_order_end(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("mutable iterator from post_order_begin()/post_order_end() postfix")
        {
            for (auto iter = p.post_order_begin(); iter != p.post_order_end(); iter++)
                visited.push_back(&*iter);
        }

        SECTION("const-iterator from post_order_begin()/post_order_end()")
        {
            for (auto iter = cp.post_order_begin(); iter != cp.post_order_end(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("const-iterator from post_order_cbegin()/post_order_cend()")
        {
            for (auto iter = p.post_order_cbegin(); iter != p.post_order_cend(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("iteration via mutable post-order-view")
        {
            for (State_t& state : p.post_order_subtree())
                visited.push_back(&state);
        }

        SECTION("iteration via const post-order-view")
        {
            for (const State_t& state : cp.post_order_subtree())
                visited.push_back(&state);
        }

        REQUIRE(visited.size() == 8);

        auto visited_iter = visited.begin();
        for (const State_t* iter : {&c11, &c12, &c1, &c2, &c31, &c32, &c3, &p})
        {
            REQUIRE(*visited_iter == iter);
            ++visited_iter;
        }
    }

    SECTION("post-order iterator over sub-tree")
    {
        SECTION("mutable iterator from post_order_begin()/post_order_end()")
        {
            for (auto iter = c1.post_order_begin(); iter != c1.post_order_end(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("const-iterator from post_order_begin()/post_order_end()")
        {
            for (auto iter = cc1.post_order_begin(); iter != cc1.post_order_end(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("const-iterator from post_order_cbegin()/post_order_cend()")
        {
            for (auto iter = c1.post_order_cbegin(); iter != c1.post_order_cend(); ++iter)
                visited.push_back(&*iter);
        }

        SECTION("iteration via mutable post-order-view")
        {
            for (State_t& state : c1.post_order_subtree())
                visited.push_back(&state);
        }

        SECTION("iteration via const post-order-view")
        {
            for (const State_t& state : cc1.post_order_subtree())
                visited.push_back(&state);
        }

        REQUIRE(visited.size() == 3);

        auto visited_iter = visited.begin();
        for (const State_t* iter : {&c11, &c12, &c1})
        {
            REQUIRE(*visited_iter == iter);
            ++visited_iter;
        }
    }
}

TEST_CASE("iterate over empty state machine", "[iteration]")
{
    StateMachine_t sm;
    const StateMachine_t& csm = sm;

    // There is always the root state in the state machine. A begin()-iterator
    // can never be an end()-iterator. If the begin()-iterator is advanced,
    // we have to reach the end()-iterator.

    SECTION("pre-order iterator")
    {
        REQUIRE(sm.begin()   != sm.end());
        REQUIRE(sm.cbegin()  != sm.cend());
        REQUIRE(csm.begin()  != csm.end());
        REQUIRE(csm.cbegin() != csm.cend());

        REQUIRE(++sm.begin()   == sm.end());
        REQUIRE(++sm.cbegin()  == sm.cend());
        REQUIRE(++csm.begin()  == csm.end());
        REQUIRE(++csm.cbegin() == csm.cend());

        REQUIRE(sm.begin()++   == sm.begin());
        REQUIRE(sm.cbegin()++  == sm.cbegin());
        REQUIRE(csm.begin()++  == csm.begin());
        REQUIRE(csm.cbegin()++ == csm.cbegin());
    }

    SECTION("post-order iterator")
    {
        REQUIRE(sm.post_order_begin()   != sm.post_order_end());
        REQUIRE(sm.post_order_cbegin()  != sm.post_order_cend());
        REQUIRE(csm.post_order_begin()  != csm.post_order_end());
        REQUIRE(csm.post_order_cbegin() != csm.post_order_cend());

        REQUIRE(++sm.post_order_begin()   == sm.post_order_end());
        REQUIRE(++sm.post_order_cbegin()  == sm.post_order_cend());
        REQUIRE(++csm.post_order_begin()  == csm.post_order_end());
        REQUIRE(++csm.post_order_cbegin() == csm.post_order_cend());

        REQUIRE(sm.post_order_begin()++   == sm.post_order_begin());
        REQUIRE(sm.post_order_cbegin()++  == sm.post_order_cbegin());
        REQUIRE(csm.post_order_begin()++  == csm.post_order_begin());
        REQUIRE(csm.post_order_cbegin()++ == csm.post_order_cbegin());
    }

    SECTION("atomic iterator")
    {
        REQUIRE(sm.atomic_begin()   != sm.atomic_end());
        REQUIRE(sm.atomic_cbegin()  != sm.atomic_cend());
        REQUIRE(csm.atomic_begin()  != csm.atomic_end());
        REQUIRE(csm.atomic_cbegin() != csm.atomic_cend());

        REQUIRE(++sm.atomic_begin()   == sm.atomic_end());
        REQUIRE(++sm.atomic_cbegin()  == sm.atomic_cend());
        REQUIRE(++csm.atomic_begin()  == csm.atomic_end());
        REQUIRE(++csm.atomic_cbegin() == csm.atomic_cend());

        REQUIRE(sm.atomic_begin()++   == sm.atomic_begin());
        REQUIRE(sm.atomic_cbegin()++  == sm.atomic_cbegin());
        REQUIRE(csm.atomic_begin()++  == csm.atomic_begin());
        REQUIRE(csm.atomic_cbegin()++ == csm.atomic_cbegin());
    }

    SECTION("child iterator")
    {
        REQUIRE(sm.child_begin() == sm.child_end());
        REQUIRE(sm.child_cbegin() == sm.child_cend());

        REQUIRE(csm.child_begin() == csm.child_end());
        REQUIRE(csm.child_cbegin() == csm.child_cend());
    }
}

TEST_CASE("iterate over non-empty state machine", "[iteration]")
{
    StateMachine_t sm;
    const StateMachine_t& csm = sm;
    State_t p("p", &sm);
    State_t c1("c1", &p);
    State_t c2("c2", &p);
    State_t c3("c3", &p);
    State_t c11("c11", &c1);
    State_t c12("c12", &c1);
    State_t c31("c31", &c3);
    State_t c32("c32", &c3);
    c3.setChildMode(ChildMode::Parallel);

    std::map<const State_t*, int> visitOrder;
    int counter = 0;

    SECTION("pre-order iterator")
    {
        SECTION("mutable iterator from begin()/end()")
        {
            for (StateMachine_t::pre_order_iterator iter = sm.begin();
                 iter != sm.end(); ++iter)
            {
                visitOrder[&*iter] = ++counter;
            }
        }

        SECTION("const-iterator from begin()/end()")
        {
            for (StateMachine_t::const_pre_order_iterator iter = csm.begin();
                 iter != csm.end(); ++iter)
            {
                visitOrder[&*iter] = ++counter;
            }
        }

        SECTION("const-iterator from cbegin()/cend()")
        {
            for (StateMachine_t::const_pre_order_iterator iter = sm.cbegin();
                 iter != sm.cend(); ++iter)
            {
                visitOrder[&*iter] = ++counter;
            }
        }

        REQUIRE(counter == 9);

        // Parents have to be visited before their children.
        REQUIRE(visitOrder[&p] < visitOrder[&c1]);
        REQUIRE(visitOrder[&p] < visitOrder[&c2]);
        REQUIRE(visitOrder[&p] < visitOrder[&c3]);
        REQUIRE(visitOrder[&c1] < visitOrder[&c11]);
        REQUIRE(visitOrder[&c1] < visitOrder[&c12]);
        REQUIRE(visitOrder[&c3] < visitOrder[&c31]);
        REQUIRE(visitOrder[&c3] < visitOrder[&c32]);
    }

    SECTION("pre-order iterator with child skipping")
    {
        StateMachine_t::pre_order_iterator iter = sm.begin();
        ++iter;
        ++iter;
        REQUIRE(&*iter == &c1);

        iter.skipChildren();
        while (iter != sm.end())
        {
            visitOrder[&*iter] = ++counter;
            ++iter;
        }

        REQUIRE(counter == 5);
        REQUIRE(contains(visitOrder, &c1));
        REQUIRE(contains(visitOrder, &c2));
        REQUIRE(contains(visitOrder, &c3));
        REQUIRE(contains(visitOrder, &c31));
        REQUIRE(contains(visitOrder, &c32));

        REQUIRE(!contains(visitOrder, &c11));
        REQUIRE(!contains(visitOrder, &c12));
    }

    SECTION("post-order iterator")
    {
        SECTION("mutable iterator from begin()/end()")
        {
            for (StateMachine_t::post_order_iterator iter = sm.post_order_begin();
                 iter != sm.post_order_end(); ++iter)
            {
                visitOrder[&*iter] = ++counter;
            }
        }

        SECTION("const-iterator from begin()/end()")
        {
            for (StateMachine_t::const_post_order_iterator iter
                     = csm.post_order_begin();
                 iter != csm.post_order_end(); ++iter)
            {
                visitOrder[&*iter] = ++counter;
            }
        }

        SECTION("const-iterator from cbegin()/cend()")
        {
            for (StateMachine_t::const_post_order_iterator iter
                     = sm.post_order_cbegin();
                 iter != sm.post_order_cend(); ++iter)
            {
                visitOrder[&*iter] = ++counter;
            }
        }

        REQUIRE(counter == 9);

        // Parents have to be visited after their children.
        REQUIRE(visitOrder[&p] > visitOrder[&c1]);
        REQUIRE(visitOrder[&p] > visitOrder[&c2]);
        REQUIRE(visitOrder[&p] > visitOrder[&c3]);
        REQUIRE(visitOrder[&c1] > visitOrder[&c11]);
        REQUIRE(visitOrder[&c1] > visitOrder[&c12]);
        REQUIRE(visitOrder[&c3] > visitOrder[&c31]);
        REQUIRE(visitOrder[&c3] > visitOrder[&c32]);
    }

    SECTION("atomic iterator")
    {
        SECTION("mutable iterator from begin()/end()")
        {
            for (StateMachine_t::atomic_iterator iter = sm.atomic_begin();
                 iter != sm.atomic_end(); ++iter)
            {
                visitOrder[&*iter] = ++counter;
            }
        }

        SECTION("const-iterator from begin()/end()")
        {
            for (StateMachine_t::const_atomic_iterator iter = csm.atomic_begin();
                 iter != csm.atomic_end(); ++iter)
            {
                visitOrder[&*iter] = ++counter;
            }
        }

        SECTION("const-iterator from cbegin()/cend()")
        {
            for (StateMachine_t::const_atomic_iterator iter = sm.atomic_cbegin();
                 iter != sm.atomic_cend(); ++iter)
            {
                visitOrder[&*iter] = ++counter;
            }
        }

        REQUIRE(counter == 5);

        REQUIRE(contains(visitOrder, &c2));
        REQUIRE(contains(visitOrder, &c11));
        REQUIRE(contains(visitOrder, &c12));
        REQUIRE(contains(visitOrder, &c31));
        REQUIRE(contains(visitOrder, &c32));
    }

    SECTION("works with <algorithm>")
    {
        auto predicate = [](const State_t&) { return true; };
        REQUIRE(std::count_if(sm.begin(),   sm.end(),
                              predicate) == 9);
        REQUIRE(std::count_if(sm.cbegin(),  sm.cend(),
                              predicate) == 9);
        REQUIRE(std::count_if(csm.begin(),  csm.end(),
                              predicate) == 9);
        REQUIRE(std::count_if(csm.cbegin(), csm.cend(),
                              predicate) == 9);

        REQUIRE(std::count_if(sm.post_order_begin(), sm.post_order_end(),
                              predicate) == 9);
        REQUIRE(std::count_if(csm.post_order_begin(), csm.post_order_end(),
                              predicate) == 9);
        REQUIRE(std::count_if(sm.post_order_cbegin(), sm.post_order_cend(),
                              predicate) == 9);
        REQUIRE(std::count_if(csm.post_order_cbegin(), csm.post_order_cend(),
                              predicate) == 9);

        REQUIRE(std::count_if(sm.atomic_begin(), sm.atomic_end(),
                              predicate) == 5);
        REQUIRE(std::count_if(csm.atomic_begin(), csm.atomic_end(),
                              predicate) == 5);
        REQUIRE(std::count_if(sm.atomic_cbegin(), sm.atomic_cend(),
                              predicate) == 5);
        REQUIRE(std::count_if(csm.atomic_cbegin(), csm.atomic_cend(),
                              predicate) == 5);
    }

    SECTION("pre-order, post-order and atomic iterators are equal for leaf states")
    {
        auto pre = sm.pre_order_begin();
        auto post = sm.post_order_begin();
        for (auto atomic = sm.atomic_begin(); atomic != sm.atomic_end();
             ++atomic, ++pre, ++post)
        {
            while (!pre->isAtomic() && pre != sm.pre_order_end())
                ++pre;
            while (!post->isAtomic() && post != sm.post_order_end())
                ++post;

            REQUIRE(pre != sm.pre_order_end());
            REQUIRE(post != sm.post_order_end());

            REQUIRE(&*atomic == &*pre);
            REQUIRE(&*atomic == &*post);
        }
    }
}

TEST_CASE("child iterators from pre-order iterator", "[iteration]")
{
    // TODO
}
