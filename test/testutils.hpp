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
