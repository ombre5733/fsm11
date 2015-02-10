#ifndef FSM11_DETAIL_STORAGE_HPP
#define FSM11_DETAIL_STORAGE_HPP

#include "statemachine_fwd.hpp"
#include "options.hpp"

#include <cstddef>
#include <utility>
#include <tuple>

namespace fsm11
{
namespace fsm11_detail
{

template <typename TDerived, typename TList>
class Storage;

template <typename TDerived, typename... TTypes>
class Storage<TDerived, type_list<TTypes...>>
{
    typedef std::tuple<TTypes...> tuple_type;

    // A helper to check the index agains the tuple size.
    template <std::size_t TIndex>
    struct get_type
    {
        static_assert(TIndex < std::tuple_size<tuple_type>::value,
                      "Index out of bounds");

        using type = typename std::tuple_element<TIndex, tuple_type>::type;
    };

public:
    template <std::size_t TIndex>
    const typename get_type<TIndex>::type& load() const
    {
        auto lock = derived().getLock();
        return std::get<TIndex>(m_data);
    }

    template <std::size_t TIndex, typename TType>
    void store(TType&& value)
    {
        static_assert(TIndex < std::tuple_size<tuple_type>::value,
                      "Index out of bounds");
        auto lock = derived().getLock();
        std::get<TIndex>(m_data) = std::forward<TType>(value);
    }

private:
    tuple_type m_data;

    TDerived& derived()
    {
        return *static_cast<TDerived*>(this);
    }

    const TDerived& derived() const
    {
        return *static_cast<const TDerived*>(this);
    }
};

template <typename TDerived>
class Storage<TDerived, type_list<>>
{
public:
    template <std::size_t TIndex>
    void load() const
    {
        static_assert(TIndex != TIndex, "No storage specified");
    }

    template <std::size_t TIndex, typename TType>
    void store(TType&& /*value*/)
    {
        static_assert(TIndex != TIndex, "No storage specified");
    }
};

template <typename TOptions>
struct get_storage
{
    typedef Storage<StateMachineImpl<TOptions>,
                    typename TOptions::storage> type;
};

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_STORAGE_HPP
