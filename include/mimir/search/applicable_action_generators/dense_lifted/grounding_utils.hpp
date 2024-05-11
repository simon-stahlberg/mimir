#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_GROUNDING_UTILS_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_GROUNDING_UTILS_HPP_

#include "mimir/formalism/factories.hpp"

#include <vector>

namespace mimir
{

inline void ground_variables(const TermList& terms, const ObjectList& binding, ObjectList& out_terms)
{
    out_terms.clear();

    for (const auto& term : terms)
    {
        if (const auto term_object = std::get_if<TermObjectImpl>(term))
        {
            out_terms.emplace_back(term_object->get_object());
        }
        else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
        {
            out_terms.emplace_back(binding[term_variable->get_variable()->get_parameter_index()]);
        }
    }
}

inline GroundLiteral ground_literal(const Literal& literal, const ObjectList& binding, PDDLFactories& pddl_factories)
{
    ObjectList grounded_terms;
    ground_variables(literal->get_atom()->get_terms(), binding, grounded_terms);
    auto grounded_atom = pddl_factories.get_or_create_ground_atom(literal->get_atom()->get_predicate(), grounded_terms);
    auto grounded_literal = pddl_factories.get_or_create_ground_literal(literal->is_negated(), grounded_atom);
    return grounded_literal;
}

}

#endif