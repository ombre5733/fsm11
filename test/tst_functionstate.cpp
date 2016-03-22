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

#include "../src/functionstate.hpp"
#include "../src/statemachine.hpp"

using StateMachine_t = fsm11::StateMachine<>;
using State_t = fsm11::FunctionState<StateMachine_t>;

TEST_CASE("construct function state", "[functionstate]")
{
    State_t s0("s0");
    REQUIRE(!s0.isActive());
    REQUIRE(s0.parent() == nullptr);
    REQUIRE(!s0.entryFunction());
    REQUIRE(!s0.exitFunction());

    State_t s1("s1", nullptr, nullptr);
    REQUIRE(!s1.isActive());
    REQUIRE(s1.parent() == nullptr);
    REQUIRE(!s1.entryFunction());
    REQUIRE(!s1.exitFunction());

    int entered = 0;
    int left = 0;
    auto enter = [&](int) { ++entered; };
    auto leave = [&](int) { ++left; };

    State_t s2("s2", enter, nullptr, &s1);
    REQUIRE(s2.parent() == &s1);

    REQUIRE(s2.entryFunction());
    REQUIRE(!s2.exitFunction());
    s2.entryFunction()(0);
    REQUIRE(entered == 1);
    REQUIRE(left == 0);

    State_t s3("s3", nullptr, leave);
    REQUIRE(!s3.entryFunction());
    REQUIRE(s3.exitFunction());
    s3.exitFunction()(0);
    REQUIRE(entered == 1);
    REQUIRE(left == 1);

    State_t s4("s4", enter, leave);
    REQUIRE(s4.entryFunction());
    REQUIRE(s4.exitFunction());
    s4.entryFunction()(0);
    s4.exitFunction()(0);
    REQUIRE(entered == 2);
    REQUIRE(left == 2);
}

TEST_CASE("set actions of function state", "[functionstate]")
{
    int entered = 0;
    int left = 0;
    auto enter = [&](int ev) { entered += ev; };
    auto leave = [&](int ev) { left += ev; };

    State_t s("s");
    REQUIRE(!s.entryFunction());
    REQUIRE(!s.exitFunction());

    s.setEntryFunction(enter);
    REQUIRE(s.entryFunction() != nullptr);
    REQUIRE(!s.exitFunction());

    s.entryFunction()(3);
    REQUIRE(entered == 3);
    REQUIRE(left == 0);
    REQUIRE(s.entryFunction() != nullptr);
    REQUIRE(!s.exitFunction());

    s.setExitFunction(leave);
    REQUIRE(s.entryFunction() != nullptr);
    REQUIRE(s.exitFunction() != nullptr);

    s.exitFunction()(5);
    REQUIRE(entered == 3);
    REQUIRE(left == 5);
    REQUIRE(s.entryFunction() != nullptr);
    REQUIRE(s.exitFunction() != nullptr);

    s.setEntryFunction(nullptr);
    REQUIRE(!s.entryFunction());
    REQUIRE(s.exitFunction() != nullptr);

    s.setExitFunction(nullptr);
    REQUIRE(!s.entryFunction());
    REQUIRE(!s.exitFunction());
}
