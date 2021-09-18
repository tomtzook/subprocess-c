#pragma once

#include <cerrno>
#include <exception>

namespace subprocess {

class SubprocessException : public std::exception {
};

class ErrnoException : public SubprocessException {
public:
    explicit ErrnoException(int errorCode)
            : m_errorCode(errorCode) {}

    int errorCode() const {
        return m_errorCode;
    }

    virtual const char* what() const noexcept override {
        return "Errno exception";
    }

private:
    int m_errorCode;
};

class WaitNoChildException : public ErrnoException {
public:
    WaitNoChildException()
            : ErrnoException(ECHILD) {}

    virtual const char* what() const noexcept override {
        return "No child detected while waiting";
    }
};

class FdClosedException : public SubprocessException {
public:
    virtual const char* what() const noexcept override {
        return "FD closed";
    }
};

}
