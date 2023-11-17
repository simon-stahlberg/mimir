#ifndef MIMIR_PLANNERS_LIFTED_SUCCESSOR_GENERATOR_HPP_
#define MIMIR_PLANNERS_LIFTED_SUCCESSOR_GENERATOR_HPP_

#include "../formalism/action.hpp"
#include "../formalism/action_schema.hpp"
#include "../formalism/domain.hpp"
#include "../formalism/problem.hpp"
#include "../formalism/state.hpp"
#include "lifted_schema_successor_generator.hpp"
#include "successor_generator.hpp"

#include <map>
#include <vector>

namespace mimir::planners
{
    class LiftedSuccessorGenerator : public SuccessorGeneratorBase
    {
      private:
        mimir::formalism::ProblemDescription problem_;
        std::map<mimir::formalism::ActionSchema, LiftedSchemaSuccessorGenerator> generators_;

      public:
        LiftedSuccessorGenerator(const mimir::formalism::ProblemDescription& problem);

        mimir::formalism::ProblemDescription get_problem() const override;

        mimir::formalism::ActionList get_applicable_actions(const mimir::formalism::State& state) const override;

        bool get_applicable_actions(const std::chrono::high_resolution_clock::time_point end_time,
                                    const mimir::formalism::State& state,
                                    mimir::formalism::ActionList& out_actions) const;
    };
}  // namespace planners

#endif  // MIMIR_PLANNERS_LIFTED_SUCCESSOR_GENERATOR_HPP_
