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
    explicit UtilsException(const std::string& error,
                            int errno_,
                            const char *file,
                            const char *func,
                            int line,
                            logger::LogLevel level = logger::LogLevel::DEBUG)
        :   std::runtime_error(error),
            mErrno(errno_),
            mFile(std::string(file?file:__FILE__)),
            mFunc(std::string(func?func:__func__)),
            mLine((line!=-1)?__LINE__:line),
            mLevel(level) {
        if (logger::Logger::getLogLevel() <= mLevel) {
            logger::Logger::logMessage(mLevel,
                                       (mErrno!=0)?(error + " (errno: " + getSystemErrorMessage(mErrno) + ")"):error,
                                       mFile,
                                       mLine,
                                       mFunc,
                                       PROJECT_SOURCE_DIR);
        }
    }

    explicit UtilsException(const std::string& error, int errno_)
        :  UtilsException(error, errno_, nullptr, nullptr, -1) {}

    explicit UtilsException(const std::string& error)
        :  UtilsException(error, errno) {}

    const int mErrno;
    const std::string mFile;
    const std::string mFunc;
    const int mLine;
    logger::LogLevel mLevel;
};

#define THROW_UTILS_EXCEPTION__(LEVEL, MSG, ERRNO, FILE, LINE, FUNC)\
    do {                                                            \
        int errnoTmp = ERRNO;                                       \
        std::ostringstream messageStream__;                         \
        messageStream__ << MSG;                                     \
        throw UtilsException(messageStream__.str(),                 \
                             errnoTmp,                              \
                             FILE,                                  \
                             FUNC,                                  \
                             LINE,                                  \
                             logger::LogLevel::LEVEL);              \
    } while (0)

#define THROW_UTILS_EXCEPTION_W(MSG) \
    THROW_UTILS_EXCEPTION__(WARN, MSG, 0, __FILE__, __LINE__, __func__)

#define THROW_UTILS_EXCEPTION_ERRNO_W(MSG, ERRNO) \
    THROW_UTILS_EXCEPTION__(WARN, MSG, ERRNO, __FILE__, __LINE__, __func__)

#define THROW_UTILS_EXCEPTION_D(MSG) \
    THROW_UTILS_EXCEPTION__(DEBUG, MSG, 0, __FILE__, __LINE__, __func__)

#define THROW_UTILS_EXCEPTION_ERRNO_D(MSG, ERRNO) \
    THROW_UTILS_EXCEPTION__(DEBUG, MSG, ERRNO, __FILE__, __LINE__, __func__)

#define THROW_UTILS_EXCEPTION_E(MSG) \
    THROW_UTILS_EXCEPTION__(ERROR, MSG, 0, __FILE__, __LINE__, __func__)

#define THROW_UTILS_EXCEPTION_ERRNO_E(MSG, ERRNO) \
    THROW_UTILS_EXCEPTION__(ERROR, MSG, ERRNO, __FILE__, __LINE__, __func__)

struct EventFDException: public UtilsException {

    explicit EventFDException(const std::string& error) : UtilsException(error) {}
};


} // namespace utils

#endif // COMMON_UTILS_EXCEPTION_HPP
