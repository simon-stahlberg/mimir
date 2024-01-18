#ifndef MIMIR_FORMALISM_PROBLEM_PROBLEM_HPP_
#define MIMIR_FORMALISM_PROBLEM_PROBLEM_HPP_

#include "../../common/mixins.hpp"

#include <loki/problem/pddl/problem.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace mimir
{
    class ProblemImpl : public FormattingMixin<ProblemImpl>
    {
      private:
        loki::pddl::Problem external_;

        explicit ProblemImpl(loki::pddl::Problem problem);

      public:
    };

    // std::ostream& operator<<(std::ostream& os, const ProblemList& problems);

}  // namespace mimir

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::ProblemImpl>
    {
        std::size_t operator()(const mimir::ProblemImpl& problem) const;
    };

    template<>
    struct less<mimir::ProblemImpl>
    {
        bool operator()(const mimir::ProblemImpl& left_problem, const mimir::ProblemImpl& right_problem) const;
    };

    template<>
    struct equal_to<mimir::ProblemImpl>
    {
        bool operator()(const mimir::ProblemImpl& left_problem, const mimir::ProblemImpl& right_problem) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_PROBLEM_PROBLEM_HPP_
