#ifndef PLANNERS_SUCCESSOR_GENERATOR_FACTORY_HPP_
#define PLANNERS_SUCCESSOR_GENERATOR_FACTORY_HPP_

#include "../formalism/problem.hpp"
#include "successor_generator.hpp"

#include <memory>

namespace mimir::planners
{
    enum SuccessorGeneratorType
    {
        AUTOMATIC,
        LIFTED,
        GROUNDED
    };

    SuccessorGenerator create_sucessor_generator(const mimir::formalism::ProblemDescription& problem, SuccessorGeneratorType type);
}  // namespace planners

#endif  // PLANNERS_SUCCESSOR_GENERATOR_FACTORY_HPP_
