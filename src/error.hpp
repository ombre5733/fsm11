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
enum class FsmErrorCode
{
    InvalidInitialState = 1,
    ThreadPoolUnderflow = 2
};

const FSM11STD::error_category& fsm11_category() noexcept;

inline
FSM11STD::error_code make_error_code(FsmErrorCode error) noexcept
{
    return FSM11STD::error_code(static_cast<int>(error), fsm11_category());
}

} // namespace fsm11

namespace FSM11STD
{

template <>
struct is_error_code_enum<fsm11::FsmErrorCode> : public true_type {};

} // namespace FSM11STD

namespace fsm11
{

class FsmError : public FSM11STD::system_error
{
public:
    FsmError(FsmErrorCode ec)
        : FSM11STD::system_error(ec)
    {
    }

    const FSM11STD::error_code& code() const noexcept
    {
        return m_errorCode;
    }

private:
    FSM11STD::error_code m_errorCode;
};

} // namespace fsm11

#endif // FSM11_ERROR_HPP
