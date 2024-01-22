
#ifndef MIMIR_BUFFER_CONTAINERS_UNORDERED_SET_HPP_
#define MIMIR_BUFFER_CONTAINERS_UNORDERED_SET_HPP_

#include "../byte_stream_segmented.hpp"
#include "../builder_base.hpp"
#include "../view_base.hpp"

#include <unordered_set>


namespace mimir {

template<typename V>
class UnorderedSet : public UncopyableMixin<UnorderedSet<V>> {
private:
    // Persistent storage
    ByteStreamSegmented<100000> m_storage;

    // Data to be accessed
    std::unordered_set<V> m_data;

public:
    [[nodiscard]] V insert(const Builder<V>& builder) {
        // TODO (Dominik): implement
        const char* data = builder.get_buffer().get_data();
        size_t amount = builder.get_buffer().get_size();
        char* new_data = m_storage.write(data, amount);
        auto view = V(new_data);
        auto it = m_data.find(view);
        if (it != m_data.end()) {
            // not unique, mark the storage as free again
            m_storage.undo_last_write();
            return *it;
        }
        auto result = m_data.insert(view);
        return *result.first;
    }

    [[nodiscard]] size_t get_size() const {
        return m_data.size();
    }
};





}


#endif
