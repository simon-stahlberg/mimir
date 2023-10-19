#if !defined(PLANNERS_EAGER_ASTAR_SEARCH_HPP_)
#define PLANNERS_EAGER_ASTAR_SEARCH_HPP_

#include "../formalism/problem.hpp"
#include "../generators/successor_generator.hpp"
#include "heuristics/heuristic_base.hpp"
#include "openlists/open_list_base.hpp"
#include "search_base.hpp"

#include <memory>

namespace planners
{
    class EagerAStarSearchImpl : public SearchBase
    {
      private:
        std::map<std::string, std::variant<int32_t, double>> statistics_;
        formalism::ProblemDescription problem_;
        planners::SuccessorGenerator successor_generator_;
        planners::Heuristic heuristic_;
        planners::OpenList open_list_;

      public:
        EagerAStarSearchImpl(const formalism::ProblemDescription& problem,
                             const planners::SuccessorGenerator& successor_generator,
                             const planners::Heuristic& heuristic,
                             const planners::OpenList& open_list);

        std::map<std::string, std::variant<int32_t, double>> get_statistics() const override;

        SearchResult plan(formalism::ActionList& out_plan) override;
    };

    using EagerAStarSearch = std::shared_ptr<EagerAStarSearchImpl>;

    EagerAStarSearch create_eager_astar(const formalism::ProblemDescription& problem,
                                        const planners::SuccessorGenerator& successor_generator,
                                        const planners::Heuristic& heuristic,
                                        const planners::OpenList& open_list);
}  // namespace planners

#endif  // PLANNERS_EAGER_ASTAR_SEARCH_HPP_
