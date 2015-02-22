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

#ifndef FSM11_DETAIL_OPTIONS_HPP
#define FSM11_DETAIL_OPTIONS_HPP

namespace fsm11
{
namespace fsm11_detail
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

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_OPTIONS_HPP
