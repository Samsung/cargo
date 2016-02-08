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
 * @brief   SMACK utils headers
 */

#ifndef COMMON_UTILS_SMACK_HPP
#define COMMON_UTILS_SMACK_HPP

#include <string>

#define SMACK_MOUNT_PATH "/sys/fs/smackfs"
#define SMACK_LABEL_MAX_LEN 255

#ifndef SMACK_MAGIC
#define SMACK_MAGIC 0x43415d53 // "SMAC"
#endif

namespace utils {

// ------------------- syscall wrappers -------------------
// Throw exception on error
std::string getxattr(const std::string &path, const std::string &name);
std::string lgetxattr(const std::string &path, const std::string &name);
void removexattr(const std::string &path, const std::string &name);
void lremovexattr(const std::string &path, const std::string &name);
void setxattr(const std::string &path, const std::string &name,
              const std::string &value, int flags);
void lsetxattr(const std::string &path, const std::string &name,
               const std::string &value, int flags);

// ------------------- higher level functions -------------------
enum class SmackLabelType : int {
    SMACK_LABEL_ACCESS = 0,
    SMACK_LABEL_EXEC,
    SMACK_LABEL_MMAP,
    SMACK_LABEL_TRANSMUTE,
    SMACK_LABEL_IPIN,
    SMACK_LABEL_IPOUT,

    // ---
    SMACK_LABEL_TERMINATOR
};

bool isSmackActive();
bool isSmackNamespaceActive();
std::string smackXattrName(SmackLabelType type);
std::string smackGetSelfLabel();
std::string smackGetFileLabel(const std::string &path,
                              SmackLabelType labelType,
                              bool followLinks);
void smackSetFileLabel(const std::string &path,
                       const std::string &label,
                       SmackLabelType labelType,
                       bool followLinks);
void copySmackLabel(const std::string &src, const std::string &dst, bool resolveLink = true);

} // namespace utils

#endif // COMMON_UTILS_SMACK_HPP
