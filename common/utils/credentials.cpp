/*
 *  Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Maciej Karpiuk (m.karpiuk2@samsung.com)
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
 * @author  Maciej Karpiuk (m.karpiuk2@samsung.com)
 * @brief   Credential management related functions
 */

#include "utils/credentials.hpp"
#include "utils/exception.hpp"
#include "logger/logger.hpp"

#include <unistd.h>
#include <grp.h>
#include <vector>

namespace utils {

// ------------------- syscall wrappers -------------------
void setgroups(const std::vector<gid_t>& gids)
{
    if (::setgroups(gids.size(), gids.data()) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in setgroups()", errno);
    }
}

void setregid(const gid_t rgid, const gid_t egid)
{
    if (::setregid(rgid, egid) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in setregid()", errno);
    }
}

void setreuid(const uid_t ruid, const uid_t euid)
{
    if (::setreuid(ruid, euid) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in setreuid()", errno);
    }
}

pid_t setsid()
{
    pid_t pid = ::setsid();
    if (pid == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Error in setsid()", errno);
    }
    return pid;
}

} // namespace utils
