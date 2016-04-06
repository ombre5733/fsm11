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

#ifndef FSM11_DETAIL_THREADPOOL_HPP
#define FSM11_DETAIL_THREADPOOL_HPP

#include "../statemachine_fwd.hpp"

#include "threadedstatebase.hpp"
#include "../threadpool.hpp"

namespace fsm11
{
namespace fsm11_detail
{

class WithoutThreadPool
{
protected:
    struct NoThreadPool
    {
    };

    using internal_thread_pool_type = NoThreadPool;
};

template <typename TOptions>
class WithThreadPool
{
public:
    using thread_pool_type = ThreadPool<TOptions::thread_pool_size>;

protected:
    using internal_thread_pool_type = thread_pool_type;
};

template <typename TOptions>
struct get_threadpool
{
    using type = typename std::conditional<
                     TOptions::threadpool_enable,
                     WithThreadPool<TOptions>,
                     WithoutThreadPool>::type;
};

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_THREADPOOL_HPP
