/*******************************************************************************
  fsm11 - A C++ library for finite state machines

  Copyright (c) 2015-2016, Manuel Freiberger
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

#ifndef FSM11_TESTUTILS_HPP
#define FSM11_TESTUTILS_HPP

#include "catch.hpp"

#include <array>
#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
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
        constexpr size_t nargs = sizeof...(TArgs);
        constexpr bool   hasArgs = nargs > 0;
        return all<0, nargs>(map, t, std::integral_constant<bool, hasArgs>());
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

template <typename T>
struct ConfigurationChangeTracker
{
    explicit ConfigurationChangeTracker(T& sm)
        : m_sm(sm)
    {
        sm.setConfigurationChangeCallback([this] {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_configurationChanged = true;
            m_cv.notify_all();
        });
    }

    ~ConfigurationChangeTracker()
    {
        m_sm.setConfigurationChangeCallback(nullptr);
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return m_configurationChanged; });
        m_configurationChanged = false;
    }

    T& m_sm;
    std::mutex m_mutex;
    bool m_configurationChanged{false};
    std::condition_variable m_cv;
};

#endif // FSM11_TESTUTILS_HPP
