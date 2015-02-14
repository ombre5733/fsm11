#ifndef FSM11_OPTIONS_HPP
#define FSM11_OPTIONS_HPP

#include "statemachine_fwd.hpp"
#include "detail/options.hpp"

#include <deque>
#include <memory>

namespace fsm11
{
namespace fsm11_detail
{

struct default_options
{
    using event_type = unsigned;
    using event_list_type = std::deque<unsigned>;
    using storage = type_list<>;
    using transition_allocator_type = std::allocator<Transition<void>>;

    static constexpr bool synchronous_dispatch = true;
    static constexpr bool multithreading_enable = false;

    static constexpr bool event_callbacks_enable = false;
    static constexpr bool configuration_change_callbacks_enable = false;
    static constexpr bool state_callbacks_enable = false;
};

} // namespace fsm11_detail


struct SynchronousEventDispatching
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool synchronous_dispatch = true;
    };
    //! \endcond
};

struct AsynchronousEventDispatching
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool synchronous_dispatch = false;
    };
    //! \endcond
};

template <typename TType>
struct EventType
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        using event_type = TType;
    };
    //! \endcond
};

template <typename TType>
struct EventListType
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        using event_list_type = TType;
    };
    //! \endcond
};

template <bool TEnable>
struct EnableEventCallbacks
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool event_callbacks_enable = TEnable;
    };
    //! \endcond
};

template <bool TEnable>
struct EnableConfigurationChangeCallbacks
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool configuration_change_callbacks_enable = TEnable;
    };
    //! \endcond
};

template <bool TEnable>
struct EnableStateCallbacks
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        static constexpr bool state_callbacks_enable = TEnable;
    };
    //! \endcond
};

template <typename... TTypes>
struct Storage
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        using storage = fsm11_detail::type_list<TTypes...>;
    };
    //! \endcond
};

template <typename TAllocator>
struct TransitionAllocator
{
    //! \cond
    template <typename TBase>
    struct pack : TBase
    {
        using transition_allocator_type = TAllocator;
    };
    //! \endcond
};

} // namespace fsm11

#endif // FSM11_OPTIONS_HPP
