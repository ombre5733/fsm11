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

#include "catch.hpp"

#include "../src/statemachine.hpp"


using namespace fsm11;

TEST_CASE("an FSM without multithreading support is smaller",
          "[multithreading]")
{
    using ssm_t = StateMachine<SynchronousEventDispatching,
                               MultithreadingEnable<false>>;
    using ssm_mt_t = StateMachine<SynchronousEventDispatching,
                                  MultithreadingEnable<true>>;
    REQUIRE(sizeof(ssm_t) < sizeof(ssm_mt_t));

    using asm_t = StateMachine<AsynchronousEventDispatching,
                               MultithreadingEnable<false>>;
    using asm_mt_t = StateMachine<AsynchronousEventDispatching,
                                  MultithreadingEnable<true>>;
    REQUIRE(sizeof(asm_t) < sizeof(asm_mt_t));
}

TEST_CASE("a multithreading FSM is a C++11 lockable")
{
    using sm_t = StateMachine<MultithreadingEnable<true>>;
    sm_t sm;

    std::unique_lock<sm_t> lock(sm, std::try_to_lock);
    REQUIRE(lock.owns_lock());
    lock.unlock();
    REQUIRE(!lock.owns_lock());
    lock.lock();
    REQUIRE(lock.owns_lock());
    lock.unlock();
}

TEST_CASE("mutual exclusion during configuration change", "[multithreading]")
{
    using StateMachine_t = StateMachine<SynchronousEventDispatching,
                                        MultithreadingEnable<true>>;
    using State_t = StateMachine_t::state_type;

    StateMachine_t sm;
    State_t a("a", &sm);
    State_t b("b", &sm);

    sm += a + event(1) > b;
    sm += b + event(2) > a;

    std::atomic_int counter{0};
    sm += b + noEvent ([&](int) { std::this_thread::yield(); return ++counter % 10000 != 0; }) > b;

    std::atomic_bool terminate{false};
    auto observer = std::async(std::launch::async, [&]{
        while (!terminate)
        {
            std::lock_guard<StateMachine_t> lock(sm);
            if (counter % 10000)
                return false;
        }
        return true;
    });

    sm.start();
    for (int tries = 0; tries < 10; ++tries)
    {
        sm.addEvent(1);
        sm.addEvent(2);
    }

    terminate = true;
    REQUIRE(observer.get());
}
