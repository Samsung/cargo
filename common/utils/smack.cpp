/*
 *  Copyright (C) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file
 * @author  Lukasz Pawelczyk (l.pawelczyk@samsung.com)
 * @brief   SMACK utils implementation
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "logger/logger.hpp"
#include "utils/exception.hpp"
#include "utils/smack.hpp"
#include "utils/fs.hpp"

#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/xattr.h>


namespace utils {
// ------------------- syscall wrappers -------------------
std::string getxattr(const std::string &path, const std::string &name)
{
    char value[SMACK_LABEL_MAX_LEN + 1];

    int ret = ::getxattr(path.c_str(), name.c_str(), value, sizeof(value));
    if (ret == -1) {
        if (errno == ENODATA) {
            return "";
        }

        THROW_EXCEPTION(UtilsException, "Error in getxattr(" << path << ")", errno);
    }

    value[ret] = '\0';
    return value;
}

std::string lgetxattr(const std::string &path, const std::string &name)
{
    char value[SMACK_LABEL_MAX_LEN + 1];

    int ret = ::lgetxattr(path.c_str(), name.c_str(), value, sizeof(value));
    if (ret == -1) {
        if (errno == ENODATA) {
            return "";
        }

        THROW_EXCEPTION(UtilsException, "Error in lgetxattr(" << path << ")", errno);
    }

    value[ret] = '\0';
    return value;
}

void removexattr(const std::string &path, const std::string &name)
{
    if (::removexattr(path.c_str(), name.c_str()) == -1) {
        if (errno == ENODATA) {
            return;
        }

        THROW_EXCEPTION(UtilsException, "Error in removexattr(" << path << ")", errno);
    }
}

void lremovexattr(const std::string &path, const std::string &name)
{
    if (::lremovexattr(path.c_str(), name.c_str()) == -1) {
        if (errno == ENODATA) {
            return;
        }

        THROW_EXCEPTION(UtilsException, "Error in lremovexattr(" << path << ")", errno);
    }
}

void setxattr(const std::string &path, const std::string &name,
              const std::string &value, int flags)
{
    if (::setxattr(path.c_str(), name.c_str(), value.c_str(), value.size(), flags) == -1) {
        THROW_EXCEPTION(UtilsException, "Error in setxattr(" << path << ")", errno);
    }
}

void lsetxattr(const std::string &path, const std::string &name,
               const std::string &value, int flags)
{
    if (::lsetxattr(path.c_str(), name.c_str(), value.c_str(), value.size(), flags) == -1) {
        THROW_EXCEPTION(UtilsException, "Error in setxattr(" << path << ")", errno);
    }
}


// ------------------- higher level functions -------------------
bool isSmackActive()
{
    try {
        struct statfs sfbuf = utils::statfs(SMACK_MOUNT_PATH);
        if ((uint32_t)sfbuf.f_type == (uint32_t)SMACK_MAGIC) {
            return true;
        }
    } catch(const UtilsException & e) {}

    return false;
}

bool isSmackNamespaceActive()
{
    return utils::exists("/proc/self/attr/label_map");
}

std::string smackXattrName(SmackLabelType type)
{
    switch (type) {
    case SmackLabelType::SMACK_LABEL_ACCESS:
        return "security.SMACK64";
    case SmackLabelType::SMACK_LABEL_EXEC:
        return "security.SMACK64EXEC";
    case SmackLabelType::SMACK_LABEL_MMAP:
        return "security.SMACK64MMAP";
    case SmackLabelType::SMACK_LABEL_TRANSMUTE:
        return "security.SMACK64TRANSMUTE";
    case SmackLabelType::SMACK_LABEL_IPIN:
        return "security.SMACK64IPIN";
    case SmackLabelType::SMACK_LABEL_IPOUT:
        return "security.SMACK64IPOUT";
    default:
        THROW_EXCEPTION(UtilsException, "Wrong SMACK label type passed");
    }
}

std::string smackGetSelfLabel()
{
    return utils::readFileStream("/proc/self/attr/current");
}

std::string smackGetFileLabel(const std::string &path,
                              SmackLabelType labelType,
                              bool followLinks)
{
    const std::string xattrName = smackXattrName(labelType);

    if (followLinks) {
        return utils::getxattr(path, xattrName);
    } else {
        return utils::lgetxattr(path, xattrName);
    }
}

void smackSetFileLabel(const std::string &path,
                       const std::string &label,
                       SmackLabelType labelType,
                       bool followLinks)
{
    const std::string xattrName = smackXattrName(labelType);

    if (label.size() > SMACK_LABEL_MAX_LEN) {
        THROW_EXCEPTION(UtilsException, "SMACK label too long");
    }

    if (label.empty()) {
        if (followLinks) {
            utils::removexattr(path, xattrName);
        } else {
            utils::lremovexattr(path, xattrName);
        }
    } else {
        if (followLinks) {
            utils::setxattr(path, xattrName, label, 0);
        } else {
            utils::lsetxattr(path, xattrName, label, 0);
        }
    }
}

void copySmackLabel(const std::string& src, const std::string& dst, bool resolveLink)
{
    smackSetFileLabel(dst, smackGetFileLabel(src, SmackLabelType::SMACK_LABEL_ACCESS, resolveLink),
                                                  SmackLabelType::SMACK_LABEL_ACCESS, resolveLink);
    smackSetFileLabel(dst, smackGetFileLabel(src, SmackLabelType::SMACK_LABEL_EXEC, resolveLink),
                                                  SmackLabelType::SMACK_LABEL_EXEC, resolveLink);
    smackSetFileLabel(dst, smackGetFileLabel(src, SmackLabelType::SMACK_LABEL_MMAP, resolveLink),
                                                  SmackLabelType::SMACK_LABEL_MMAP, resolveLink);
    smackSetFileLabel(dst, smackGetFileLabel(src, SmackLabelType::SMACK_LABEL_TRANSMUTE, resolveLink),
                                                  SmackLabelType::SMACK_LABEL_TRANSMUTE, resolveLink);
}

} // namespace lxcpp
