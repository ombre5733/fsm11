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

#include "error.hpp"

#ifdef FSM11_USE_WEOS
#include <weos/utility.hpp>
#else
#include <utility>
#endif // FSM11_USE_WEOS

using namespace FSM11STD;

namespace fsm11
{

class fsm11_category_impl : public error_category
{
public:
    virtual const char* name() const noexcept override
    {
        return "fsm11";
    }

    virtual auto message(int err_val) const
        -> decltype(declval<error_category>().message(0))
        override
    {
        switch (static_cast<ErrorCode>(err_val))
        {
        case ErrorCode::InvalidStateRelationship:
            return "Invalid state relationship";
        case ErrorCode::TransitionConflict:
            return "Transition conflict";
        case ErrorCode::ThreadPoolUnderflow:
            return "Thread pool underflow";
        default:
            return "Unkown error";
        }
    }
};

const error_category& fsm11_category() noexcept
{
    static fsm11_category_impl categoryInstance;
    return categoryInstance;
}

} // namespace fsm11
