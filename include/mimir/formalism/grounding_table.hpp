#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_GROUNDING_TABLE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_GROUNDING_TABLE_HPP_

#include "mimir/common/hash.hpp"
#include "mimir/formalism/declarations.hpp"

#include <unordered_map>

namespace mimir
{

template<typename T>
using GroundingTable = std::unordered_map<ObjectList, T, Hash<ObjectList>>;

template<typename T>
using GroundingTableList = std::vector<GroundingTable<T>>;

}

#endif