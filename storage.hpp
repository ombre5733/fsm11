#ifndef FSM11_DETAIL_STORAGE_HPP
#define FSM11_DETAIL_STORAGE_HPP

#include <cstddef>
#include <tuple>

namespace statemachine
{
namespace detail
{
template <typename... TTypes>
class Storage;

template <>
class Storage<>
{
public:
    template <std::size_t TIndex>
    void load() const
    {
        static_assert(TIndex != TIndex, "No storage specified");
    }

    template <std::size_t TIndex, typename TType>
    void store(TType&& element)
    {
        static_assert(TIndex != TIndex, "No storage specified");
    }
};

template <typename... TTypes>
class Storage
{
    typedef std::tuple<TTypes...> tuple_type;

public:
    template <std::size_t TIndex>
    const typename std::tuple_element<TIndex, tuple_type>::type& load() const
    {
        return std::get<TIndex>(m_data);
    }

    template <std::size_t TIndex, typename TType>
    void store(TType&& element)
    {
        std::get<TIndex>(m_data) = std::forward<TType>(element);
    }

private:
    tuple_type m_data;
};

} // namespace detail
} // namespace statemachine

#endif // FSM11_DETAIL_STORAGE_HPP
