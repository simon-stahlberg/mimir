#ifndef MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
#define MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_

#include "../algorithm_base.hpp"


namespace mimir
{

/// @brief A general implementation of a breadth-first-search.
/// @tparam Configuration
template<typename Configuration>
class BrFS : public AlgorithmBase<BrFS<Configuration>> {
private:
    // Implement configuration independent functionality.
};


template<typename Configuration>
struct TypeTraits<BrFS<Configuration>> {
    using ConfigurationType = Configuration;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_BRFS_HPP_
