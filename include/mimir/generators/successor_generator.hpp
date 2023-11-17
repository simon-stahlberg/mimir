#ifndef MIMIR_PLANNERS_SUCCESSOR_GENERATOR_HPP_
#define MIMIR_PLANNERS_SUCCESSOR_GENERATOR_HPP_

#include "../formalism/declarations.hpp"
#include "../formalism/state.hpp"

#include <memory>

namespace mimir::planners
{
    class SuccessorGeneratorBase
    {
      public:
        virtual ~SuccessorGeneratorBase() = default;

        virtual mimir::formalism::ProblemDescription get_problem() const = 0;

        virtual mimir::formalism::ActionList get_applicable_actions(const mimir::formalism::State& state) const = 0;
    };

    using SuccessorGenerator = std::shared_ptr<SuccessorGeneratorBase>;
}  // namespace planners

#endif  // MIMIR_PLANNERS_SUCCESSOR_GENERATOR_HPP_
