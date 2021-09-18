#pragma once

#include <cstddef>

namespace subprocess {

class SharedMemory {
public:
    SharedMemory();
    SharedMemory(SharedMemory&& other) noexcept;
    ~SharedMemory();

    const void* ptr() const;
    void* ptr();

    size_t size() const;

    void open(size_t size);

private:
    void* m_ptr;
    size_t m_size;
};

}
