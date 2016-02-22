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
 * @brief   File descriptor utility functions
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "config.hpp"

#include "utils/fd-utils.hpp"
#include "utils/exception.hpp"
#include "logger/logger.hpp"

#include <cerrno>
#include <cstring>
#include <chrono>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
namespace chr = std::chrono;

namespace utils {

// TODO: Add here various fixes from cargo::FDStore

namespace {

void waitForEvent(int fd,
                  short event,
                  const chr::high_resolution_clock::time_point deadline)
{
    // Wait for the rest of the data
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = event;

    for (;;) {
        chr::milliseconds timeoutMS = chr::duration_cast<chr::milliseconds>(deadline - chr::high_resolution_clock::now());
        if (timeoutMS.count() < 0) {
            THROW_EXCEPTION(UtilsException, "Timeout while waiting for event: " << std::hex << event <<
                 " on fd: " << std::dec << fd);
        }

        int ret = ::poll(fds, 1 /*fds size*/, timeoutMS.count());

        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            THROW_EXCEPTION(UtilsException, "Error in poll", errno);
        }

        if (ret == 0) {
            THROW_EXCEPTION(UtilsException, "Timeout in read");
        }

        if (fds[0].revents & event) {
            // Here Comes the Sun
            break;
        }

        if (fds[0].revents & POLLHUP) {
            THROW_EXCEPTION(UtilsException, "Peer disconnected");
        }
    }
}

void setFDFlag(const int fd, const int getOp, const int setOp, const int flag, const bool set)
{
    int ret = ::fcntl(fd, getOp);
    if (ret == -1) {
        THROW_EXCEPTION(UtilsException, "fcntl(): Failed to get FD flags", errno);
    }

    if (set) {
        ret = ::fcntl(fd, setOp, ret | flag);
    } else {
        ret = ::fcntl(fd, setOp, ret & ~flag);
    }

    if (ret == -1) {
        THROW_EXCEPTION(UtilsException, "fcntl(): Failed to set FD flag", errno);
    }
}

} // namespace


int open(const std::string &path, int flags, mode_t mode)
{
    assert(!((flags & O_CREAT) == O_CREAT || (flags & O_TMPFILE) == O_TMPFILE) || mode != static_cast<unsigned>(-1));

    int fd;
    for (;;) {
        fd = ::open(path.c_str(), flags, mode);

        if (-1 == fd) {
            if (errno == EINTR) {
                LOGT("open() interrupted by a signal, retrying");
                continue;
            }
            THROW_EXCEPTION(UtilsException, path + ": open() failed", errno);
        }
        break;
    }

    return fd;
}

void close(int fd) noexcept
{
    if (fd < 0) {
        return;
    }

    for (;;) {
        if (-1 == ::close(fd)) {
            if (errno == EINTR) {
                LOGT("close() interrupted by a signal, retrying");
                continue;
            }
            LOGE("Error in close: " << getSystemErrorMessage());
        }
        break;
    }
}

void shutdown(int fd)
{
    if (fd < 0) {
        return;
    }

    if (-1 == ::shutdown(fd, SHUT_RDWR)) {
        THROW_EXCEPTION(UtilsException, "shutdown() failed", errno);
    }
}

int ioctl(int fd, unsigned long request, void *argp)
{
    int ret = ::ioctl(fd, request, argp);
    if (ret == -1) {
        THROW_EXCEPTION(UtilsException, "ioctl() failed", errno);
    }
    return ret;
}

int dup2(int oldFD, int newFD, bool closeOnExec)
{
    int flags = 0;
    if (closeOnExec) {
        flags |= O_CLOEXEC;
    }
    int fd = dup3(oldFD, newFD, flags);
    if (fd == -1) {
        THROW_EXCEPTION(UtilsException, "dup3() failed", errno);
    }
    return fd;
}

void write(int fd, const void* bufferPtr, const size_t size, int timeoutMS)
{
    chr::high_resolution_clock::time_point deadline = chr::high_resolution_clock::now() +
            chr::milliseconds(timeoutMS);

    size_t nTotal = 0;
    for (;;) {
        int n  = ::write(fd,
                         reinterpret_cast<const char*>(bufferPtr) + nTotal,
                         size - nTotal);
        if (n >= 0) {
            nTotal += n;
            if (nTotal == size) {
                // All data is written, break loop
                break;
            }
        } else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            // Neglected errors
            LOGD("Retrying write");
        } else {
            THROW_EXCEPTION(UtilsException, "Error during write()", errno);
        }

        waitForEvent(fd, POLLOUT, deadline);
    }
}

void read(int fd, void* bufferPtr, const size_t size, int timeoutMS)
{
    chr::high_resolution_clock::time_point deadline = chr::high_resolution_clock::now() +
            chr::milliseconds(timeoutMS);

    size_t nTotal = 0;
    for (;;) {
        int n  = ::read(fd,
                        reinterpret_cast<char*>(bufferPtr) + nTotal,
                        size - nTotal);
        if (n >= 0) {
            nTotal += n;
            if (nTotal == size) {
                // All data is read, break loop
                break;
            }
            if (n == 0) {
                THROW_EXCEPTION(UtilsException, "Peer disconnected");
            }
        } else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
            // Neglected errors
            LOGD("Retrying read");
        } else {
            THROW_EXCEPTION(UtilsException, "Error during read()", errno);
        }

        waitForEvent(fd, POLLIN, deadline);
    }
}

unsigned int getMaxFDNumber()
{
    struct rlimit rlim;
    if (-1 ==  ::getrlimit(RLIMIT_NOFILE, &rlim)) {
        THROW_EXCEPTION(UtilsException, "Error during getrlimit()", errno);
    }
    return rlim.rlim_cur;
}

void setMaxFDNumber(unsigned int limit)
{
    struct rlimit rlim;
    rlim.rlim_cur = limit;
    rlim.rlim_max = limit;
    if (-1 ==  ::setrlimit(RLIMIT_NOFILE, &rlim)) {
        THROW_EXCEPTION(UtilsException, "Error during setrlimit()", errno);
    }
}

unsigned int getFDNumber()
{
    const std::string path = "/proc/self/fd/";
    return std::distance(fs::directory_iterator(path),
                         fs::directory_iterator());
}

int fdRecv(int socket, const unsigned int timeoutMS)
{
    std::chrono::high_resolution_clock::time_point deadline =
        std::chrono::high_resolution_clock::now() +
        std::chrono::milliseconds(timeoutMS);

    // Space for the file descriptor
    union {
        struct cmsghdr cmh;
        char   control[CMSG_SPACE(sizeof(int))];
    } controlUnion;

    // Describe the data that we want to recive
    controlUnion.cmh.cmsg_len = CMSG_LEN(sizeof(int));
    controlUnion.cmh.cmsg_level = SOL_SOCKET;
    controlUnion.cmh.cmsg_type = SCM_RIGHTS;

    // Setup the input buffer
    // Ensure at least 1 byte is transmited via the socket
    char buf;
    struct iovec iov;
    iov.iov_base = &buf;
    iov.iov_len = sizeof(char);

    // Set the ancillary data buffer
    // The socket has to be connected, so we don't need to specify the name
    struct msghdr msgh;
    ::memset(&msgh, 0, sizeof(msgh));

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;

    msgh.msg_control = controlUnion.control;
    msgh.msg_controllen = sizeof(controlUnion.control);

    // Receive
    for(;;) {
        ssize_t ret = ::recvmsg(socket, &msgh, MSG_WAITALL);
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                // Neglected errors, retry
            } else {
                THROW_EXCEPTION(UtilsException, "Error during recvmsg()", errno);
            }
        } else if (ret == 0) {
            THROW_EXCEPTION(UtilsException, "Peer disconnected");
        } else {
            // We receive only 1 byte of data. No need to repeat
            break;
        }

        waitForEvent(socket, POLLIN, deadline);
    }

    struct cmsghdr *cmhp;
    cmhp = CMSG_FIRSTHDR(&msgh);
    if (cmhp == NULL || cmhp->cmsg_len != CMSG_LEN(sizeof(int))) {
        THROW_EXCEPTION(UtilsException, "Bad cmsg length");
    } else if (cmhp->cmsg_level != SOL_SOCKET) {
        THROW_EXCEPTION(UtilsException, "cmsg_level != SOL_SOCKET");
    } else if (cmhp->cmsg_type != SCM_RIGHTS) {
        THROW_EXCEPTION(UtilsException, "cmsg_type != SCM_RIGHTS");
    }

    return *(reinterpret_cast<int*>(CMSG_DATA(cmhp)));
}

bool fdSend(int socket, int fd, const unsigned int timeoutMS)
{
    std::chrono::high_resolution_clock::time_point deadline =
        std::chrono::high_resolution_clock::now() +
        std::chrono::milliseconds(timeoutMS);

    // Space for the file descriptor
    union {
        struct cmsghdr cmh;
        char   control[CMSG_SPACE(sizeof(int))];
    } controlUnion;

    // Ensure at least 1 byte is transmited via the socket
    struct iovec iov;
    char buf = '!';
    iov.iov_base = &buf;
    iov.iov_len = sizeof(char);

    // Fill the message to send:
    // The socket has to be connected, so we don't need to specify the name
    struct msghdr msgh;
    ::memset(&msgh, 0, sizeof(msgh));

    // Only iovec to transmit one element
    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;

    // Ancillary data buffer
    msgh.msg_control = controlUnion.control;
    msgh.msg_controllen = sizeof(controlUnion.control);

    // Describe the data that we want to send
    struct cmsghdr *cmhp;
    cmhp = CMSG_FIRSTHDR(&msgh);
    cmhp->cmsg_len = CMSG_LEN(sizeof(int));
    cmhp->cmsg_level = SOL_SOCKET;
    cmhp->cmsg_type = SCM_RIGHTS;
    *(reinterpret_cast<int*>(CMSG_DATA(cmhp))) = fd;

    // Send
    for(;;) {
        ssize_t ret = ::sendmsg(socket, &msgh, MSG_NOSIGNAL);
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                // Neglected errors, retry
            } else {
                THROW_EXCEPTION(UtilsException, "Error during sendmsg()", errno);
            }
        } else if (ret == 0) {
            // Retry the sending
        } else {
            // We send only 1 byte of data. No need to repeat
            break;
        }

        waitForEvent(socket, POLLOUT, deadline);
    }

    // TODO: It shouldn't return
    return true;
}

void setCloseOnExec(int fd, bool closeOnExec)
{
    setFDFlag(fd, F_GETFD, F_SETFD, FD_CLOEXEC, closeOnExec);
}

void setNonBlocking(int fd, bool nonBlocking)
{
    setFDFlag(fd, F_GETFL, F_SETFL, O_NONBLOCK, nonBlocking);
}

} // namespace utils
