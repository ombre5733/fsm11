/*******************************************************************************
  fsm11 - A C++11-compliant framework for finite state machines

  Copyright (c) 2015, Manuel Freiberger
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
