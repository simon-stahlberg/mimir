#if !defined(PLANNERS_BREADTH_FIRST_SEARCH_HPP_)
#define PLANNERS_BREADTH_FIRST_SEARCH_HPP_

#include "../formalism/problem.hpp"
#include "../generators/successor_generator.hpp"
#include "search_base.hpp"

namespace planners
{
    class BreadthFirstSearch : public SearchBase
    {
      private:
        std::map<std::string, std::variant<int32_t, double>> statistics_;
        formalism::ProblemDescription problem_;
        planners::SuccessorGenerator successor_generator_;

      public:
        BreadthFirstSearch(const formalism::ProblemDescription& problem, const planners::SuccessorGenerator& successor_generator);

        std::map<std::string, std::variant<int32_t, double>> get_statistics() const override;

        SearchResult plan(formalism::ActionList& out_plan) override;
    };
}  // namespace planners

#endif  // PLANNERS_BREADTH_FIRST_SEARCH_HPP_
