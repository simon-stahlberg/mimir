/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/formalism/grounders/literal_grounder.hpp"

#include "mimir/formalism/grounders/variable_grounder.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/utils.hpp"

namespace mimir
{

LiteralGrounder::LiteralGrounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    m_problem(problem),
    m_pddl_repositories(std::move(pddl_repositories))
{
}

template<PredicateTag P>
GroundLiteral<P> LiteralGrounder::ground(Literal<P> literal, const ObjectList& binding)
{
    /* 1. Access the type specific grounding tables. */
    auto& grounding_tables = boost::hana::at_key(m_grounding_tables, boost::hana::type<GroundLiteral<P>> {});

    /* 2. Access the context-independent literal grounding table */
    const auto is_negated = literal->is_negated();
    const auto predicate_index = literal->get_atom()->get_predicate()->get_index();
    auto& polarity_grounding_tables = grounding_tables[is_negated];
    if (predicate_index >= polarity_grounding_tables.size())
    {
        polarity_grounding_tables.resize(predicate_index + 1);
    }

    auto& grounding_table = polarity_grounding_tables.at(predicate_index);

    /* 3. Check if grounding is cached */

    // We have to fetch the literal-relevant part of the binding first.
    auto grounded_terms = ObjectList {};
    ground_variables(literal->get_atom()->get_terms(), binding, grounded_terms);

    const auto it = grounding_table.find(grounded_terms);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 4. Ground the literal */

    auto grounded_atom = m_pddl_repositories->get_or_create_ground_atom(literal->get_atom()->get_predicate(), grounded_terms);
    auto grounded_literal = m_pddl_repositories->get_or_create_ground_literal(literal->is_negated(), grounded_atom);

    /* 5. Insert to grounding_table table */

    grounding_table.emplace(std::move(grounded_terms), GroundLiteral<P>(grounded_literal));

    /* 6. Return the resulting ground literal */

    return grounded_literal;
}

template GroundLiteral<Static> LiteralGrounder::ground(Literal<Static> literal, const ObjectList& binding);
template GroundLiteral<Fluent> LiteralGrounder::ground(Literal<Fluent> literal, const ObjectList& binding);
template GroundLiteral<Derived> LiteralGrounder::ground(Literal<Derived> literal, const ObjectList& binding);

template<PredicateTag P>
void LiteralGrounder::ground_and_fill_bitset(const std::vector<Literal<P>>& literals,
                                             FlatBitset& ref_positive_bitset,
                                             FlatBitset& ref_negative_bitset,
                                             const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        const auto grounded_literal = ground(literal, binding);

        if (grounded_literal->is_negated())
        {
            ref_negative_bitset.set(grounded_literal->get_atom()->get_index());
        }
        else
        {
            ref_positive_bitset.set(grounded_literal->get_atom()->get_index());
        }
    }
}

template void LiteralGrounder::ground_and_fill_bitset(const std::vector<Literal<Static>>& literals,
                                                      FlatBitset& ref_positive_bitset,
                                                      FlatBitset& ref_negative_bitset,
                                                      const ObjectList& binding);
template void LiteralGrounder::ground_and_fill_bitset(const std::vector<Literal<Fluent>>& literals,
                                                      FlatBitset& ref_positive_bitset,
                                                      FlatBitset& ref_negative_bitset,
                                                      const ObjectList& binding);
template void LiteralGrounder::ground_and_fill_bitset(const std::vector<Literal<Derived>>& literals,
                                                      FlatBitset& ref_positive_bitset,
                                                      FlatBitset& ref_negative_bitset,
                                                      const ObjectList& binding);

template<PredicateTag P>
void LiteralGrounder::ground_and_fill_vector(const std::vector<Literal<P>>& literals,
                                             FlatIndexList& ref_positive_indices,
                                             FlatIndexList& ref_negative_indices,
                                             const ObjectList& binding)
{
    for (const auto& literal : literals)
    {
        const auto grounded_literal = ground(literal, binding);

        if (grounded_literal->is_negated())
        {
            ref_negative_indices.push_back(grounded_literal->get_atom()->get_index());
        }
        else
        {
            ref_positive_indices.push_back(grounded_literal->get_atom()->get_index());
        }
    }
    std::sort(ref_positive_indices.begin(), ref_positive_indices.end());
    std::sort(ref_negative_indices.begin(), ref_negative_indices.end());
}

template void LiteralGrounder::ground_and_fill_vector(const std::vector<Literal<Static>>& literals,
                                                      FlatIndexList& ref_positive_indices,
                                                      FlatIndexList& ref_negative_indices,
                                                      const ObjectList& binding);
template void LiteralGrounder::ground_and_fill_vector(const std::vector<Literal<Fluent>>& literals,
                                                      FlatIndexList& ref_positive_indices,
                                                      FlatIndexList& ref_negative_indices,
                                                      const ObjectList& binding);
template void LiteralGrounder::ground_and_fill_vector(const std::vector<Literal<Derived>>& literals,
                                                      FlatIndexList& ref_positive_indices,
                                                      FlatIndexList& ref_negative_indices,
                                                      const ObjectList& binding);

Problem LiteralGrounder::get_problem() const { return m_problem; }

const std::shared_ptr<PDDLRepositories>& LiteralGrounder::get_pddl_repositories() const { return m_pddl_repositories; }
}
