#if !defined(PLANNERS_SUCCESSOR_GENERATOR_HPP_)
#define PLANNERS_SUCCESSOR_GENERATOR_HPP_

#include "../formalism/declarations.hpp"
#include "../formalism/state.hpp"

#include <memory>

namespace planners
{
    class SuccessorGeneratorBase
    {
      public:
        virtual ~SuccessorGeneratorBase() = default;

        virtual formalism::ProblemDescription get_problem() const = 0;

        virtual formalism::ActionList get_applicable_actions(const formalism::State& state) const = 0;
    };

    using SuccessorGenerator = std::shared_ptr<SuccessorGeneratorBase>;
}  // namespace planners

#endif  // PLANNERS_SUCCESSOR_GENERATOR_HPP_
