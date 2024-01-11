#ifndef MIMIR_PLANNERS_HEURISTIC_H1_HPP_
#define MIMIR_PLANNERS_HEURISTIC_H1_HPP_

#include "../../formalism/atom.hpp"
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

        mimir::formalism::Repository repository_;
        std::vector<InternalAction> actions_;
        std::vector<int32_t> goal_;
        mutable std::vector<double> table_;

        double evaluate(const std::vector<int32_t>& ranks) const;
        void update(int32_t rank, double value, bool& changed) const;
        void fill_tables(const mimir::formalism::State& state) const;

      public:
        H1Heuristic(const mimir::formalism::Repository& repository, const mimir::planners::SuccessorGenerator& successor_generator);

        double evaluate(const mimir::formalism::State& state) const override;
    };

    std::shared_ptr<H1Heuristic> create_h1_heuristic(const mimir::formalism::Repository& repository,
                                                     const mimir::planners::SuccessorGenerator& successor_generator);
}  // namespace planners

#endif  // MIMIR_PLANNERS_HEURISTIC_H1_HPP_