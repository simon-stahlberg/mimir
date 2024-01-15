#ifndef MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_LIFTED_HPP_
#define MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_LIFTED_HPP_

#include "../successor_generator_base.hpp"

#include "../../common/config.hpp"
#include "../../common/mixins.hpp"

#include <stdexcept>
#include <vector>


namespace mimir::search
{

/// @brief Concrete implementation of a lifted successor generator.
template<>
class SuccessorGenerator<Lifted> : public SuccessorGeneratorBase<SuccessorGenerator<Lifted>> {
private:
    // Implement configuration specific functionality.
};


}  // namespace mimir::search

#endif  // MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_LIFTED_HPP_
