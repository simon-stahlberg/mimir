#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_DENSE_LIFTED_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_DENSE_LIFTED_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"
#include "mimir/search/axiom_evaluators/axiom_stratification.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/axioms.hpp"
#include "mimir/search/states.hpp"

#include <flatmemory/details/view_const.hpp>
#include <unordered_map>
#include <vector>

namespace mimir
{

/**
 * Fully specialized implementation class.
 */
template<>
class AE<AEDispatcher<DenseStateTag>> : public IStaticAE<AE<AEDispatcher<DenseStateTag>>>
{
private:
    Problem m_problem;
    PDDLFactories& m_pddl_factories;

    std::vector<AxiomPartition> m_partitioning;

    DenseAxiomSet m_applicable_axioms;

    FlatDenseAxiomSet m_axioms;
    DenseAxiomList m_axioms_by_index;
    DenseAxiomBuilder m_axiom_builder;

    std::unordered_map<Axiom, consistency_graph::StaticConsistencyGraph> m_static_consistency_graphs;

    /// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
    bool nullary_preconditions_hold(const Axiom& axiom, const FlatBitsetBuilder& state_atoms) const;

    void nullary_case(const Axiom& axiom, const FlatBitsetBuilder& state_atoms, GroundAxiomList& out_applicable_axioms);

    void unary_case(const Axiom& axiom, const FlatBitsetBuilder& state_atoms, GroundAxiomList& out_applicable_axioms);

    void general_case(const AssignmentSet& assignment_sets, const Axiom& axiom, const FlatBitsetBuilder& state_atoms, GroundAxiomList& out_applicable_axioms);

    /* Implement IStaticAE interface */
    friend class IStaticAE<AE<AEDispatcher<DenseStateTag>>>;

    void generate_and_apply_axioms_impl(FlatBitsetBuilder& ref_state_atoms);

public:
    AE(Problem problem, PDDLFactories& pddl_factories);

    /// @brief Return the axiom partitioning.
    [[nodiscard]] const std::vector<AxiomPartition>& get_axiom_partitioning() const;

    /// @brief Ground an axiom and return a view onto it.
    [[nodiscard]] GroundAxiom ground_axiom(const Axiom& axiom, ObjectList&& binding);

    /// @brief Return all axioms.
    [[nodiscard]] const FlatDenseAxiomSet& get_axioms() const;

    /// @brief Return all applicable axioms.
    [[nodiscard]] const DenseAxiomSet& get_applicable_axioms() const;
};

/**
 * Types
 */

using DenseAE = AE<AEDispatcher<DenseStateTag>>;

}  // namespace mimir

#endif