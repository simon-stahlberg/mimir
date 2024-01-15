#ifndef MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_

#include "../common/config.hpp"
#include "../common/mixins.hpp"

#include <stdexcept>
#include <vector>


namespace mimir::search
{

/// @brief Top-level CRTP based interface for a SuccessorGenerator.
/// @tparam Derived
template<typename Derived>
class SuccessorGeneratorBase {
private:
    SuccessorGeneratorBase() = default;
    friend Derived;

public:

};


template<typename Configuration>
class SuccessorGenerator : public SuccessorGeneratorBase<SuccessorGenerator<Configuration>> {
    // Implement configuration independent functionality.
};


}  // namespace mimir::search

#endif  // MIMIR_SEARCH_SUCCESSOR_GENERATOR_BASE_HPP_
