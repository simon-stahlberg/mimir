#ifndef MIMIR_FORMALISM_GROUNDERS_GROUNDING_TABLE_HPP_
#define MIMIR_FORMALISM_GROUNDERS_GROUNDING_TABLE_HPP_

#include "mimir/formalism/declarations.hpp"

#include <absl/container/flat_hash_map.h>
#include <loki/details/utils/hash.hpp>

namespace mimir::formalism
{

template<typename T>
using GroundingTable = absl::flat_hash_map<ObjectList, T, loki::Hash<ObjectList>>;

template<typename T>
using GroundingTableList = std::vector<GroundingTable<T>>;

}

#endif