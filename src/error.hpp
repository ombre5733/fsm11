/*******************************************************************************
  fsm11 - A C++ library for finite state machines

  Copyright (c) 2015-2016, Manuel Freiberger
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

#ifndef FSM11_ERROR_HPP
#define FSM11_ERROR_HPP

#include "statemachine_fwd.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/system_error.hpp>
#else
#include <system_error>
#endif // FSM11_USE_WEOS

namespace fsm11
{
enum class ErrorCode
{
    InvalidStateRelationship = 1,
    TransitionConflict = 2,
    ThreadPoolUnderflow = 3
};

const std::error_category& fsm11_category() noexcept;

inline
std::error_code make_error_code(ErrorCode error) noexcept
{
    return std::error_code(static_cast<int>(error), fsm11_category());
}

} // namespace fsm11

namespace std
{

template <>
struct is_error_code_enum<fsm11::ErrorCode> : public true_type {};

} // namespace std

namespace fsm11
{

class Error : public std::system_error
{
public:
    Error(ErrorCode ec)
        : std::system_error(make_error_code(ec))
    {
    }
};

template <typename TTransition>
class TransitionConflictError : public Error
{
public:
    TransitionConflictError(TTransition* first, TTransition* second)
        : Error(ErrorCode::TransitionConflict),
          m_first(first),
          m_second(second)
    {
    }

    TTransition* first() const
    {
        return m_first;
    }

    TTransition* second() const
    {
        return m_second;
    }

private:
    TTransition* m_first;
    TTransition* m_second;
};

} // namespace fsm11

#endif // FSM11_ERROR_HPP
