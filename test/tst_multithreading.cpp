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