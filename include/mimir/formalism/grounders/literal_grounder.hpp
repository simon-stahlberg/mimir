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

#ifndef MIMIR_FORMALISM_GROUNDERS_LITERAL_GROUNDER_HPP_
#define MIMIR_FORMALISM_GROUNDERS_LITERAL_GROUNDER_HPP_

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/grounders/grounding_table.hpp"

namespace mimir
{
class LiteralGrounder
{
private:
    Problem m_problem;
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;

    template<typename T>
    using LiteralGroundingTable = absl::flat_hash_map<ObjectList, T, loki::Hash<ObjectList>>;

    // We a table for each pair (is_negated,predicate_index) since those are context independent.
    template<typename T>
    using LiteralGroundingTableList = std::array<std::vector<GroundingTable<T>>, 2>;

    using GroundedTypeToGroundingTableList =
        boost::hana::map<boost::hana::pair<boost::hana::type<GroundLiteral<Static>>, LiteralGroundingTableList<GroundLiteral<Static>>>,
                         boost::hana::pair<boost::hana::type<GroundLiteral<Fluent>>, LiteralGroundingTableList<GroundLiteral<Fluent>>>,
                         boost::hana::pair<boost::hana::type<GroundLiteral<Derived>>, LiteralGroundingTableList<GroundLiteral<Derived>>>>;

    GroundedTypeToGroundingTableList m_grounding_tables;

public:
    /// @brief Simplest construction
    LiteralGrounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories);

    // Uncopyable
    LiteralGrounder(const LiteralGrounder& other) = delete;
    LiteralGrounder& operator=(const LiteralGrounder& other) = delete;
    // Moveable
    LiteralGrounder(LiteralGrounder&& other) = default;
    LiteralGrounder& operator=(LiteralGrounder&& other) = default;

    template<StaticOrFluentOrDerived P>
    GroundLiteral<P> ground(Literal<P> literal, const ObjectList& binding);

    template<StaticOrFluentOrDerived P>
    void ground_and_fill_bitset(const std::vector<Literal<P>>& literals,
                                FlatBitset& ref_positive_bitset,
                                FlatBitset& ref_negative_bitset,
                                const ObjectList& binding);

    template<StaticOrFluentOrDerived P>
    void ground_and_fill_vector(const std::vector<Literal<P>>& literals,
                                FlatIndexList& ref_positive_indices,
                                FlatIndexList& ref_negative_indices,
                                const ObjectList& binding);

    /**
     * Getters
     */

    Problem get_problem() const;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;
};

}  // namespace mimir

#endif
