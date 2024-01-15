#ifndef MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_GROUNDED_HPP_
#define MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_GROUNDED_HPP_

#include "../successor_generator_base.hpp"

#include "../../common/config.hpp"
#include "../../common/mixins.hpp"

#include <stdexcept>
#include <vector>


namespace mimir::search
{

/// @brief Concrete implementation of a grounded successor generator.
template<>
class SuccessorGenerator<Grounded> : public SuccessorGeneratorBase<SuccessorGenerator<Grounded>> {
private:
    // Implement configuration specific functionality.
};


}  // namespace mimir::search

#endif  // MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_GROUNDED_HPP_
