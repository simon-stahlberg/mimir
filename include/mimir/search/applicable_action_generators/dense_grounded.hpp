#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"

#include <loki/loki.hpp>

namespace mimir
{

class MatchTree
{
private:
    using NodeID = size_t;
    using ActionIter = size_t;

    struct SelectorNode
    {
        size_t ground_atom_id;
        NodeID true_succ;
        NodeID false_succ;
        NodeID dontcare_succ;
    };

    struct GeneratorNode
    {
        ActionIter begin;
        ActionIter end;
    };

    using Node = std::variant<SelectorNode, GeneratorNode>;

    std::vector<Node> m_nodes;
    std::vector<ConstDenseActionViewProxy> m_actions;

    using ConstDenseActionViewProxySet =
        std::unordered_set<ConstDenseActionViewProxy, loki::Hash<ConstDenseActionViewProxy>, loki::EqualTo<ConstDenseActionViewProxy>>;

    NodeID build_recursively(const size_t atom_id, size_t const num_atoms, const std::vector<ConstDenseActionViewProxy>& actions);

public:
    MatchTree();
    MatchTree(const size_t num_atoms, const std::vector<ConstDenseActionViewProxy>& actions);

    void get_applicable_actions(const ConstDenseStateViewProxy state, std::vector<ConstDenseActionViewProxy>& out_applicable_actions);

    [[nodiscard]] size_t get_num_nodes() const;
};

/**
 * Fully specialized implementation class.
 */
template<>
class AAG<GroundedAAGDispatcher<DenseStateTag>> : public IStaticAAG<AAG<GroundedAAGDispatcher<DenseStateTag>>>
{
private:
    using ConstStateView = ConstView<StateDispatcher<DenseStateTag>>;
    using ConstActionView = ConstView<ActionDispatcher<DenseStateTag>>;

    Problem m_problem;
    PDDLFactories& m_pddl_factories;

    AAG<LiftedAAGDispatcher<DenseStateTag>> m_lifted_aag;

    Builder<StateDispatcher<DenseStateTag>> m_state_builder;

    MatchTree m_match_tree;

    /* Implement IStaticAAG interface */
    friend class IStaticAAG<AAG<GroundedAAGDispatcher<DenseStateTag>>>;

    void generate_applicable_actions_impl(const ConstStateView state, std::vector<ConstActionView>& out_applicable_actions);

public:
    AAG(Problem problem, PDDLFactories& pddl_factories);

    /// @brief Return the action with the given id.
    [[nodiscard]] ConstActionView get_action(size_t action_id) const;
};
}

#endif