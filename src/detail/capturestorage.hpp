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

#ifndef FSM11_DETAIL_CAPTURESTORAGE_HPP
#define FSM11_DETAIL_CAPTURESTORAGE_HPP

#include "../statemachine_fwd.hpp"
#include "options.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/utility.hpp>
#include <weos/tuple.hpp>
#else
#include <utility>
#include <tuple>
#endif // FSM11_USE_WEOS

#include <cstddef>

namespace fsm11
{
namespace fsm11_detail
{

template <typename TDerived, typename TList>
class CaptureStorage;

template <typename TDerived, typename... TTypes>
class CaptureStorage<TDerived, type_list<TTypes...>>
{
    typedef FSM11STD::tuple<TTypes...> tuple_type;

    // A helper to check the index agains the tuple size.
    template <std::size_t TIndex>
    struct get_type
    {
        static_assert(TIndex < FSM11STD::tuple_size<tuple_type>::value,
                      "Index out of bounds");

        using type = typename FSM11STD::tuple_element<TIndex, tuple_type>::type;
    };

public:
    template <std::size_t TIndex>
    const typename get_type<TIndex>::type& load() const
    {
        auto lock = derived().getLock();
        return FSM11STD::get<TIndex>(m_data);
    }

    template <std::size_t TIndex, typename TType>
    void store(TType&& value)
    {
        static_assert(TIndex < FSM11STD::tuple_size<tuple_type>::value,
                      "Index out of bounds");
        auto lock = derived().getLock();
        FSM11STD::get<TIndex>(m_data) = FSM11STD::forward<TType>(value);
    }

    template <typename TType>
    void setCaptureStorageCallback(TType&& callback)
    {
        m_updateStorageCallback = FSM11STD::forward<TType>(callback);
    }

protected:
    inline
    void invokeCaptureStorageCallback()
    {
        if (m_updateStorageCallback)
            m_updateStorageCallback();
    }

private:
    tuple_type m_data;
    FSM11STD::function<void()> m_updateStorageCallback;

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
class CaptureStorage<TDerived, type_list<>>
{
public:
    template <std::size_t TIndex>
    void load() const
    {
        static_assert(TIndex != TIndex, "No capture storage specified");
    }

    template <std::size_t TIndex, typename TType>
    void store(TType&& /*value*/)
    {
        static_assert(TIndex != TIndex, "No capture storage specified");
    }

    template <typename TType>
    void setCaptureStorageCallback(TType&&)
    {
        static_assert(!FSM11STD::is_same<TType, TType>::value,
                      "No capture storage specified");
    }

protected:
    inline
    void invokeCaptureStorageCallback()
    {
    }
};

template <typename TOptions>
struct get_storage
{
    typedef CaptureStorage<StateMachineImpl<TOptions>,
                           typename TOptions::capture_storage> type;
};

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_CAPTURESTORAGE_HPP
