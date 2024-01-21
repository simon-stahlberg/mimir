#ifndef MIMIR_BUFFER_BYTE_STREAM_UTILS_HPP_
#define MIMIR_BUFFER_BYTE_STREAM_UTILS_HPP_

#include "../common/mixins.hpp"

#include <cstddef>
#include <vector>
#include <iostream>


namespace mimir {

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
    return reinterpret_cast<const T*>(*reinterpret_cast<uintptr_t*>(data));
}

}  // namespace mimir

#endif // MIMIR_BUFFER_BYTE_STREAM_UTILS_HPP_
