#if !defined(PLANNERS_BREADTH_FIRST_SEARCH_HPP_)
#define PLANNERS_BREADTH_FIRST_SEARCH_HPP_

#include "../formalism/problem.hpp"
#include "../generators/successor_generator.hpp"
#include "search_base.hpp"

#include <memory>

namespace planners
{
    class BreadthFirstSearchImpl : public SearchBase
    {
      private:
        formalism::ProblemDescription problem_;
        planners::SuccessorGenerator successor_generator_;
        double max_g_value_;
        int32_t max_depth_;
        int32_t expanded_;
        int32_t generated_;

        void reset_statistics();

      public:
        BreadthFirstSearchImpl(const formalism::ProblemDescription& problem, const planners::SuccessorGenerator& successor_generator);

        std::map<std::string, std::variant<int32_t, double>> get_statistics() const override;

        SearchResult plan(formalism::ActionList& out_plan) override;
    };

    using BreadthFirstSearch = std::shared_ptr<BreadthFirstSearchImpl>;

    BreadthFirstSearch create_breadth_first_search(const formalism::ProblemDescription& problem, const planners::SuccessorGenerator& successor_generator);
}  // namespace planners

#endif  // PLANNERS_BREADTH_FIRST_SEARCH_HPP_
