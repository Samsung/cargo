/*
 *  Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Jan Olszak <j.olszak@samsung.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License
 */

/**
 * @file
 * @author  Jan Olszak (j.olszak@samsung.com)
 * @brief   Signal related functions
 */

#include "config.hpp"

#include "utils/signal.hpp"
#include "utils/exception.hpp"
#include "logger/logger.hpp"

#include <string>
#include <cerrno>
#include <cstring>
#include <csignal>

namespace utils {

// ------------------- syscall wrappers -------------------
void pthread_sigmask(int how, const ::sigset_t *set, ::sigset_t *get)
{
    int ret = ::pthread_sigmask(how, set, get);
    if (ret != 0) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in pthread_sigmask()", ret);
    }
}

void sigemptyset(::sigset_t *set)
{
    if (::sigemptyset(set) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in sigemptyset()", errno);
    }
}

void sigfillset(::sigset_t *set)
{
    if(-1 == ::sigfillset(set)) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in sigfillset()", errno);
    }
}

void sigaddset(::sigset_t *set, int signum)
{
    if (::sigaddset(set, signum) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in sigaddset()", errno);
    }
}

void sigpending(::sigset_t *set)
{
    if (::sigpending(set) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in sigpending()", errno);
    }
}

bool sigismember(const ::sigset_t *set, int signum)
{
    int ret = ::sigismember(set, signum);
    if (ret == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in sigismember()", errno);
    }
    return ret == 1;
}

int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout)
{
    int ret = ::sigtimedwait(set, info, timeout);
    if (ret == -1) {
        if (errno == EAGAIN) {
            return 0;
        }

        THROW_UTILS_EXCEPTION_ERRNO_E("Error in sigtimedwait()", errno);
    }
    return ret;
}

void sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    if(-1 == ::sigaction(signum, act, oldact)) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in sigaction()", errno);
    }
}

namespace {
void changeSignal(int how, const int sigNum) {
    ::sigset_t set;
    utils::sigemptyset(&set);
    utils::sigaddset(&set, sigNum);
    utils::pthread_sigmask(how, &set, nullptr);
}
}// namespace

// ------------------- higher level functions -------------------
::sigset_t getSignalMask()
{
    ::sigset_t set;
    utils::pthread_sigmask(0 /*ignored*/, nullptr /*get the oldset*/, &set);
    return set;
}

bool isSignalPending(const int sigNum)
{
    ::sigset_t set;
    utils::sigpending(&set);

    return utils::sigismember(&set, sigNum);
}

bool waitForSignal(const int sigNum, int timeoutMs)
{
    int timeoutS = timeoutMs / 1000;
    timeoutMs -= timeoutS * 1000;

    struct ::timespec timeout = {
        timeoutS,
        timeoutMs * 1000000
    };

    ::sigset_t set;
    utils::sigemptyset(&set);
    utils::sigaddset(&set, sigNum);

    ::siginfo_t info;
    if(utils::sigtimedwait(&set, &info, &timeout) == 0) {
        return false;
    }

    return true;
}

bool isSignalBlocked(const int sigNum)
{
    ::sigset_t set = getSignalMask();
    return utils::sigismember(&set, sigNum);
}

void signalBlock(const int sigNum)
{
    changeSignal(SIG_BLOCK, sigNum);
}

void signalBlockAllExcept(const std::initializer_list<int>& signals)
{
    ::sigset_t set;
    utils::sigfillset(&set);
    for(const int s: signals) {
        utils::sigaddset(&set, s);
    }
    utils::pthread_sigmask(SIG_BLOCK, &set, nullptr);
}

void signalUnblock(const int sigNum)
{
    changeSignal(SIG_UNBLOCK, sigNum);
}

std::vector<std::pair<int, struct ::sigaction>> signalIgnore(const std::initializer_list<int>& signals)
{
    struct ::sigaction act;
    struct ::sigaction old;
    act.sa_handler = SIG_IGN;
    std::vector<std::pair<int, struct ::sigaction>> oldAct;

    for(const int s: signals) {
        utils::sigaction(s, &act, &old);
        oldAct.emplace_back(s, old);
    }

    return oldAct;
}

struct ::sigaction signalSet(const int sigNum, const struct ::sigaction *sigAct)
{
    struct ::sigaction old;
    utils::sigaction(sigNum, sigAct, &old);
    return old;
}

void sendSignal(const pid_t pid, const int sigNum)
{
    if (-1 == ::kill(pid, sigNum)) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error during killing pid: " << std::to_string(pid)
                                      << " sigNum: " + std::to_string(sigNum), errno);
    }
}

} // namespace utils
