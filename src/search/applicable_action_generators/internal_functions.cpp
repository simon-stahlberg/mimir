#include <mimir/search/applicable_action_generators/internal_functions.hpp>

namespace mimir
{

    Assignment::Assignment(size_t parameter_index, size_t object_id) : parameter_index(parameter_index), object_id(object_id) {}

    AssignmentPair::AssignmentPair(size_t first_position, const Assignment& first_assignment, size_t second_position, const Assignment& second_assignment) :
        first_position(first_position),
        second_position(second_position),
        first_assignment(first_assignment),
        second_assignment(second_assignment)
    {
    }

size_t get_assignment_position(int32_t first_position, int32_t first_object, int32_t second_position, int32_t second_object, int32_t arity, int32_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto rank = (first * (first_position + 1)) + (second * (second_position + 1)) + (third * (first_object + 1)) + (fourth * (second_object + 1));
    return (size_t) rank;
}

size_t num_assignments(int32_t arity, int32_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto max = (first * arity) + (second * arity) + (third * num_objects) + (fourth * num_objects);
    return (size_t) (max + 1);
}

std::vector<std::vector<bool>> build_assignment_sets(Problem problem, const std::vector<size_t>& atom_identifiers, const PDDLFactories& factories)
{
    const auto num_objects = problem->get_objects().size();
    const auto& predicates = problem->get_domain()->get_predicates();

    std::vector<std::vector<bool>> assignment_sets;
    assignment_sets.resize(predicates.size());

    for (const auto& predicate : predicates)
    {
        auto& assignment_set = assignment_sets[predicate->get_identifier()];
        assignment_set.resize(num_assignments(predicate->get_arity(), num_objects));
    }

    for (const auto& identifier : atom_identifiers)
    {
        const auto& ground_atom = factories.ground_atoms.get(identifier);
        const auto& arity = ground_atom->get_arity();
        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = assignment_sets[predicate->get_identifier()];

        for (size_t first_position = 0; first_position < arity; ++first_position)
        {
            const auto& first_object = arguments[first_position];
            assignment_set[get_assignment_position(first_position, first_object->get_identifier(), -1, -1, arity, num_objects)] = true;

            for (size_t second_position = first_position + 1; second_position < arity; ++second_position)
            {
                const auto& second_object = arguments[second_position];
                assignment_set[get_assignment_position(second_position, second_object->get_identifier(), -1, -1, arity, num_objects)] = true;
                assignment_set[get_assignment_position(first_position,
                                                       first_object->get_identifier(),
                                                       second_position,
                                                       second_object->get_identifier(),
                                                       arity,
                                                       num_objects)] = true;
            }
        }
    }

    return assignment_sets;
}

bool literal_all_consistent(const std::vector<std::vector<bool>>& assignment_sets,
                            const std::vector<FlatLiteral>& literals,
                            const Assignment& first_assignment,
                            const Assignment& second_assignment,
                            Problem problem)
{
    for (const auto& literal : literals)
    {
        int32_t first_position = -1;
        int32_t second_position = -1;
        int32_t first_object_id = -1;
        int32_t second_object_id = -1;
        bool empty_assignment = true;

        for (std::size_t index = 0; index < literal.arity; ++index)
        {
            const auto& term = literal.arguments[index];

            if (term.is_constant())
            {
                if (literal.arity <= 2)
                {
                    const auto term_id = term.get_value();

                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(term_id);
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(term_id);
                    }

                    empty_assignment = false;
                }
            }
            else
            {
                const auto term_index = term.get_value();

                if (first_assignment.parameter_index == term_index)
                {
                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(first_assignment.object_id);
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(first_assignment.object_id);
                        break;
                    }

                    empty_assignment = false;
                }
                else if (second_assignment.parameter_index == term_index)
                {
                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(second_assignment.object_id);
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(second_assignment.object_id);
                        break;
                    }

                    empty_assignment = false;
                }
            }
        }

        if (!empty_assignment)
        {
            const auto& assignment_set = assignment_sets[literal.predicate_id];
            const auto assignment_rank = get_assignment_position(first_position,
                                                                 first_object_id,
                                                                 second_position,
                                                                 second_object_id,
                                                                 static_cast<int32_t>(literal.arity),
                                                                 static_cast<int32_t>(problem->get_objects().size()));

            const auto consistent_with_state = assignment_set[assignment_rank];

            if (!literal.negated && !consistent_with_state)
            {
                return false;
            }
            else if (literal.negated && consistent_with_state && ((literal.arity == 1) || ((literal.arity == 2) && (second_position >= 0))))
            {
                return false;
            }
        }
    }

    return true;
}

ObjectList ground_parameters(const std::vector<ParameterIndexOrConstantId>& parameters, const ObjectList& terms, const PDDLFactories& factories)
{
    ObjectList atom_terms;

    for (const auto& term : parameters)
    {
        if (term.is_constant())
        {
            atom_terms.emplace_back(factories.objects.get(term.get_value()));
        }
        else
        {
            atom_terms.emplace_back(terms[term.get_value()]);
        }
    }

    return atom_terms;
}

GroundLiteral ground_literal(const FlatLiteral& flat_literal, const ObjectList& terms, PDDLFactories& ref_factories)
{
    auto predicate = flat_literal.source->get_atom()->get_predicate();
    auto atom = ref_factories.get_or_create_ground_atom(predicate, ground_parameters(flat_literal.arguments, terms, ref_factories));
    auto literal = ref_factories.get_or_create_ground_literal(flat_literal.negated, atom);
    return literal;
}

Action create_action(const FlatActionSchema& flat_action_schema, ObjectList&& terms, PDDLFactories& ref_factories)
{
    // Get the precondition of the ground action

    GroundLiteralList precondition;

    for (const auto& literal : flat_action_schema.static_precondition)
    {
        precondition.emplace_back(ground_literal(literal, terms, ref_factories));
    }

    for (const auto& literal : flat_action_schema.fluent_precondition)
    {
        precondition.emplace_back(ground_literal(literal, terms, ref_factories));
    }

    // Get the effect of the ground action

    GroundLiteralList unconditional_effect;

    for (const auto& literal : flat_action_schema.unconditional_effect)
    {
        unconditional_effect.emplace_back(ground_literal(literal, terms, ref_factories));
    }

    // Get the cost of the ground action

    // const double cost = 1.0;

    // TODO: Implement...

    // Finally, create the ground action

    throw std::runtime_error("creating ations is not implemented");

    // return create_action(problem,
    //                      flat_action_schema.source,
    //                      std::move(terms),
    //                      std::move(precondition),
    //                      std::move(unconditional_effect),
    //                      std::move(conditional_effect),
    //                      cost);
}

}
