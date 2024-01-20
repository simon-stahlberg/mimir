
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
    [[nodiscard]] View<T> insert(const Builder<T>& builder) {
        // TODO (Dominik): implement
        const char* data = builder.get_buffer().get_data();
        size_t amount = builder.get_buffer().get_size();
        char* new_data = m_storage.write(data, amount);
        auto view = View<T>(new_data);
        auto it = m_data.find(view);
        if (it != m_data.end()) {
            // not unique, mark the storage as free again
            m_storage.erase(amount);
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
