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
        REQUIRE(invoked == 0);
        ++invoked;
        TBase::enterInvoke();
    }

    virtual std::exception_ptr exitInvoke() override
    {
        REQUIRE(invoked == 1);
        --invoked;
        return TBase::exitInvoke();
    }

    std::atomic_int entered = 0;
    std::atomic_int left = 0;
    std::atomic_int invoked = 0;
};

#endif // FSM11_TESTUTILS_HPP
