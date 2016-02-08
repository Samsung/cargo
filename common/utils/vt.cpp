/*
 *  Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  Contact: Lukasz Kostyra <l.kostyra@samsung.com>
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
 * @author  Lukasz Kostyra (l.kostyra@samsung.com)
 * @brief   VT-related utility functions
 */

#include "config.hpp"

#include "utils/vt.hpp"
#include "logger/logger.hpp"
#include "utils/exception.hpp"
#include "utils/fd-utils.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/vt.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

namespace {

const std::string TTY_DEV = "/dev/tty0";

} // namespace

namespace utils {

bool activateVT(const int& vt)
{
    int consoleFD = -1;

    try {
        consoleFD = utils::open(TTY_DEV, O_WRONLY);

        struct vt_stat vtstat;
        vtstat.v_active = 0;
        utils::ioctl(consoleFD, VT_GETSTATE, &vtstat);

        if (vtstat.v_active == vt) {
            LOGW("vt" << vt << " is already active.");
        }
        else {
            // activate vt
            utils::ioctl(consoleFD, VT_ACTIVATE, reinterpret_cast<void*>(vt));

            // wait until activation is finished
            utils::ioctl(consoleFD, VT_WAITACTIVE, reinterpret_cast<void*>(vt));
        }

        utils::close(consoleFD);
        return true;
    } catch(const UtilsException & e) {
        LOGE("Failed to activate vt" << vt << ": " << e.what() << " (" << getSystemErrorMessage(e.mErrno) << ")");
        utils::close(consoleFD);
        return false;
    }
}

} // namespace utils
