#if !defined(PLANNERS_LIFTED_SUCCESSOR_GENERATOR_HPP_)
#define PLANNERS_LIFTED_SUCCESSOR_GENERATOR_HPP_

#include "../formalism/action.hpp"
#include "../formalism/action_schema.hpp"
#include "../formalism/domain.hpp"
#include "../formalism/problem.hpp"
#include "../formalism/state.hpp"
#include "lifted_schema_successor_generator.hpp"
#include "successor_generator.hpp"

#include <map>
#include <vector>

namespace planners
{
    class LiftedSuccessorGenerator : public SuccessorGeneratorBase
    {
      private:
        formalism::ProblemDescription problem_;
        std::map<formalism::ActionSchema, LiftedSchemaSuccessorGenerator> generators_;

      public:
        LiftedSuccessorGenerator(const formalism::ProblemDescription& problem);

        formalism::ProblemDescription get_problem() const override;

        formalism::ActionList get_applicable_actions(const formalism::State& state) const override;

        bool get_applicable_actions(const std::chrono::high_resolution_clock::time_point end_time,
                                    const formalism::State& state,
                                    formalism::ActionList& out_actions) const;
    };
}  // namespace planners

#endif  // PLANNERS_LIFTED_SUCCESSOR_GENERATOR_HPP_
