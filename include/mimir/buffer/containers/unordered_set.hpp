
#ifndef MIMIR_BUFFER_CONTAINERS_UNORDERED_SET_HPP_
#define MIMIR_BUFFER_CONTAINERS_UNORDERED_SET_HPP_

#include "../char_stream_segmented.hpp"
#include "../builder_base.hpp"
#include "../view_base.hpp"

#include <unordered_set>


namespace mimir {

template<typename T>
class UnorderedSet : public UncopyableMixin<UnorderedSet<T>> {
private:
    // Persistent storage
    CharStreamSegmented<100000> m_storage;

    // Data to be accessed
    std::unordered_set<View<T>> m_data;

public:
    [[nodiscard]] auto insert(const Builder<T>& builder) {
        // TODO (Dominik): implement
    }
};





}


#endif
