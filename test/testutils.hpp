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

#ifndef FSM11_TESTUTILS_HPP
#define FSM11_TESTUTILS_HPP

#include "catch.hpp"
#include <array>
#include <atomic>
#include <map>
#include <set>
#include <tuple>

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

struct DontCareTag
{
};
constexpr static DontCareTag X;

struct Pattern
{
    constexpr Pattern(int id)
        : m_id(id)
    {
    }

    bool operator<(Pattern other) const
    {
        return m_id < other.m_id;
    }

    int m_id;
};
constexpr static Pattern Y(1), Z(2);

template <typename TBase>
class TrackingState : public TBase
{
    typedef std::map<Pattern, int> PatternMap;

    static bool compare(PatternMap&, int a, int b)
    {
        return a == b;
    }

    static bool compare(PatternMap&, DontCareTag, int)
    {
        return true;
    }

    static bool compare(PatternMap& m, Pattern t, int v)
    {
        auto it = m.find(t);
        if (it != m.end())
            return it->second == v;

        m[t] = v;
        return true;
    }

    // TODO: Can I has an index_sequence<>, please?
    template <std::size_t I, std::size_t N, typename T>
    bool all(PatternMap& m, const T& t, std::true_type) const
    {
        return compare(m, std::get<I>(t), counters.at(I))
               && all<I+1, N>(m, t, std::integral_constant<bool, (I + 1 < N)>());
    }

    template <std::size_t I, std::size_t N, typename T>
    bool all(PatternMap&, const T&, std::false_type) const
    {
        return true;
    }

public:
    using event_type = typename TBase::event_type;

    template <typename T>
    explicit TrackingState(const char* name, T* parent = nullptr)
        : TBase(name, parent),
          entered(counters[0]),
          left(counters[1]),
          enteredInvoke(counters[2]),
          leftInvoke(counters[3])
    {
        for (auto& cnt : counters)
            cnt = 0;
    }

    template <typename... TArgs>
    bool operator==(const std::tuple<TArgs...>& t) const
    {
        PatternMap map;
        return all<0, sizeof...(TArgs)>(
                   map, t, std::integral_constant<bool, sizeof...(TArgs)>());
    }

    virtual void onEntry(event_type event) override
    {
        ++entered;
        TBase::onEntry(event);
    }

    virtual void onExit(event_type event) override
    {
        ++left;
        TBase::onExit(event);
    }

    virtual void enterInvoke() override
    {
        REQUIRE(isInvoked.exchange(true) == false);
        ++enteredInvoke;
        TBase::enterInvoke();
    }

    virtual void exitInvoke() override
    {
        REQUIRE(isInvoked.exchange(false) == true);
        ++leftInvoke;
        TBase::exitInvoke();
    }

    std::atomic_bool isInvoked{false};
    // The number of onEntry(), onExit(), enterInvoke(), exitInvoke() calls.
    std::array<std::atomic_int, 4> counters;

    std::atomic_int& entered;
    std::atomic_int& left;
    std::atomic_int& enteredInvoke;
    std::atomic_int& leftInvoke;
};

#endif // FSM11_TESTUTILS_HPP
