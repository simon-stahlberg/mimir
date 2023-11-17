#ifndef MIMIR_PLANNERS_GROUNDED_SUCCESSOR_GENERATOR_HPP_
#define MIMIR_PLANNERS_GROUNDED_SUCCESSOR_GENERATOR_HPP_

#include "../formalism/action.hpp"
#include "../formalism/declarations.hpp"
#include "../formalism/problem.hpp"
#include "../formalism/state.hpp"
#include "successor_generator.hpp"

#include <memory>
#include <vector>

namespace mimir::planners
{
    class DecisionNode
    {
      public:
        virtual ~DecisionNode();

        virtual void get_applicable_actions(const mimir::formalism::State& state, mimir::formalism::ActionList& applicable_actions) const = 0;
    };

    class BranchNode : public DecisionNode
    {
      public:
        uint32_t rank_;
        std::unique_ptr<DecisionNode> present_;
        std::unique_ptr<DecisionNode> not_present_;
        std::unique_ptr<DecisionNode> dont_care_;

        BranchNode(uint32_t rank);

        void get_applicable_actions(const mimir::formalism::State& state, mimir::formalism::ActionList& applicable_actions) const override;
    };

    class LeafNode : public DecisionNode
    {
      public:
        mimir::formalism::ActionList actions_;

        LeafNode(const mimir::formalism::ActionList& actions);

        void get_applicable_actions(const mimir::formalism::State& state, mimir::formalism::ActionList& applicable_actions) const override;
    };

    class GroundedSuccessorGenerator : public SuccessorGeneratorBase
    {
      private:
        mimir::formalism::ProblemDescription problem_;
        mimir::formalism::ActionList actions_;
        std::unique_ptr<DecisionNode> root_;

      public:
        GroundedSuccessorGenerator(const mimir::formalism::ProblemDescription& problem, const mimir::formalism::ActionList& ground_actions);

        mimir::formalism::ProblemDescription get_problem() const override;

        const mimir::formalism::ActionList& get_actions() const;

        mimir::formalism::ActionList get_applicable_actions(const mimir::formalism::State& state) const override;

      private:
        mimir::formalism::AtomList::const_iterator select_branching_atom(const mimir::formalism::ActionList& ground_actions,
                                                                         const mimir::formalism::AtomList& atoms,
                                                                         mimir::formalism::AtomList::const_iterator next_atom);

        std::unique_ptr<DecisionNode> build_decision_tree(const mimir::formalism::ProblemDescription& problem,
                                                          const mimir::formalism::ActionList& ground_actions);

        std::unique_ptr<DecisionNode> build_decision_tree_recursive(const mimir::formalism::ProblemDescription& problem,
                                                                    const mimir::formalism::ActionList& ground_actions,
                                                                    const mimir::formalism::AtomList& atoms,
                                                                    mimir::formalism::AtomList::const_iterator next_atom);
    };
}  // namespace planners

#endif  // MIMIR_PLANNERS_GROUNDED_SUCCESSOR_GENERATOR_HPP_
