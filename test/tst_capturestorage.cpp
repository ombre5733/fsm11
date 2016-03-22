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
#include "testutils.hpp"

using namespace fsm11;

TEST_CASE("empty storage", "[storage]")
{
    using StateMachine_t = fsm11::StateMachine<CaptureStorage<>>;
    StateMachine_t sm;
}

TEST_CASE("storage with one built-in type", "[storage]")
{
    SECTION("int")
    {
        using StateMachine_t = fsm11::StateMachine<CaptureStorage<int>>;
        StateMachine_t sm;
        sm.store<0>(21);
        REQUIRE(sm.load<0>() == 21);
        sm.store<0>(6);
        REQUIRE(sm.load<0>() == 6);
    }

    SECTION("double")
    {
        using StateMachine_t = fsm11::StateMachine<CaptureStorage<double>>;
        StateMachine_t sm;
        sm.store<0>(3.14);
        REQUIRE(sm.load<0>() == 3.14);
        sm.store<0>(2.71);
        REQUIRE(sm.load<0>() == 2.71);
    }

    SECTION("const char*")
    {
        const char* greeting = "hello";
        const char* person = "Manuel";
        using StateMachine_t = fsm11::StateMachine<CaptureStorage<const char*>>;
        StateMachine_t sm;
        sm.store<0>(greeting);
        REQUIRE(sm.load<0>() == greeting);
        sm.store<0>(person);
        REQUIRE(sm.load<0>() == person);
    }
}

TEST_CASE("storage with two built-in types", "[storage]")
{
    using StateMachine_t = fsm11::StateMachine<CaptureStorage<double, int>>;
    StateMachine_t sm;

    sm.store<0>(3.14);
    sm.store<1>(42);

    REQUIRE(sm.load<0>() == 3.14);
    REQUIRE(sm.load<1>() == 42);
}

enum Color
{
    Red,
    Green,
    Yellow
};

TEST_CASE("storage with user-defined enum", "[storage]")
{
    using StateMachine_t = fsm11::StateMachine<CaptureStorage<Color>>;
    StateMachine_t sm;

    sm.store<0>(Red);
    REQUIRE(sm.load<0>() == Red);

    sm.store<0>(Green);
    REQUIRE(sm.load<0>() == Green);
}

TEST_CASE("access storage in guard", "[storage]")
{
    using StateMachine_t = fsm11::StateMachine<CaptureStorage<int>>;
    using State_t = StateMachine_t::state_type;

    StateMachine_t sm;
    State_t a("a", &sm);
    State_t b("b", &sm);
    State_t c("c", &sm);

    sm += a + noEvent ([&](int){ return sm.load<0>() == 1; }) > b;
    sm += a + noEvent ([&](int){ return sm.load<0>() == 2; }) > c;

    SECTION("set to 0")
    {
        sm.store<0>(0);
        sm.start();
        REQUIRE(isActive(sm, {&sm, &a}));
    }

    SECTION("set to 1")
    {
        sm.store<0>(1);
        sm.start();
        REQUIRE(isActive(sm, {&sm, &b}));
    }

    SECTION("set to 2")
    {
        sm.store<0>(2);
        sm.start();
        REQUIRE(isActive(sm, {&sm, &c}));
    }
}

TEST_CASE("capture storage callback is executed", "[storage]")
{
    using StateMachine_t = fsm11::StateMachine<CaptureStorage<int>>;
    using State_t = StateMachine_t::state_type;

    StateMachine_t sm;
    State_t a("a", &sm);
    State_t b("b", &sm);
    State_t c("c", &sm);

    int value = 0;
    sm.setCaptureStorageCallback([&] { sm.store<0>(value); });

    value = 31;
    sm.start();
    REQUIRE(sm.load<0>() == 31);
}
