#ifndef FSM11_TESTUTILS_HPP
#define FSM11_TESTUTILS_HPP

#include "catch.hpp"
#include <set>

template <typename T>
bool isActive(const T& sm,
              const std::set<const typename T::state_type*>& expected)
{
    for (const auto& state : sm.pre_order_subtree())
    {
        auto iter = expected.find(&state);
        if (iter != expected.end())
            REQUIRE(state.isActive());
        else
            REQUIRE(!state.isActive());
    }
    return true;
}

#endif // FSM11_TESTUTILS_HPP
