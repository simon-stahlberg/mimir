#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"

namespace mimir
{
/**
 * Implementation of https://arxiv.org/pdf/1109.6051.pdf section 5
 *
 * Dominik (2024-04-20): I tried an implementation with std::variant
 * which has similar performance so I switched the to implementation
 * using virtual, which is simpler.
 */
class MatchTree
{
private:
    class INode
    {
    public:
        virtual ~INode() = default;
        virtual void get_applicable_actions(const DenseState state, DenseActionList& out_applicable_actions) = 0;
    };

    class SelectorNode : public INode
    {
    private:
        size_t m_atom_id;
        std::unique_ptr<INode> m_true_succ;
        std::unique_ptr<INode> m_false_succ;
        std::unique_ptr<INode> m_dontcare_succ;

    public:
        SelectorNode(size_t ground_atom_id, std::unique_ptr<INode>&& true_succ, std::unique_ptr<INode>&& false_succ, std::unique_ptr<INode>&& dontcare_succ);

        void get_applicable_actions(const DenseState state, DenseActionList& out_applicable_actions) override;
    };

    class GeneratorNode : public INode
    {
    private:
        DenseActionList m_actions;

    public:
        explicit GeneratorNode(DenseActionList actions);

        void get_applicable_actions(const DenseState state, DenseActionList& out_applicable_actions) override;
    };

    size_t m_num_nodes;
    std::unique_ptr<INode> m_root_node;

    std::unique_ptr<INode> build_recursively(const size_t atom_id, size_t const num_atoms, const DenseActionList& actions);

public:
    MatchTree();
    MatchTree(const size_t num_atoms, const DenseActionList& actions);

    void get_applicable_actions(const DenseState state, DenseActionList& out_applicable_actions);

    [[nodiscard]] size_t get_num_nodes() const;
};

/**
 * Fully specialized implementation class.
 */
template<>
class AAG<GroundedAAGDispatcher<DenseStateTag>> : public IStaticAAG<AAG<GroundedAAGDispatcher<DenseStateTag>>>
{
private:
    Problem m_problem;
    PDDLFactories& m_pddl_factories;

    LiftedDenseAAG m_lifted_aag;

    DenseStateBuilder m_state_builder;

    MatchTree m_match_tree;

    /* Implement IStaticAAG interface */
    friend class IStaticAAG<AAG<GroundedAAGDispatcher<DenseStateTag>>>;

    void generate_applicable_actions_impl(const DenseState state, DenseActionList& out_applicable_actions);

    void generate_and_apply_axioms_impl(FlatBitsetBuilder& ref_ground_atoms);

public:
    AAG(Problem problem, PDDLFactories& pddl_factories);

    /// @brief Return all actions.
    [[nodiscard]] const FlatDenseActionSet& get_actions() const;

    /// @brief Return the action with the given id.
    [[nodiscard]] DenseAction get_action(size_t action_id) const;
};

/**
 * Types
 */

using GroundedDenseAAG = AAG<GroundedAAGDispatcher<DenseStateTag>>;

}

#endif