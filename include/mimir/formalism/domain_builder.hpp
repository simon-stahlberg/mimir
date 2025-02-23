/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_DOMAIN_BUILDER_HPP_
#define MIMIR_FORMALISM_DOMAIN_BUILDER_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/repositories.hpp"

#include <optional>
#include <string>

namespace mimir
{

class DomainBuilder
{
private:
    PDDLRepositories m_repositories;

    std::optional<fs::path> m_filepath;
    std::string m_name;
    Requirements m_requirements;
    ObjectList m_constants;
    PredicateList<Static> m_static_predicates;
    PredicateList<Fluent> m_fluent_predicates;
    PredicateList<Derived> m_derived_predicates;
    FunctionSkeletonList<Static> m_static_function_skeletons;
    FunctionSkeletonList<Fluent> m_fluent_function_skeletons;
    std::optional<FunctionSkeleton<Auxiliary>> m_auxiliary_function_skeleton;
    ActionList m_actions;
    AxiomList m_axioms;

public:
    /// @brief Create an empty builder.
    DomainBuilder() = default;

    /// @brief Finalizes the `Domain` and returns it.
    /// The `DomainBuilder` is in an invalid state afterwards.
    /// @return the resulting `Domain`.
    Domain get_result();

    /**
     * Get and modify components of the domain.
     */

    PDDLRepositories& get_repositories();

    std::optional<fs::path>& get_filepath();
    std::string& get_name();
    Requirements& get_requirements();
    ObjectList& get_constants();
    template<StaticOrFluentOrDerived P>
    PredicateList<P>& get_predicates();
    template<StaticOrFluentTag F>
    FunctionSkeletonList<F>& get_function_skeletons();
    std::optional<FunctionSkeleton<Auxiliary>>& get_auxiliary_function_skeleton();
    ActionList& get_actions();
    AxiomList& get_axioms();
};
}

#endif
