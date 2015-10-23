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

    FSM11STD::mutex m_workerMutex; // TODO: rethink the locking policy
    FSM11STD::condition_variable m_workerCv;
    FSM11STD::condition_variable m_assignmentCv;
    unsigned m_assignedWorkers{0};
    int m_idleWorkers{TSize};
    Handle* m_handles{nullptr};
#ifdef FSM11_USE_WEOS
    boost::container::static_vector<Task, TSize> m_tasks;
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
    // two pool mutexes here or we dead-lock the application.
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
        throw FSM11_EXCEPTION(Error(ErrorCode::ThreadPoolUnderflow));
    --m_idleWorkers;

    m_tasks.emplace_back(state);
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
            lock.lock();
            ++pool->m_idleWorkers;
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
