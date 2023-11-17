#if !defined(PLANNERS_HEURISTIC_BASE_HPP_)
#define PLANNERS_HEURISTIC_BASE_HPP_

#include "../../formalism/state.hpp"

#include <memory>

namespace planners
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
        virtual double evaluate(const formalism::State& state) const = 0;
    };

    using Heuristic = std::shared_ptr<HeuristicBase>;
}  // namespace planners

#endif  // PLANNERS_HEURISTIC_BASE_HPP_
