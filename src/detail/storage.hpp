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

#ifndef FSM11_DETAIL_STORAGE_HPP
#define FSM11_DETAIL_STORAGE_HPP

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
class Storage;

template <typename TDerived, typename... TTypes>
class Storage<TDerived, type_list<TTypes...>>
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
    void setUpdateStorageCallback(TType&&)
    {
        static_assert(!FSM11STD::is_same<TType, TType>::value,
                      "No storage specified");
    }

protected:
    inline
    void invokeUpdateStorageCallback()
    {
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

    template <typename TType>
    void setUpdateStorageCallback(TType&& callback)
    {
        m_updateStorageCallback = FSM11STD::forward<TType>(callback);
    }

protected:
    inline
    void invokeUpdateStorageCallback()
    {
        if (m_updateStorageCallback)
            m_updateStorageCallback();
    }

private:
    FSM11STD::function<void()> m_updateStorageCallback;
};

template <typename TOptions>
struct get_storage
{
    typedef Storage<StateMachineImpl<TOptions>,
                    typename TOptions::capture_storage> type;
};

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_STORAGE_HPP
