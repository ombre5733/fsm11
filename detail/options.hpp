#ifndef FSM11_DETAIL_OPTIONS_HPP
#define FSM11_DETAIL_OPTIONS_HPP

namespace fsm11
{
namespace detail
{

template <typename... TOptions>
struct type_list
{
};



template <typename TList, typename T>
struct append;

template <typename... Ts, typename T>
struct append<type_list<Ts...>, T>
{
    using type = type_list<Ts..., T>;
};



template <typename TList>
struct reverse;

template <>
struct reverse<type_list<>>
{
    using type = type_list<>;
};

template <typename THead, typename... TTail>
struct reverse<type_list<THead, TTail...>>
{
    using type = typename append<typename reverse<type_list<TTail...>>::type, THead>::type;
};



template <typename TList>
struct pack_options_helper;

template <typename THead, typename... TTail>
struct pack_options_helper<type_list<THead, TTail...>>
{
    using type = typename THead::template pack<
                     typename pack_options_helper<type_list<TTail...>>::type>;
};

template <typename THead>
struct pack_options_helper<type_list<THead>>
{
    using type = THead;
};

template <typename TDefaultOptions, typename... TOptions>
class pack_options
{
    using reverted_options = typename reverse<type_list<TDefaultOptions, TOptions...>>::type;

public:
    using type = typename pack_options_helper<reverted_options>::type;
};

} // namespace detail
} // namespace fsm11

#endif // FSM11_DETAIL_OPTIONS_HPP
