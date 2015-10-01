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

#ifndef FSM11_THREADPOOL_HPP
#define FSM11_THREADPOOL_HPP

#include "statemachine_fwd.hpp"
#include "error.hpp"
#include "detail/threadedstatebase.hpp"

#ifdef FSM11_USE_WEOS
#include <boost/container/static_vector.hpp>
#include <weos/condition_variable.hpp>
#include <weos/future.hpp>
#include <weos/mutex.hpp>
#include <weos/thread.hpp>
#include <weos/tuple.hpp>
#include <weos/utility.hpp>
#else
#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#endif // FSM11_USE_WEOS


namespace fsm11
{
namespace fsm11_detail
{

template <bool... TValues>
struct all : FSM11STD::true_type
{
};

template <bool THead, bool... TTail>
struct all<THead, TTail...> : FSM11STD::conditional<THead,
                                                    all<TTail...>,
                                                    FSM11STD::false_type>::type
{
};

} // namespace fsm11_detail


template <std::size_t TSize>
class ThreadPool
{
    static_assert(TSize > 0, "The thread pool must be non-empty.");

    struct Task
    {
        Task(fsm11_detail::ThreadedStateBase& s)
            : state(s)
        {
        }

        Task(Task&& other)
            : promise(FSM11STD::move(other.promise)),
              state(other.state)
        {
        }

        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        FSM11STD::promise<void> promise;
        fsm11_detail::ThreadedStateBase& state;
    };

    struct Handle
    {
        explicit Handle(ThreadPool* pool, int id);

        Handle(const Handle&) = delete;
        Handle& operator=(const Handle&) = delete;

        bool hasChanged(ThreadPool* current) const
        {
            return m_pool != current;
        }

        ThreadPool* change(ThreadPool* current, int id);

        ThreadPool* m_pool;
        Handle* m_next{nullptr};
    };

public:
#ifdef FSM11_USE_WEOS
    template <typename... TAttributes>
    explicit ThreadPool(const FSM11STD::thread::attributes& attr,
                        const TAttributes&... attributes);
#else
    //! Constructs a thread pool.
    ThreadPool();
#endif // FSM11_USE_WEOS

    ThreadPool(const ThreadPool&) = delete;

    //! Move-constructs a thread pool from the \p other pool.
    ThreadPool(ThreadPool&& other);

    //! Destroys the thread pool.
    ~ThreadPool();

    ThreadPool& operator=(const ThreadPool&) = delete;

    //! Move-assigns the \p other pool to this one.
    ThreadPool& operator=(ThreadPool&& other);

    FSM11STD::future<void> enqueue(fsm11_detail::ThreadedStateBase& state);

private:
    FSM11STD::mutex m_poolMutex;

    FSM11STD::mutex m_workerMutex;
    FSM11STD::condition_variable m_workerCv;
    FSM11STD::condition_variable m_assignmentCv;
    unsigned m_assignedWorkers{0};
    int m_idleWorkers{TSize};
    Handle* m_handles{nullptr};
#ifdef FSM11_USE_WEOS
    boost::static_vector<Task, TSize> m_tasks;
#else
    FSM11STD::vector<Task> m_tasks;
#endif


    void moveTo(FSM11STD::unique_lock<FSM11STD::mutex>&& lock,
                ThreadPool* newPool);

    static void work(ThreadPool* pool, int id);

#ifdef FSM11_USE_WEOS
    template <typename TAttributes, std::size_t... TIndices>
    void construct(TAttributes&& attributes,
                   FSM11STD::integer_sequence<std::size_t, TIndices...>)
    {
        using namespace FSM11STD;

        unsigned workers = 0;
        try
        {
            call(constructOne(get<TIndices>(attributes), TIndices, workers)...);
            unique_lock<mutex> lock(m_workerMutex);
            m_assignmentCv.wait(lock,
                                [&]{ return m_assignedWorkers == workers; });
        }
        catch (...)
        {
            unique_lock<mutex> lock(m_workerMutex);
            m_assignmentCv.wait(lock,
                                [&]{ return m_assignedWorkers == workers; });
            moveTo(move(lock), nullptr);
            throw;
        }
    }

    int constructOne(const FSM11STD::thread::attributes& attr,
                     std::size_t idx, unsigned& workers)
    {
        FSM11STD::thread(attr, &ThreadPool::work, this, idx).detach();
        workers |= 1 << idx;
        return 0;
    }

    template <typename... T>
    void call(T...)
    {
    }
#endif
};

#ifdef FSM11_USE_WEOS
template <std::size_t TSize>
template <typename... TAttributes>
ThreadPool<TSize>::ThreadPool(const FSM11STD::thread::attributes& attr,
                              const TAttributes&... attributes)
{
    using namespace FSM11STD;

    static_assert(fsm11_detail::all<
                      is_same<TAttributes, thread::attributes>::value...
                  >::value,
                  "All arguments have to be thread attributes");
    static_assert(1 + sizeof...(TAttributes) == TSize,
                  "The number of thread attributes must equal the pool size.");

    construct(forward_as_tuple(attr, attributes...),
              make_index_sequence<1 + sizeof...(attributes)>());
}
#else
template <std::size_t TSize>
ThreadPool<TSize>::ThreadPool()
{
    using namespace FSM11STD;

    unsigned workers = 0;
    try
    {
        for (std::size_t idx = 0; idx < TSize; ++idx)
        {
            thread(&ThreadPool::work, this, idx).detach();
            workers |= 1 << idx;
        }

        unique_lock<mutex> lock(m_workerMutex);
        m_assignmentCv.wait(lock, [&]{ return m_assignedWorkers == workers; });
    }
    catch (...)
    {
        unique_lock<mutex> lock(m_workerMutex);
        m_assignmentCv.wait(lock, [&]{ return m_assignedWorkers == workers; });
        moveTo(move(lock), nullptr);
        throw;
    }
}
#endif // FSM11_USE_WEOS

template <std::size_t TSize>
ThreadPool<TSize>::ThreadPool(ThreadPool&& other)
{
    using namespace FSM11STD;

    lock_guard<mutex> otherPoolLock(m_poolMutex);

    unique_lock<mutex> otherWorkerLock(other.m_workerMutex);
    unsigned workers = other.m_assignedWorkers;
    m_handles = other.m_handles;
    other.moveTo(move(otherWorkerLock), this);

    unique_lock<mutex> thisWorkerLock(m_workerMutex);
    m_assignmentCv.wait(thisWorkerLock,
                        [&]{ return m_assignedWorkers == workers; });
}

template <std::size_t TSize>
ThreadPool<TSize>::~ThreadPool()
{
    moveTo(FSM11STD::unique_lock<FSM11STD::mutex>(m_workerMutex), nullptr);
}

template <std::size_t TSize>
auto ThreadPool<TSize>::operator=(ThreadPool&& other) -> ThreadPool&
{
    using namespace FSM11STD;

    if (this == &other)
        return *this;

    // It is important to use a dead-lock avoiding algorithm for the
    // two pool mutexes here.
    lock(m_poolMutex, other.m_poolMutex);
    lock_guard<mutex> thisPoolLock(m_poolMutex, adopt_lock);
    lock_guard<mutex> otherPoolLock(other.m_poolMutex, adopt_lock);

    moveTo(unique_lock<mutex>(m_workerMutex), nullptr);

    unique_lock<mutex> otherWorkerLock(other.m_workerMutex);
    unsigned workers = other.m_assignedWorkers;
    m_handles = other.m_handles;
    other.moveTo(move(otherWorkerLock), this);

    unique_lock<mutex> thisWorkerLock(m_workerMutex);
    m_assignmentCv.wait(thisWorkerLock,
                        [&]{ return m_assignedWorkers == workers; });

    return *this;
}

template <std::size_t TSize>
FSM11STD::future<void>
ThreadPool<TSize>::enqueue(fsm11_detail::ThreadedStateBase& state)
{
    using namespace FSM11STD;

    lock_guard<mutex> lock(m_workerMutex);
    if (m_idleWorkers == 0)
        throw FSM11_EXCEPTION(FsmError(make_error_code(
                FsmErrorCode::ThreadPoolUnderflow)));
    --m_idleWorkers;

    m_tasks.push_back(Task(state));
    m_workerCv.notify_one();
    return m_tasks.back().promise.get_future();
}

template <std::size_t TSize>
void ThreadPool<TSize>::moveTo(FSM11STD::unique_lock<FSM11STD::mutex>&& lock,
                               ThreadPool* newPool)
{
    for (Handle* iter = m_handles; iter != nullptr; iter = iter->m_next)
        iter->m_pool = newPool;
    m_workerCv.notify_all();
    m_assignmentCv.wait(lock, [this]{ return m_assignedWorkers == 0; });
    m_handles = nullptr;
}

template <std::size_t TSize>
void ThreadPool<TSize>::work(ThreadPool* pool, int id)
{
    using namespace FSM11STD;

    Handle handle(pool, id);
    while (pool)
    {
        unique_lock<mutex> lock(pool->m_workerMutex);
        pool->m_workerCv.wait(
                    lock,
                    [&](){ return handle.hasChanged(pool)
                                  || !pool->m_tasks.empty(); });
        if (!pool->m_tasks.empty())
        {
            Task task = move(pool->m_tasks.back());
            pool->m_tasks.pop_back();
            lock.unlock();
            try
            {
                task.state.invoke(task.state.m_exitRequest);
                task.promise.set_value();
            }
            catch (...)
            {
                task.promise.set_exception(current_exception());
            }
        }
        else if (handle.hasChanged(pool))
        {
            pool = handle.change(pool, id);
        }
    }
}

template <std::size_t TSize>
ThreadPool<TSize>::Handle::Handle(ThreadPool* pool, int id)
    : m_pool(pool)
{
    pool->m_workerMutex.lock();
    if (!pool->m_handles)
    {
        pool->m_handles = this;
    }
    else
    {
        Handle* iter = pool->m_handles;
        while (iter->m_next)
            iter = iter->m_next;
        iter->m_next = this;
    }
    pool->m_assignedWorkers |= 1 << id;
    pool->m_workerMutex.unlock();
    pool->m_assignmentCv.notify_one();
}

template <std::size_t TSize>
auto ThreadPool<TSize>::Handle::change(ThreadPool* current, int id)
    -> ThreadPool*
{
    current->m_assignedWorkers &= ~(1 << id);
    current->m_assignmentCv.notify_one();

    if (m_pool)
    {
        m_pool->m_workerMutex.lock();
        m_pool->m_assignedWorkers |= 1 << id;
        m_pool->m_workerMutex.unlock();
        m_pool->m_assignmentCv.notify_one();
    }

    return m_pool;
}

} // namespace fsm11

#endif // FSM11_THREADPOOL_HPP
