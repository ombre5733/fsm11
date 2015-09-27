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

#include "error.hpp"

namespace fsm11
{

class fsm11_category_impl : public FSM11STD::error_category
{
public:
    virtual const char* name() const noexcept override
    {
        return "fsm11";
    }

    virtual auto message(int err_val) const
        -> decltype(FSM11STD::declval<FSM11STD::error_category>().message(0))
        override
    {
        switch (static_cast<FsmErrorCode>(err_val))
        {
        case FsmErrorCode::InvalidInitialState:
            return "Invalid initial state";
        case FsmErrorCode::ThreadPoolUnderflow:
            return "Thread pool underflow";
        default:
            return "Unkown error";
        }
    }
};

const FSM11STD::error_category& fsm11_category() noexcept
{
    static fsm11_category_impl categoryInstance;
    return categoryInstance;
}

} // namespace fsm11
