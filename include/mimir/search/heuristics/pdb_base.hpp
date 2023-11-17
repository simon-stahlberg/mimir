#ifndef MIMIR_PLANNERS_PDB_BASE_HPP_
#define MIMIR_PLANNERS_PDB_BASE_HPP_

#include "../../formalism/problem.hpp"
#include "../../formalism/state.hpp"
#include "../../generators/successor_generator.hpp"
#include "heuristic_base.hpp"

namespace mimir::planners
{
    class PDBBase : public HeuristicBase
    {
      private:
        mimir::formalism::ProblemDescription problem_;
        std::vector<mimir::formalism::ActionList> relevant_actions_;  // An action is in the list at index i if it affects the atom with rank i

      protected:
        PDBBase(const mimir::formalism::ProblemDescription& problem, const mimir::planners::SuccessorGenerator& successor_generator);

        std::vector<double> compute_table(const std::vector<int32_t>& pattern);

        std::size_t get_index(const mimir::formalism::State& state, const std::vector<int32_t>& pattern);
    };

}  // namespace planners

#endif  // MIMIR_PLANNERS_PDB_BASE_HPP_
