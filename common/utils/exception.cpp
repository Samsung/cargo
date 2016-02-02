/*
 *  Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Mateusz Malicki  <m.malicki2@samsung.com>
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
 * @author  Mateusz Malicki (m.malicki2@samsung.com)
 * @brief   Utils base exception helper implementation
 */

#include "config.hpp"

#include "utils/exception.hpp"

#include <string>
#include <cstring>
#include <cerrno>
#include <memory>

#include <execinfo.h>
#include <cxxabi.h>

namespace {

std::string demangle(const std::string& name)
{
    int status;
    std::string ret;
    char *realname = abi::__cxa_demangle(name.c_str(), NULL, NULL, &status);

    if (status == 0) {
        ret.append(realname);
        free(realname);
    } else {
        ret.append(name);
    }

    return ret;
}

}

namespace utils {

const int ERROR_MESSAGE_BUFFER_CAPACITY = 256;
const int STACK_FETCH_DEPTH = 50;

std::string getSystemErrorMessage()
{
    return getSystemErrorMessage(errno);
}

std::string getSystemErrorMessage(int err)
{
    char buf[ERROR_MESSAGE_BUFFER_CAPACITY];
    return strerror_r(err, buf, sizeof(buf));
}

void fillInStackTrace(std::vector<std::string>& bt)
{
    void *frames[STACK_FETCH_DEPTH];
    size_t size;
    size = ::backtrace(frames, sizeof(frames)/sizeof(*frames));
    char** symbollist = ::backtrace_symbols(frames, size);

    // skip 2 stack frames:
    //   [0] = fillInStackTrace
    //   [1] = Exception constructor
    //   ---------------------------
    //   [2] = location where Exception was created
    for (size_t i = 2; i < size; i++) {
        int namePos = 0; // offsets to tokens found in symbol
        int offsPos = 0;
        int tailPos = 0;

        for (char *p = symbollist[i]; *p != 0; ++p) {
            if (*p == '(') {
                namePos = p - symbollist[i] + 1;
            } else if (*p == '+') {
                offsPos = p - symbollist[i] + 1;
            } else if (*p == ')' && offsPos) {
                tailPos = p - symbollist[i] + 1;
                break;
            }
        }

        const std::string& item = symbollist[i];
        if (namePos && tailPos && namePos < offsPos) {
            std::string demangled = demangle(item.substr(namePos, offsPos - namePos - 1));
            bt.push_back(item.substr(0, namePos - 1) + ":" + demangled + "+" + item.substr(offsPos));
        } else {
            // add whole line as is.
            bt.push_back(item);
        }
    }
    ::free(symbollist);
}

} // namespace utils
