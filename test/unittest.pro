################################################################################
# fsm11 - A C++ library for finite state machines
#
# Copyright (c) 2015-2016, Manuel Freiberger
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# - Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
################################################################################

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += DEBUG

QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra
QMAKE_LFLAGS += -pthread -Wl,--no-as-needed

INCLUDEPATH += ../src/

SOURCES += \
    ../src/fsm11.cpp \
    main.cpp \
    tst_behavior.cpp \
    tst_capturestorage.cpp \
    tst_configurationchangecallback.cpp \
    tst_error.cpp \
    tst_event.cpp \
    tst_eventcallback.cpp \
    tst_eventlist.cpp \
    tst_exceptions.cpp \
    tst_functionstate.cpp \
    tst_hierarchy.cpp \
    tst_iteration.cpp \
    tst_multithreading.cpp \
    tst_state.cpp \
    tst_statecallbacks.cpp \
    tst_statemachine.cpp \
    tst_threadedstate.cpp \
    tst_threadpool.cpp \
    tst_transition.cpp \
    tst_transitionconflict.cpp \
    tst_transitionconflictcallback.cpp

HEADERS += \
    ../src/error.hpp \
    ../src/exitrequest.hpp \
    ../src/functionstate.hpp \
    ../src/historystate.hpp \
    ../src/options.hpp \
    ../src/state.hpp \
    ../src/statemachine_fwd.hpp \
    ../src/statemachine.hpp \
    ../src/threadedfunctionstate.hpp \
    ../src/threadedstate.hpp \
    ../src/threadpool.hpp \
    ../src/transition.hpp \
    ../src/detail/callbacks.hpp \
    ../src/detail/capturestorage.hpp \
    ../src/detail/eventdispatcher.hpp \
    ../src/detail/multithreading.hpp \
    ../src/detail/options.hpp \
    ../src/detail/scopeguard.hpp \
    ../src/detail/threadedstatebase.hpp \
    ../src/detail/threadpool.hpp

HEADERS += catch.hpp \
           fsm11_user_config.hpp \
           testutils.hpp
