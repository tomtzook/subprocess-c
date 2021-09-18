
#include <sys/mman.h>
#include <cerrno>

#include "cxx/except.h"
#include "cxx/sharedmem.h"

namespace subprocess {

SharedMemory::SharedMemory()
    : m_ptr(nullptr)
    , m_size(0)
{ }

SharedMemory::SharedMemory(SharedMemory&& other) noexcept
    : m_ptr(other.m_ptr)
    , m_size(other.m_size) {
    other.m_ptr = nullptr;
    other.m_size = 0;
}

SharedMemory::~SharedMemory() {
    if (nullptr != m_ptr) {
        munmap(m_ptr, m_size);
        m_ptr = nullptr;
    }
}

const void* SharedMemory::ptr() const {
    return m_ptr;
}

void* SharedMemory::ptr() {
    return m_ptr;
}

size_t SharedMemory::size() const {
    return m_size;
}

void SharedMemory::open(size_t size) {
    void* mem = mmap(nullptr, size, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (nullptr == mem) {
        throw ErrnoException(errno);
    }

    m_ptr = mem;
    m_size = size;
}

}
