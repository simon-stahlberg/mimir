#include "mimir/search/axioms/dense.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/factories.hpp"

#include <flatmemory/flatmemory.hpp>
#include <ostream>
#include <tuple>

namespace mimir
{

DenseGroundAxiomList to_ground_axioms(const FlatDenseAxiomSet& flat_axioms)
{
    auto result = DenseGroundAxiomList {};
    result.reserve(flat_axioms.size());
    for (const auto& flat_axiom : flat_axioms)
    {
        result.push_back(DenseGroundAxiom(flat_axiom));
    }
    return result;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<DenseGroundAxiom, const PDDLFactories&>& data)
{
    const auto [axiom, pddl_factories] = data;

    auto positive_precondition_bitset = axiom.get_applicability_positive_precondition_bitset();
    auto negative_precondition_bitset = axiom.get_applicability_negative_precondition_bitset();

    auto positive_precondition = GroundAtomList {};
    auto negative_precondition = GroundAtomList {};

    pddl_factories.get_ground_atoms_from_ids(positive_precondition_bitset, positive_precondition);
    pddl_factories.get_ground_atoms_from_ids(negative_precondition_bitset, negative_precondition);

    os << "Axiom("
       << "id=" << axiom.get_id() << ", "
       << "name=" << axiom.get_axiom()->get_literal()->get_atom()->get_predicate()->get_name() << ", "
       << "positive precondition=" << positive_precondition << ", "
       << "negative precondition=" << negative_precondition << ", "
       << "effect=" << std::make_tuple(axiom.get_simple_effect(), std::cref(pddl_factories)) << ")";

    return os;
}

}
