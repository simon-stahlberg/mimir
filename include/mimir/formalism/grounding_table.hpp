#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_GROUNDING_TABLE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_GROUNDING_TABLE_HPP_

#include "mimir/formalism/object.hpp"

#include <unordered_map>

namespace mimir
{

/**
 * Wrapper around an unordered_map that maps bindings to objects of type T.
 *
 * We use it to cache groundings for actions and axioms.
 */
template<typename T>
class GroundingTable
{
private:
    std::unordered_map<ObjectList, T, loki::hash_container_type<ObjectList>> m_table;

public:
    auto find(const ObjectList& binding) const { return m_table.find(binding); }

    auto emplace(ObjectList&& binding, T&& grounding) { return m_table.emplace(std::move(binding), grounding); }

    auto end() const { return m_table.end(); }
};

}

#endif