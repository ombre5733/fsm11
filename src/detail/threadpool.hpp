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
    using type = typename FSM11STD::conditional<
                     TOptions::threadpool_enable,
                     WithThreadPool<TOptions>,
                     WithoutThreadPool>::type;
};

} // namespace fsm11_detail
} // namespace fsm11

#endif // FSM11_DETAIL_THREADPOOL_HPP
