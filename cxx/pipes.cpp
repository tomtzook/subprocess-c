
#include <unistd.h>
#include <errno.h>

#include "pipes.h"


static constexpr int FD_CLEAR = -1;
static constexpr int PIPE_READ = 0;
static constexpr int PIPE_WRITE = 1;

namespace subprocess {

Fd::Fd()
    : m_fd(FD_CLEAR)
{ }

Fd::Fd(int fd)
    : m_fd(fd)
{}

Fd::Fd(Fd&& other) noexcept
    : m_fd(other.m_fd) {
    other.m_fd = FD_CLEAR;
}

Fd::~Fd() {
    reset();
}

int Fd::fd() const {
    return m_fd;
}

bool Fd::isEmpty() const {
    return m_fd < 0;
}

void Fd::dup2(int targetFd) const {
    if (isEmpty()) {
        throw FdClosedException();
    }

    ::dup2(m_fd, targetFd);
}

size_t Fd::read(char* buffer, size_t count) const {
    if (isEmpty()) {
        throw FdClosedException();
    }

    ssize_t result = ::read(m_fd, buffer, count);
    if (result < 0) {
        throw ErrnoException(errno);
    }

    return static_cast<size_t>(result);
}

void Fd::reset() {
    if (!isEmpty()) {
        close(m_fd);
        m_fd = FD_CLEAR;
    }
}

void Fd::reset(int fd) {
    reset();
    m_fd = fd;
}

Pipe::Pipe()
{}

Pipe::Pipe(Pipe&& other)
    : m_read(std::move(other.m_read))
    , m_write(std::move(other.m_write))
{}

const Fd& Pipe::readFd() const {
    return m_read;
}

Fd& Pipe::readFd() {
    return m_read;
}

const Fd& Pipe::writeFd() const {
    return m_write;
}

Fd& Pipe::writeFd() {
    return m_write;
}

void Pipe::open() {
    int fds[2] = {0};
    if (-1 == pipe(fds)) {
        throw ErrnoException(errno);
    }

    m_read.reset(fds[PIPE_READ]);
    m_write.reset(fds[PIPE_WRITE]);
}

void Pipe::close() {
    m_read.reset();
    m_write.reset();
}

}
