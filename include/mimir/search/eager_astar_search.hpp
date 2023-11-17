#ifndef MIMIR_PLANNERS_EAGER_ASTAR_SEARCH_HPP_
#define MIMIR_PLANNERS_EAGER_ASTAR_SEARCH_HPP_

#include "../formalism/problem.hpp"
#include "../generators/successor_generator.hpp"
#include "heuristics/heuristic_base.hpp"
#include "openlists/open_list_base.hpp"
#include "search_base.hpp"

#include <memory>

namespace mimir::planners
{
    class EagerAStarSearchImpl : public SearchBase
    {
      private:
        mimir::formalism::ProblemDescription problem_;
        mimir::planners::SuccessorGenerator successor_generator_;
        mimir::planners::Heuristic heuristic_;
        mimir::planners::OpenList open_list_;
        double max_g_value_;
        double max_f_value_;
        int32_t max_depth_;
        int32_t expanded_;
        int32_t generated_;
        int32_t evaluated_;

        void reset_statistics();

      public:
        EagerAStarSearchImpl(const mimir::formalism::ProblemDescription& problem,
                             const mimir::planners::SuccessorGenerator& successor_generator,
                             const mimir::planners::Heuristic& heuristic,
                             const mimir::planners::OpenList& open_list);

        std::map<std::string, std::variant<int32_t, double>> get_statistics() const override;

        SearchResult plan(mimir::formalism::ActionList& out_plan) override;
    };

    using EagerAStarSearch = std::shared_ptr<EagerAStarSearchImpl>;

    EagerAStarSearch create_eager_astar(const mimir::formalism::ProblemDescription& problem,
                                        const mimir::planners::SuccessorGenerator& successor_generator,
                                        const mimir::planners::Heuristic& heuristic,
                                        const mimir::planners::OpenList& open_list);
}  // namespace planners

#endif  // MIMIR_PLANNERS_EAGER_ASTAR_SEARCH_HPP_
