#pragma once

#include "except.h"

namespace subprocess {

class Fd {
public:
    Fd();
    explicit Fd(int fd);
    Fd(Fd&& other) noexcept;
    ~Fd();

    int fd() const;
    bool isEmpty() const;

    void dup2(int targetFd) const;

    size_t read(char* buffer, size_t count) const;

    void reset();
    void reset(int fd);

private:
    int m_fd;
};

class Pipe {
public:
    Pipe();
    Pipe(Pipe&& other);

    const Fd& readFd() const;
    Fd& readFd();
    const Fd& writeFd() const;
    Fd& writeFd();

    void open();
    void close();

private:
    Fd m_read;
    Fd m_write;
};

}
