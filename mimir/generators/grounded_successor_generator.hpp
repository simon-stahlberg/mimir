#if !defined(PLANNERS_GROUNDED_SUCCESSOR_GENERATOR_HPP_)
#define PLANNERS_GROUNDED_SUCCESSOR_GENERATOR_HPP_

#include "../formalism/action.hpp"
#include "../formalism/declarations.hpp"
#include "../formalism/problem.hpp"
#include "../formalism/state.hpp"
#include "successor_generator.hpp"

#include <memory>
#include <unordered_set>
#include <vector>

namespace planners
{
    class DecisionNode
    {
      public:
        virtual ~DecisionNode();

        virtual void get_applicable_actions(const formalism::State& state, formalism::ActionList& applicable_actions) const = 0;
    };

    class BranchNode : public DecisionNode
    {
      public:
        uint32_t rank_;
        std::unique_ptr<DecisionNode> present_;
        std::unique_ptr<DecisionNode> not_present_;
        std::unique_ptr<DecisionNode> dont_care_;

        BranchNode(uint32_t rank);

        void get_applicable_actions(const formalism::State& state, formalism::ActionList& applicable_actions) const override;
    };

    class LeafNode : public DecisionNode
    {
      public:
        formalism::ActionList actions_;

        LeafNode(const formalism::ActionList& actions);

        void get_applicable_actions(const formalism::State& state, formalism::ActionList& applicable_actions) const override;
    };

    class GroundedSuccessorGenerator : public SuccessorGeneratorBase
    {
      private:
        formalism::ProblemDescription problem_;
        formalism::ActionList actions_;
        std::unique_ptr<DecisionNode> root_;

      public:
        GroundedSuccessorGenerator(const formalism::ProblemDescription& problem, const formalism::ActionList& ground_actions);

        formalism::ProblemDescription get_problem() const override;

        const formalism::ActionList& get_actions() const;

        formalism::ActionList get_applicable_actions(const formalism::State& state) const override;

      private:
        formalism::AtomList::const_iterator
        select_branching_atom(const formalism::ActionList& ground_actions, const formalism::AtomList& atoms, formalism::AtomList::const_iterator next_atom);

        std::unique_ptr<DecisionNode> build_decision_tree(const formalism::ProblemDescription& problem, const formalism::ActionList& ground_actions);

        std::unique_ptr<DecisionNode> build_decision_tree_recursive(const formalism::ProblemDescription& problem,
                                                                    const formalism::ActionList& ground_actions,
                                                                    const formalism::AtomList& atoms,
                                                                    formalism::AtomList::const_iterator next_atom);
    };
}  // namespace planners

#endif  // PLANNERS_GROUNDED_SUCCESSOR_GENERATOR_HPP_
