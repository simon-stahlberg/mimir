#ifndef MIMIR_PLANNERS_HEURISTIC_H1_HPP_
#define MIMIR_PLANNERS_HEURISTIC_H1_HPP_

#include "../../formalism/atom.hpp"
#include "../../formalism/problem.hpp"
#include "../../formalism/state.hpp"
#include "../../generators/successor_generator.hpp"
#include "heuristic_base.hpp"

#include <limits>
#include <unordered_map>
#include <vector>

namespace mimir::planners
{
    class H1Heuristic : public HeuristicBase
    {
      private:
        using InternalAction = std::tuple<std::vector<int32_t>, std::vector<int32_t>, std::vector<int32_t>, double>;

        mimir::formalism::ProblemDescription problem_;
        std::vector<InternalAction> actions_;
        std::vector<int32_t> goal_;
        mutable std::vector<double> table_;

        double eval(const std::vector<int32_t>& ranks) const;
        void update(int32_t rank, double value, bool& changed) const;
        void fill_tables(const mimir::formalism::State& state) const;

      public:
        H1Heuristic(const mimir::formalism::ProblemDescription& problem, const mimir::planners::SuccessorGenerator& successor_generator);

        double evaluate(const mimir::formalism::State& state) const override;
    };

    std::shared_ptr<H1Heuristic> create_h1_heuristic(const mimir::formalism::ProblemDescription& problem,
                                                     const mimir::planners::SuccessorGenerator& successor_generator);
}  // namespace planners

#endif  // MIMIR_PLANNERS_HEURISTIC_H1_HPP_
