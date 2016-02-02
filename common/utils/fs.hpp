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
 * @brief   File utility functions declaration
 */

#ifndef COMMON_UTILS_FS_HPP
#define COMMON_UTILS_FS_HPP

#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/vfs.h>
#include <vector>
#include <boost/filesystem.hpp>


namespace utils {

// syscall wrappers
// Throw exception on error

// remove file/dir (not recursive)
// returns true if removed, false otherwise
bool remove(const std::string& path);
struct ::stat stat(const std::string& path);
struct ::statfs statfs(const std::string& path);
// true if can be accessed with given mode
bool access(const std::string& path, int mode);
void mount(const std::string& source,
           const std::string& target,
           const std::string& filesystemtype,
           unsigned long mountflags,
           const std::string& data);
void umount(const std::string& path, int flags = 0);
// returns descriptor to the newly created item
void mkfifo(const std::string& path, mode_t mode);
void chown(const std::string& path, uid_t uid, gid_t gid);
void lchown(const std::string& path, uid_t uid, gid_t gid);
void chown(const std::string& path, uid_t owner, gid_t group);
void link(const std::string& src, const std::string& dst);
void symlink(const std::string& target, const std::string& linkpath);
void fchdir(int fd);
void chdir(const std::string& path);
// returns true if new dir created, false if dir was already present
bool mkdir(const std::string& path, mode_t mode);
// returns true if removed, false otherwise
bool rmdir(const std::string& path);
void mknod(const std::string& path, mode_t mode, dev_t dev);
void chmod(const std::string& path, mode_t mode);
void pivot_root(const std::string& new_root, const std::string& put_old);


/**
 * Reads the content of file stream (no seek); Throws exception on error
 */
std::string readFileStream(const std::string& path);

/**
 * Reads the content of a file (performs seek); Throws exception on error
 */
std::string readFileContent(const std::string& path);

/**
 * Save the content to the file
 */
void saveFileContent(const std::string& path, const std::string& content);

/**
 * Read a line from file
 * Its goal is to read a kernel config files (eg. from /proc, /sys/).
 * Throws exception on error
 */
void readFirstLineOfFile(const std::string& path, std::string& ret);

/**
 * Remove directory and its content. Throws exception on error
 * @return:
 *    true if directory sucessfuly removed
 *    false if directory does not exist
 */
bool removeDir(const std::string& path);

/**
 * Checks if a path exists and points to an expected item type.
 * @return: true if exists and is a directory, false otherwise
 */
bool exists(const std::string& path, int inodeType = 0);

/**
 * Checks if a path exists and points to an expected item type.
 */
void assertExists(const std::string& path, int inodeType = 0);

/**
 * Checks if a char device exists
 */
bool isCharDevice(const std::string& path);

/**
 * Checks if a path exists and points to a directory
 * @return: true if exists and is a directory, false otherwise
 */
bool isDir(const std::string& path);

/**
 * Checks if a path exists and points to a directory
 */
void assertIsDir(const std::string& path);

/**
 * Checks if a path exists and points to a regular file or link
 * @return: true if exists and is a regular file, false otherwise
 */
bool isRegularFile(const std::string& path);

/**
 * Checks if a path exists and points to a regular file or link
 */
void assertIsRegularFile(const std::string& path);

/**
 * Checks if path is absolute
 * @return: true if path is valid absolute path, false otherwise
 */
bool isAbsolute(const std::string& path);

/**
 * Checks if path is absolute
 */
void assertIsAbsolute(const std::string& path);

/**
 * Mounts run as a tmpfs on a given path.
 * Throws exception on error
 */
void mountRun(const std::string& path);

/**
 * Check if given path is a mount point.
 * Throws exception on error.
 * @return boolean result
 */
bool isMountPoint(const std::string& path);

/**
 * Detect whether path is mounted as MS_SHARED.
 * Parses /proc/self/mountinfo
 * Throws exception on error.
 *
 * @param path mount point
 * @return is the mount point shared
 */
bool isMountPointShared(const std::string& path);

/**
 * Checks whether the given paths are under the same mount point.
 * Throws exception on error.
 * @return boolean result
 */
bool hasSameMountPoint(const std::string& path1, const std::string& path2);

/**
 * Moves the file either by rename if under the same mount point
 * or by copy&delete if under a different one.
 * The destination has to be a full path including file name.
 * Throws exception on error
 */
void moveFile(const std::string& src, const std::string& dst);

/**
 * Recursively copy contents of src dir to dst dir.
 * Throws exception on error
 */
void copyDirContents(const std::string& src, const std::string& dst);

/**
 * Creates a directory with specific UID, GID and permissions set.
 * Throws exception on error
 */
void createDir(const std::string& path, uid_t uid, uid_t gid, boost::filesystem::perms mode);

/**
 * Creates a path directories with specific permissions set.
 * Throws exception on error
 */
void createDirs(const std::string& path, mode_t mode = 0755);

/**
 * Recursively do lchown on directory
 */
void chownDir(const std::string& path, uid_t uid, uid_t gid);

/**
 * Creates an empty directory, ready to serve as mount point.
 * Succeeds either if path did not exist and was created successfully, or if already existing dir
 * under the same path is empty and is not a mount point.
 * Throws exception on error
 */
void createEmptyDir(const std::string& path);

/**
 * Creates an empty file.
 * Throws exception on error
 */
void createFile(const std::string& path, int flags, mode_t mode);

/**
 * Creates an FIFO special file.
 * Throws exception on error
 */
void createFifo(const std::string& path, mode_t mode);

/**
 * Copy an file.
 * Throws exception on error
 */
void copyFile(const std::string& src, const std::string& dest);

/**
 * Create hard link
 * Throws exception on error.
 */
void createLink(const std::string& src, const std::string& dest);


} // namespace utils


#endif // COMMON_UTILS_FS_HPP
