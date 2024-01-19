
#ifndef MIMIR_BUFFER_CONTAINERS_UNORDERED_SET_HPP_
#define MIMIR_BUFFER_CONTAINERS_UNORDERED_SET_HPP_

#include "../char_stream_segmented.hpp"
#include "../builder_base.hpp"
#include "../view_base.hpp"

#include <unordered_set>


namespace mimir::buffer {

/// @brief Top-level CRTP based interface for a StateRepository.
/// @tparam Derived
template<typename T>
class UnorderedSet : public UncopyableMixin<UnorderedSet<T>> {
private:
    // Persistent storage
    buffer::CharStreamSegmented<100000> m_data;

    // Creates states uniquely
    std::unordered_set<View<T>> m_uniqueness;

    // Reuse memory to create states.
    Builder<T> m_builder;

public:
    [[nodiscard]] auto insert(const Builder<T>& builder) {
        // TODO (Dominik): implement
    }
};





}


#endif
