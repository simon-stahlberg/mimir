#ifndef MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_HPP_
#define MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_HPP_

#include "../successor_generator_base.hpp"

#include "../../common/config.hpp"
#include "../../common/mixins.hpp"

#include <stdexcept>
#include <vector>


namespace mimir
{

/// @brief General implementation.
template<typename Configuration>
class SuccessorGenerator : public SuccessorGeneratorBase<SuccessorGenerator<Configuration>> {
private:
    using ConfigurationType = Configuration;
    friend class SuccessorGeneratorBase<SuccessorGenerator<Configuration>>;

    // Implement configuration independent functionality.
};

// Specialize for each specific Derived type
template<typename Configuration>
struct ConfigurationTypeTrait<SuccessorGenerator<Configuration>> {
    using ConfigurationType = Configuration;
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_SUCCESSOR_GENERATORS_SUCCESSOR_GENERATOR_HPP_
