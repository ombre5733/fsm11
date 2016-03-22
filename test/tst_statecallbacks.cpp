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

#include "catch.hpp"

#include "../src/statemachine.hpp"

#include <algorithm>
#include <initializer_list>

using namespace fsm11;


namespace syncSM
{
using StateMachine_t = StateMachine<>;
using CallbackStateMachine_t = StateMachine<
                                   StateCallbacksEnable<true>>;
using CallbackState_t = State<CallbackStateMachine_t>;
} // namespace syncSM

namespace asyncSM
{
using StateMachine_t = StateMachine<AsynchronousEventDispatching>;
using CallbackStateMachine_t = StateMachine<
                                   AsynchronousEventDispatching,
                                   StateCallbacksEnable<true>>;
} // namespace asyncSM

TEST_CASE("an FSM without state callbacks compiles only when no callback is set",
          "[callback]")
{
    syncSM::StateMachine_t sync_sm;
    // When one of the following lines is included, the test must not compile.
    // sync_sm.setStateEntryCallback([](syncSM::StateMachine_t::state_type){});
    // sync_sm.setStateExitCallback([](syncSM::StateMachine_t::state_type){});

    asyncSM::StateMachine_t async_sm;
    // When one of the following lines is included, the test must not compile.
    // async_sm.setStateEntryCallback([](asyncSM::StateMachine_t::state_type){});
    // async_sm.setStateExitCallback([](asyncSM::StateMachine_t::state_type){});
}

TEST_CASE("disabling state callbacks makes the FSM smaller", "[callback]")
{
    REQUIRE(sizeof(syncSM::StateMachine_t)
            < sizeof(syncSM::CallbackStateMachine_t));
    REQUIRE(sizeof(asyncSM::StateMachine_t)
            < sizeof(asyncSM::CallbackStateMachine_t));
}

SCENARIO("state callback execution", "[callback]")
{
    GIVEN ("a state machine")
    {
        using namespace syncSM;

        int numDispatchedEvents = 0;
        int numDiscardedEvents = 0;

        CallbackStateMachine_t sm;
        CallbackState_t a("a", &sm);
        CallbackState_t aa("aa", &a);
        CallbackState_t b("b", &sm);

        std::vector<CallbackState_t*> states;

        WHEN ("no callback is set")
        {
            sm.start();
            THEN ("nothing happens")
            {
                REQUIRE(states.empty());
            }
        }

        WHEN ("a state entry callback is set")
        {
            sm.setStateEntryCallback(
                        [&](CallbackState_t* s){ states.push_back(s); });
            sm.start();
            THEN ("it is executed once per state entered")
            {
                REQUIRE(states.size() == 3);
                for (auto s : {(CallbackState_t*)&sm, &a, &aa})
                    REQUIRE(std::find(states.begin(), states.end(), s) != states.end());
            }

            WHEN ("the state machine is restarted")
            {
                sm.stop();
                sm.start();
                THEN ("the callback is executed again")
                {
                    REQUIRE(states.size() == 6);
                }
            }

            WHEN ("the callback is reset")
            {
                sm.setStateEntryCallback(nullptr);
                sm.stop();
                sm.start();
                THEN ("it is not executed anymore")
                {
                    REQUIRE(states.size() == 3);
                }
            }
        }

        WHEN ("a state exit callback is set")
        {
            sm.setStateExitCallback(
                        [&](CallbackState_t* s){ states.push_back(s); });
            sm.start();
            REQUIRE(states.size() == 0);
            sm.stop();
            THEN ("it is executed once per state left")
            {
                REQUIRE(states.size() == 3);
                for (auto s : {(CallbackState_t*)&sm, &a, &aa})
                    REQUIRE(std::find(states.begin(), states.end(), s) != states.end());
            }

            WHEN ("the state machine is restarted and stopped again")
            {
                sm.start();
                REQUIRE(states.size() == 3);
                sm.stop();
                THEN ("the callback is executed again")
                {
                    REQUIRE(states.size() == 6);
                }
            }

            WHEN ("the callback is reset")
            {
                sm.setStateExitCallback(nullptr);
                sm.start();
                sm.stop();
                THEN ("it is not executed anymore")
                {
                    REQUIRE(states.size() == 3);
                }
            }
        }
    }
}
