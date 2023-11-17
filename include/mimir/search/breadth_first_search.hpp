#ifndef MIMIR_PLANNERS_BREADTH_FIRST_SEARCH_HPP_
#define MIMIR_PLANNERS_BREADTH_FIRST_SEARCH_HPP_

#include "../formalism/problem.hpp"
#include "../generators/successor_generator.hpp"
#include "search_base.hpp"

#include <memory>

namespace mimir::planners
{
    class BreadthFirstSearchImpl : public SearchBase
    {
      private:
        mimir::formalism::ProblemDescription problem_;
        mimir::planners::SuccessorGenerator successor_generator_;
        double max_g_value_;
        int32_t max_depth_;
        int32_t expanded_;
        int32_t generated_;

        void reset_statistics();

      public:
        BreadthFirstSearchImpl(const mimir::formalism::ProblemDescription& problem, const mimir::planners::SuccessorGenerator& successor_generator);

        std::map<std::string, std::variant<int32_t, double>> get_statistics() const override;

        SearchResult plan(mimir::formalism::ActionList& out_plan) override;
    };

    using BreadthFirstSearch = std::shared_ptr<BreadthFirstSearchImpl>;

    BreadthFirstSearch create_breadth_first_search(const mimir::formalism::ProblemDescription& problem,
                                                   const mimir::planners::SuccessorGenerator& successor_generator);
}  // namespace planners

#endif  // MIMIR_PLANNERS_BREADTH_FIRST_SEARCH_HPP_
