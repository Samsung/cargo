/*
 *  Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Lukasz Pawelczyk <l.pawelczyk@partner.samsung.com>
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
 * @author  Lukasz Pawelczyk (l.pawelczyk@partner.samsung.com)
 * @brief   Exceptions for the server
 */


#ifndef COMMON_UTILS_EXCEPTION_HPP
#define COMMON_UTILS_EXCEPTION_HPP

#include "logger/logger.hpp"
#include <vector>
#include "utils/typeinfo.hpp"
#include <stdexcept>

namespace utils {

/**
 * Return string describing error number
 * it is wrapper for strerror_r
 */
std::string getSystemErrorMessage();
std::string getSystemErrorMessage(int err);

/**
 * Base class for exceptions in utils
 */
struct UtilsException : public std::runtime_error {
    explicit UtilsException(const std::string& msg, int err = 0)
        :   std::runtime_error(msg),
            mErrno(err),
            mFile(""),
            mLine(-1),
            mFunc("")
    {
    }

    void setLocation(const char *file, int line, const char *func = "")
    {
        mFile = file == NULL ? "" : file + sizeof(PROJECT_SOURCE_DIR);
        mLine = line;
        mFunc = func == NULL ? "" : func;
    }

    void log() const
    {
        if (logger::Logger::getLogLevel() <= logger::LogLevel::ERROR) {
        const std::string& msg = what();
        logger::Logger::logMessage(logger::LogLevel::ERROR,
                                   "[" + utils::getTypeName(*this) + "] " +
                                   (mErrno != 0 ? msg + ": " + getSystemErrorMessage(mErrno) + "(" + std::to_string(mErrno) + ")" : msg),
                                   mFile,
                                   mLine,
                                   mFunc,
                                   PROJECT_SOURCE_DIR);
        }
    }

    struct UtilsException& _setLocation(const char *file, int line, const char *func = "")
    {
        setLocation(file, line, func);
        return *this;
    }
    struct UtilsException& _log()
    {
        log();
        return *this;
    }

    const int mErrno;
    const char *mFile;
    int mLine;
    const char *mFunc;
};

#define THROW_EXCEPTION_IMPL(EXCEPTION_TYPE, MSG, ERRNO)            \
    do {                                                            \
        int _err = ERRNO;                                           \
        std::ostringstream _msg;                                    \
        _msg << MSG;                                                \
        throw EXCEPTION_TYPE(_msg.str(), _err)                      \
              ._setLocation(__FILE__, __LINE__,  __func__)          \
              ._log();                                              \
    } while (0)

#define THROW_EXCEPTION_3A(EXCEPTION_TYPE, MSG, ERRNO) \
    THROW_EXCEPTION_IMPL(EXCEPTION_TYPE, MSG, ERRNO)

#define THROW_EXCEPTION_2A(EXCEPTION_TYPE, MSG) \
    THROW_EXCEPTION_IMPL(EXCEPTION_TYPE, MSG, 0)

#define THROW_EXCEPTION_1A(EXCEPTION_TYPE) \
    THROW_EXCEPTION_IMPL(EXCEPTION_TYPE, "", 0)

#define THROW_CHOOSE(_1,_2,_3,F,...) F

#define THROW_EXCEPTION(...) THROW_CHOOSE(__VA_ARGS__,              \
                                THROW_EXCEPTION_3A(__VA_ARGS__),    \
                                THROW_EXCEPTION_2A(__VA_ARGS__),    \
                                THROW_EXCEPTION_1A(__VA_ARGS__),    \
                                )

#define THROW_UTILS_EXCEPTION(...) THROW_EXCEPTION(UtilsException, __VA_ARGS__)

struct EventFDException: public UtilsException {

    explicit EventFDException(const std::string& msg, int err = 0) : UtilsException(msg, err) {}
};


void fillInStackTrace(std::vector<std::string>& bt);

} // namespace utils

#endif // COMMON_UTILS_EXCEPTION_HPP
