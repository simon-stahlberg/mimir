#ifndef MIMIR_BUFFER_BYTE_STREAM_UTILS_HPP_
#define MIMIR_BUFFER_BYTE_STREAM_UTILS_HPP_

#include <cstdint>
#include <cstddef>
#include <type_traits>


namespace mimir
{

/**
 * Ensures that we only serialize types that are trivially copieable.
 * Trivially copieable types are int float double ..
 * Untrivially copieable types are std::vector, std::unordered_set, ...
 * For untrivially copieable types we will provide our own logic.
*/
template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;


/**
 * Read values from raw data.
*/

template<TriviallyCopyable T>
T& read_value(uint8_t* data) {
    return *reinterpret_cast<T*>(data);
}

template<TriviallyCopyable T>
const T& read_value(const uint8_t* data) {
    return *reinterpret_cast<const T*>(data);
}

template<TriviallyCopyable T>
T* read_pointer(uint8_t* data) {
    return reinterpret_cast<T*>(*reinterpret_cast<uintptr_t*>(data));
}

template<TriviallyCopyable T>
const T* read_pointer(const uint8_t* data) {
    return reinterpret_cast<const T*>(*reinterpret_cast<const uintptr_t*>(data));
}


/**
 * Convert between raw data and pointers
*/

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


/**
 * Ensure correct memory alignment for efficient processing
*/
template<typename T>
bool is_correctly_aligned(T num_bytes) {
    return (num_bytes % 8 == 0);
}

}

#endif
