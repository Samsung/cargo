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
 * @brief   File utility functions
 */

#include "config.hpp"

#include "logger/logger.hpp"
#include "utils/fd-utils.hpp"
#include "utils/fs.hpp"
#include "utils/paths.hpp"
#include "utils/smack.hpp"
#include "utils/text.hpp"
#include "utils/exception.hpp"

#include <boost/filesystem.hpp>
#include <dirent.h>
#include <fstream>
#include <streambuf>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/syscall.h>

#include <iostream>

namespace fs = boost::filesystem;

namespace utils {

namespace {

class ScopedDirStruct final {
public:
    ScopedDirStruct(DIR *d) : dir(d) { }
    ~ScopedDirStruct() { closedir(dir); }
private:
    DIR *dir;
};

}

// ------------------- syscalls -------------------
bool remove(const std::string& path)
{
    if (::remove(path.c_str()) == -1) {
        if (errno == ENOENT) {
            LOGW(path << ": not removed, path does not exist");
            return false;
        }
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": failed to remove", errno);
    }

    return true;
}

struct ::stat stat(const std::string & path)
{
    struct ::stat s;
    if (::stat(path.c_str(), &s) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": failed to get stat", errno);
    }
    return s;
}

struct ::statfs statfs(const std::string & path)
{
    int rc;
    struct ::statfs s;
    do {
        rc = ::statfs(path.c_str(), &s);
    } while (rc == -1 && errno == EINTR);

    if (rc == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": failed to get statfs", errno);
    }
    return s;
}

bool access(const std::string& path, int mode)
{
    if (::access(path.c_str(), mode) == -1) {
        if (errno == EACCES) {
            return false;
        }
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": trying to access() failed", errno);
    }
    return true;
}

void mount(const std::string& source,
           const std::string& target,
           const std::string& filesystemtype,
           unsigned long mountflags,
           const std::string& data)
{
    int ret = ::mount(source.c_str(),
                      target.c_str(),
                      filesystemtype.c_str(),
                      mountflags,
                      data.c_str());
    if (ret == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("Mount failed: source='"
             << source
             << "' target='"
             << target
             << "' filesystemtype='"
             << filesystemtype
             << "' mountflags="
             << std::to_string(mountflags)
             << " data='"
             << data
             << "'", errno);
    }
    LOGD("mounted " << source << " on " << target << " " << filesystemtype << " (" << mountflags << ")");
}

void umount(const std::string& path, int flags)
{
    if (::umount2(path.c_str(), flags) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": umount failed", errno);
    }
}

void mkfifo(const std::string& path, mode_t mode)
{
    if (::mkfifo(path.c_str(), mode) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": failed to create fifo", errno);
    }
}

void chown(const std::string& path, uid_t uid, gid_t gid)
{
    // set owner
    if (::chown(path.c_str(), uid, gid) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": chown() failed", errno);
    }
}

void lchown(const std::string& path, uid_t uid, gid_t gid)
{
    // set owner of a symlink
    if (::lchown(path.c_str(), uid, gid) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": lchown() failed", errno);
    }
}

void chmod(const std::string& path, mode_t mode)
{
    if (::chmod(path.c_str(), mode) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": chmod() failed", errno);
    }
}

void link(const std::string& src, const std::string& dst)
{
    if (::link(src.c_str(), dst.c_str()) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("path=host:" << src << ": failed to hard link to path=host:" << dst, errno);
    }
}

void symlink(const std::string& target, const std::string& linkpath)
{
    if (::symlink(target.c_str(), linkpath.c_str()) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(target << ": symlink(" << linkpath << ") failed", errno);
    }
}

void fchdir(int fd)
{
    if (::fchdir(fd) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E("fd:" << std::to_string(fd) << ": fchdir() failed", errno);
    }
}

void chdir(const std::string& path)
{
    if (::chdir(path.c_str()) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": chdir() failed", errno);
    }
}

bool mkdir(const std::string& path, mode_t mode)
{
    if (::mkdir(path.c_str(), mode) == -1) {
        if (errno == EEXIST) {
            return false;
        }
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": mkdir() failed", errno);
    }
    return true;
}

bool rmdir(const std::string& path)
{
    if (::rmdir(path.c_str()) == -1) {
        if (errno == ENOENT) {
            LOGW(path << ": not removed, directory does not exist");
            return false;
        }
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": failed to rmdir", errno);
    }

    return true;
}

void mknod(const std::string& path, mode_t mode, dev_t dev)
{
    if (::mknod(path.c_str(), mode, dev) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": mknod() failed", errno);
    }
}

void pivot_root(const std::string& new_root, const std::string& put_old)
{
    if (::syscall(SYS_pivot_root, new_root.c_str(), put_old.c_str()) == -1) {
        THROW_UTILS_EXCEPTION_ERRNO_E(new_root << ": pivot_root() failed", errno);
    }
}


// ------------------- higher level functions -------------------
std::string readFileStream(const std::string& path)
{
    std::ifstream file(path);

    if (!file) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": read failed", errno);
    }
    // 2 x faster then std::istreambuf_iterator
    std::stringstream content;
    content << file.rdbuf();
    return content.str();
}

std::string readFileContent(const std::string& path)
{
    std::ifstream file(path);

    if (!file) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": could not open for reading", errno);
    }

    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    if (length < 0) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": tellg failed", errno);
    }
    std::string result;
    result.resize(static_cast<size_t>(length));
    file.seekg(0, std::ios::beg);

    file.read(&result[0], length);
    if (!file) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": read error", errno);
    }
    return result;
}

void saveFileContent(const std::string& path, const std::string& content)
{
    std::ofstream file(path);
    if (!file) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": could not open for writing", errno);
    }
    file.write(content.data(), static_cast<std::streamsize>(content.size()));
    if (!file) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": could not write to", errno);
    }
}

void readFirstLineOfFile(const std::string& path, std::string& ret)
{
    std::ifstream file(path);
    if (!file) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": could not open for reading", errno);
    }

    std::getline(file, ret);
    if (!file) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path << ": read error", errno);
    }
}

bool removeDir(const std::string& path)
{
    //1. try rmdir, in case it is already empty or does not exist
    try {
        return utils::rmdir(path);
    } catch (const UtilsException& e) {
        if (e.mErrno != ENOTEMPTY && e.mErrno != EBUSY) {
            throw;
        }
    }

    //2. not empty, do recursion
    struct dirent storeEntry;
    struct dirent *entry;
    DIR *dir = ::opendir(path.c_str());

    if (dir == NULL) {
        if (errno == ENOENT) {
            LOGD(path << ": was removed by other process.");
            return false;
        }
        THROW_UTILS_EXCEPTION_E(path);
    }

    { //bracket to call scope destructor
        ScopedDirStruct scopedir(dir);
        while (::readdir_r(dir, &storeEntry, &entry) == 0 && entry != NULL) {
            if (::strcmp(entry->d_name, ".") == 0 || ::strcmp(entry->d_name, "..") == 0) {
                 continue;
            }

            std::string newpath = path + "/" + entry->d_name;
            if (entry->d_type == DT_DIR) {
                removeDir(newpath);
            } else {
                try {
                    utils::remove(newpath.c_str());
                } catch (const UtilsException&) {
                    // Ignore any errors on file deletion
                    // Error from rmdir on parent directory (in next step) will be returned anyway
                    // Note: rmdir can be successful even if directory is not empty (like for cgroup filesystem)
                    //       but first all child directories must be removed
                }
            }
        }
    }

    utils::rmdir(path);
    LOGD(path << ": successfuly removed.");
    return true;
}

void assertExists(const std::string& path, int inodeType)
{
    if (!utils::exists(path, inodeType)) {
        THROW_UTILS_EXCEPTION_E(path + ": not exists");
    }
}

bool exists(const std::string& path, int inodeType)
{
    if (path.empty()) {
        THROW_UTILS_EXCEPTION_E("Empty path");
    }

    struct ::stat s;
    try {
        s = utils::stat(path);
    } catch (const UtilsException& e) {
        if (e.mErrno == ENOENT) {
            return false;
        }
        throw;
    }

    if (inodeType != 0) {
        if (!(s.st_mode & inodeType)) {
            LOGE(path << ": wrong inodeType, expected: " << std::to_string(inodeType) <<
                                  ", actual: " << std::to_string(s.st_mode));
            return false;
        }

        if (inodeType == S_IFDIR && !utils::access(path, X_OK)) {
            THROW_UTILS_EXCEPTION_ERRNO_E(path << ": not a traversable directory", errno);
        }
    }
    return true;
}

bool isCharDevice(const std::string& path)
{
    return utils::exists(path, S_IFCHR);
}

bool isRegularFile(const std::string& path)
{
    return utils::exists(path, S_IFREG);
}

void assertIsRegularFile(const std::string& path)
{
    assertExists(path, S_IFREG);
}

bool isDir(const std::string& path)
{
    return utils::exists(path, S_IFDIR);
}

void assertIsDir(const std::string& path)
{
    assertExists(path, S_IFDIR);
}

bool isAbsolute(const std::string& path)
{
    return fs::path(path).is_absolute();
}

void assertIsAbsolute(const std::string& path)
{
    if (!isAbsolute(path)) {
        THROW_UTILS_EXCEPTION_D(path << ": must be absolute!");
    }
}


namespace {
// NOTE: Should be the same as in systemd/src/core/mount-setup.c
const std::string RUN_MOUNT_POINT_OPTIONS = "mode=755,smackfstransmute=System::Run";
const std::string RUN_MOUNT_POINT_OPTIONS_NO_SMACK = "mode=755";
const unsigned long RUN_MOUNT_POINT_FLAGS = MS_NOSUID | MS_NODEV | MS_STRICTATIME;

int mountTmpfs(const std::string& path, unsigned long flags, const std::string& options)
{
    try {
        utils::mount("tmpfs", path.c_str(), "tmpfs", flags, options.c_str());
        return 0;
    } catch(const UtilsException & e) {
        return e.mErrno;
    }
}

} // namespace

void mountRun(const std::string& path)
{
    if (utils::mountTmpfs(path, RUN_MOUNT_POINT_FLAGS, RUN_MOUNT_POINT_OPTIONS)) {
        int errno_ = utils::mountTmpfs(path, RUN_MOUNT_POINT_FLAGS, RUN_MOUNT_POINT_OPTIONS_NO_SMACK);
        if (errno_) {
            THROW_UTILS_EXCEPTION_ERRNO_E(path << ": mount failed", errno_);
        }
    }
}

bool isMountPoint(const std::string& path)
{
    std::string parentPath = dirName(path);
    return ! hasSameMountPoint(path, parentPath);
}


bool isMountPointShared(const std::string& path)
{
    std::ifstream fileStream("/proc/self/mountinfo");
    if (!fileStream.good()) {
        THROW_UTILS_EXCEPTION_E(path << ": open failed");
    }

    // Find the line corresponding to the path
    std::string line;
    while (std::getline(fileStream, line).good()) {
        std::istringstream iss(line);
        auto it = std::istream_iterator<std::string>(iss);
        std::advance(it, 4);

        if (it->compare(path)) {
            // Wrong line, different path
            continue;
        }

        // Right line, check if mount point shared
        std::advance(it, 2);
        return it->find("shared:") != std::string::npos;
    }

    // Path not found
    return false;
}

bool hasSameMountPoint(const std::string& path1, const std::string& path2)
{
    return utils::stat(path1).st_dev == utils::stat(path2).st_dev;
}

void moveFile(const std::string& src, const std::string& dst)
{
    // The destination has to be a full path (including a file name)
    // so it doesn't exist yet, we need to check upper level dir instead.
    bool bResult = hasSameMountPoint(src, dirName(dst));

    if (bResult) {
        boost::system::error_code error;
        fs::rename(src, dst, error);
        if (error) {
            THROW_UTILS_EXCEPTION_E(src << ": failed to rename to: " << dst << ", error: " << error.message());
        }
    } else {
        copyFile(src, dst);
        utils::remove(src);
    }
}

namespace {

bool copyDirContentsRec(const boost::filesystem::path& src, const boost::filesystem::path& dst)
{
    try {
        for (fs::directory_iterator file(src);
                file != fs::directory_iterator();
                ++file) {
            fs::path current(file->path());
            fs::path destination = dst / current.filename();

            boost::system::error_code ec;

            if (!fs::is_symlink(current) && fs::is_directory(current)) {
                fs::create_directory(destination, ec);
            } else {
                fs::copy(current, destination, ec);
            }

            if (ec.value() != boost::system::errc::success) {
                LOGW("Failed to copy " << current << ": " << ec.message());
                continue;
            }

            if (!fs::is_symlink(current) && fs::is_directory(current)) {
                if (!copyDirContentsRec(current, destination)) {
                    return false;
                }

                // apply permissions coming from source file/directory
                fs::file_status stat = status(current);
                fs::permissions(destination, stat.permissions(), ec);

                if (ec.value() != boost::system::errc::success) {
                    LOGW("Failed to set permissions for " << destination << ": " << ec.message());
                }
            }

            // change owner
            struct stat info = utils::stat(current.string());
            if (fs::is_symlink(destination)) {
                utils::lchown(destination.string(), info.st_uid, info.st_gid);
            } else {
                utils::chown(destination.string(), info.st_uid, info.st_gid);
            }
        }
        return true;
    } catch (const UtilsException& e) {
        LOGW(e.what());
    } catch (const fs::filesystem_error& e) {
        LOGW(e.what());
    }

    return false;
}

} // namespace

void copyDirContents(const std::string& src, const std::string& dst)
{
    if (!copyDirContentsRec(fs::path(src), fs::path(dst))) {
        THROW_UTILS_EXCEPTION_E(src << ": failed to copy contents to new location: " << dst);
    }
}

void createDir(const std::string& path, uid_t uid, uid_t gid, boost::filesystem::perms mode)
{
    fs::path dirPath(path);
    boost::system::error_code errorCode;
    bool runDirCreated = false;
    if (!fs::exists(dirPath)) {
        if (!fs::create_directory(dirPath, errorCode)) {
            THROW_UTILS_EXCEPTION_E(path << ": failed to create directory, error: " << errorCode.message());
        }
        runDirCreated = true;
    } else if (!fs::is_directory(dirPath)) {
        THROW_UTILS_EXCEPTION_E(path << ": cannot create directory, already exists!");
    }

    // set permissions
    fs::permissions(dirPath, mode, errorCode);
    if (fs::status(dirPath).permissions() != mode) {
        THROW_UTILS_EXCEPTION_E(path << ": failed to set permissions, error: " << errorCode.message());
    }

    // set owner
    try {
        utils::chown(path, uid, gid);
    } catch(...) {
        if (runDirCreated) {
            fs::remove(dirPath);
        }
        throw;
    }
}

void createDirs(const std::string& path, mode_t mode)
{
    std::vector<std::string> created;

    try {
        std::string prefix = utils::beginsWith(path, "/") ? "" : ".";
        std::vector<std::string> segments = utils::split(path, "/");

        for (const auto& seg : segments) {
            if (seg.empty()) {
                continue;
            }

            prefix += "/" + seg;
            if (utils::mkdir(prefix, mode)) {
                created.push_back(prefix);
                LOGI("dir created: " << prefix);
            }
        }
    } catch (...) {
        try {
            for (auto iter = created.rbegin(); iter != created.rend(); ++iter) {
                utils::rmdir(*iter);
            }
        } catch(...) {
            LOGE("Failed to undo created dirs after an error");
        }
    }
}

void chownDir(const std::string& path, uid_t uid, uid_t gid)
{
    struct dirent storeEntry;
    struct dirent *entry;
    DIR *dir = ::opendir(path.c_str());

    if (dir == NULL) {
        THROW_UTILS_EXCEPTION_ERRNO_E(path, errno);
    }

    { //bracket to call scope destructor
        ScopedDirStruct scopedir(dir);
        while (::readdir_r(dir, &storeEntry, &entry) == 0 && entry != NULL) {
            if (::strcmp(entry->d_name, ".") == 0 || ::strcmp(entry->d_name, "..") == 0) {
                 continue;
            }

            std::string newpath = path + "/" + entry->d_name;
            if (entry->d_type == DT_DIR) {
                chownDir(newpath, uid, gid);
            } else {
                utils::lchown(newpath, uid, gid);
            }
        }
    }

    utils::lchown(path, uid, gid);
    LOGI(path << ": successfuly chown.");
}

void createEmptyDir(const std::string& path)
{
    fs::path dirPath(path);
    boost::system::error_code ec;
    bool cleanDirCreated = false;

    if (!fs::exists(dirPath)) {
        if (!fs::create_directory(dirPath, ec)) {
            THROW_UTILS_EXCEPTION_E(dirPath.string() << ": failed to create dir, error: " << ec.message());
        }
        cleanDirCreated = true;
    } else if (!fs::is_directory(dirPath)) {
        THROW_UTILS_EXCEPTION_E(dirPath.string() << ": already exists and is not a dir, cannot create.");
    }

    if (!cleanDirCreated) {
        // check if directory is empty if it was already created
        if (!fs::is_empty(dirPath)) {
            THROW_UTILS_EXCEPTION_E(dirPath.string() << ": directory has some data inside, cannot be used.");
        }
    }
}

void createFile(const std::string& path, int flags, mode_t mode)
{
    utils::close(utils::open(path, O_CREAT | flags, mode));
}

void createFifo(const std::string& path, mode_t mode)
{
    utils::mkfifo(path.c_str(), mode);
}

void copyFile(const std::string& src, const std::string& dest)
{
    boost::system::error_code errorCode;
    fs::copy_file(src, dest, errorCode);
    if (errorCode) {
        THROW_UTILS_EXCEPTION_E("path=host:" << src << ": failed to copy file to path=host:"
                                << dest << ", error: " << errorCode.message());
    }
    try {
        copySmackLabel(src, dest);
    } catch(const UtilsException & e) {
        std::string msg = "Failed to copy file: msg: (can't copy smacklabel), path=host:"
             + src
             + ", path=host:"
             + dest + ".";
        fs::remove(src, errorCode);
        if (errorCode) {
            msg += "\nFailed to clean after copy failure: path=host:"
                 + src
                 + ", msg: "
                 + errorCode.message();
        }
        THROW_UTILS_EXCEPTION_ERRNO_E(msg, e.mErrno);
    }
}

void createLink(const std::string& src, const std::string& dest)
{
    utils::link(src, dest);
    try {
        copySmackLabel(src, dest, false);
    } catch(const UtilsException & e) {
        std::string msg = "Failed to copy smack label: path=host:"
             + src
             + ", path=host:"
             + dest + ".";
        boost::system::error_code ec;
        fs::remove(dest, ec);
        if (!ec) {
            msg += "\nFailed to clean after hard link creation failure: path=host:"
                 + src
                 + ", to: "
                 + dest
                 + ", msg: "
                 + ec.message();
        }
        THROW_UTILS_EXCEPTION_ERRNO_E(msg, e.mErrno);
    }
}


} // namespace utils
