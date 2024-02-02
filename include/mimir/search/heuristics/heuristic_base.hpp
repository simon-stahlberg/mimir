#ifndef MIMIR_PLANNERS_HEURISTIC_BASE_HPP_
#define MIMIR_PLANNERS_HEURISTIC_BASE_HPP_

#include "../../formalism/state.hpp"

#include <memory>

namespace mimir::planners
{
    class HeuristicBase
    {
      public:
        static constexpr double DEAD_END = std::numeric_limits<double>::infinity();

        static inline bool is_dead_end(double value) { return std::isinf(value); }

        virtual ~HeuristicBase() = default;

        /// @brief Find a plan for the associated problem
        /// @param out_plan The plan, if one was found
        /// @return The result of the planning step
        virtual double evaluate(const mimir::formalism::State& state) const = 0;

        virtual std::vector<double> evaluate(const mimir::formalism::StateList& states) const
        {
            std::vector<double> values;

            for (const auto& state : states)
            {
                values.emplace_back(evaluate(state));
            }

            return values;
        }
    };

    using Heuristic = std::shared_ptr<HeuristicBase>;
}  // namespace mimir::planners

#endif  // MIMIR_PLANNERS_HEURISTIC_BASE_HPP_
