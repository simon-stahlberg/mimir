#if !defined(PLANNERS_HEURISTIC_H2_HPP_)
#define PLANNERS_HEURISTIC_H2_HPP_

#include "../../formalism/atom.hpp"
#include "../../formalism/problem.hpp"
#include "../../formalism/state.hpp"
#include "../../generators/successor_generator.hpp"
#include "heuristic_base.hpp"

#include <limits>
#include <unordered_map>
#include <vector>

namespace planners
{
    class H2Heuristic : public HeuristicBase
    {
      private:
        using InternalAction = std::tuple<std::vector<int32_t>, std::vector<int32_t>, std::vector<int32_t>, double>;

        formalism::ProblemDescription problem_;
        std::vector<InternalAction> actions_;
        std::vector<int32_t> goal_;
        mutable std::vector<double> ht1_;
        mutable std::vector<std::vector<double>> ht2_;

        double evaluate(const std::vector<int32_t>& s) const;
        double evaluate(const std::vector<int32_t>& s, int32_t x) const;
        void update(const std::size_t val, const int32_t h, bool& changed) const;
        void update(const std::size_t val1, const std::size_t val2, const int32_t h, bool& changed) const;
        void fill_tables(const formalism::State& state) const;

      public:
        H2Heuristic(const formalism::ProblemDescription& problem, const planners::SuccessorGenerator& successor_generator);

        double evaluate(const formalism::State& state) const override;
    };

    std::shared_ptr<H2Heuristic> create_h2_heuristic(const formalism::ProblemDescription& problem, const planners::SuccessorGenerator& successor_generator);
}  // namespace planners

#endif  // PLANNERS_HEURISTIC_H2_HPP_
