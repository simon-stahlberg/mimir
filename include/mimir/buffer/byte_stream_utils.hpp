#ifndef MIMIR_BUFFER_BYTE_STREAM_UTILS_HPP_
#define MIMIR_BUFFER_BYTE_STREAM_UTILS_HPP_

#include <cstdint>
#include <cstddef>


namespace mimir
{

template<typename T>
T& read_value(char* data) {
    return *reinterpret_cast<T*>(data);
}

template<typename T>
const T& read_value(const char* data) {
    return *reinterpret_cast<const T*>(data);
}

template<typename T>
T* read_pointer(char* data) {
    return reinterpret_cast<T*>(*reinterpret_cast<uintptr_t*>(data));
}

template<typename T>
const T* read_pointer(const char* data) {
    return reinterpret_cast<const T*>(*reinterpret_cast<const uintptr_t*>(data));
}

template<typename T>
uint64_t pointer_to_uint64_t(const T* ptr) {
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    return static_cast<uint64_t>(address);
}

template<typename T>
T* uint64_t_to_pointer(uint64_t address) {
    uintptr_t ptrAddress = static_cast<uintptr_t>(address);
    return reinterpret_cast<T*>(ptrAddress);
}

}

#endif
