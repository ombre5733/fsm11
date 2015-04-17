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
