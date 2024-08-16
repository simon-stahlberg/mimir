#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_GROUNDING_TABLE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_GROUNDING_TABLE_HPP_

#include "mimir/common/hash.hpp"
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
    std::unordered_map<ObjectList, T, Hash<ObjectList>> m_table;

public:
    auto find(const ObjectList& binding) const { return m_table.find(binding); }

    auto emplace(ObjectList&& binding, T&& grounding) { return m_table.emplace(std::move(binding), grounding); }

    auto end() const { return m_table.end(); }
};

template<typename T>
using GroundingTableList = std::vector<GroundingTable<T>>;

template<typename... Ts>
class VariadicGroundingTableList
{
private:
    std::tuple<GroundingTableList<Ts>...> m_grounding_tables;

public:
    VariadicGroundingTableList() : m_grounding_tables(std::make_tuple(GroundingTableList<Ts>()...)) {}

    template<typename T>
    GroundingTableList<T>& get()
    {
        return std::get<GroundingTableList<T>>(m_grounding_tables);
    }

    template<typename T>
    const GroundingTableList<T>& get() const
    {
        return std::get<GroundingTableList<T>>(m_grounding_tables);
    }
};

}

#endif