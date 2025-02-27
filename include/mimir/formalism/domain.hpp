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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_DOMAIN_HPP_
#define MIMIR_FORMALISM_DOMAIN_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/domain_details.hpp"
#include "mimir/formalism/repositories.hpp"

namespace mimir
{

class DomainImpl
{
private:
    PDDLRepositories m_repositories;
    std::optional<fs::path> m_filepath;
    std::string m_name;
    Requirements m_requirements;
    ObjectList m_constants;
    PredicateLists<Static, Fluent, Derived> m_predicates;
    FunctionSkeletonLists<Static, Fluent> m_function_skeletons;
    std::optional<FunctionSkeleton<Auxiliary>> m_auxiliary_function_skeleton;
    ActionList m_actions;
    AxiomList m_axioms;

    // Below: add additional members if needed and initialize them in the constructor

    domain::Details m_details;  ///< We hide the details in a struct.

    DomainImpl(PDDLRepositories repositories,
               std::optional<fs::path> filepath,
               std::string name,
               Requirements requirements,
               ObjectList constants,
               PredicateLists<Static, Fluent, Derived> predicates,
               FunctionSkeletonLists<Static, Fluent> m_function_skeletons,
               std::optional<FunctionSkeleton<Auxiliary>> auxiliary_function_skeleton,
               ActionList actions,
               AxiomList axioms);

    // Give access to the constructor.
    friend class DomainBuilder;

public:
    // moveable but not copyable
    DomainImpl(const DomainImpl& other) = delete;
    DomainImpl& operator=(const DomainImpl& other) = delete;
    DomainImpl(DomainImpl&& other) = default;
    DomainImpl& operator=(DomainImpl&& other) = default;

    const PDDLRepositories& get_repositories() const;
    const std::optional<fs::path>& get_filepath() const;
    const std::string& get_name() const;
    const Requirements& get_requirements() const;
    const ObjectList& get_constants() const;
    template<StaticOrFluentOrDerived P>
    const PredicateList<P>& get_predicates() const;
    const PredicateLists<Static, Fluent, Derived>& get_hana_predicates() const;
    template<StaticOrFluent F>
    const FunctionSkeletonList<F>& get_function_skeletons() const;
    const FunctionSkeletonLists<Static, Fluent>& get_hana_function_skeletons() const;
    const std::optional<FunctionSkeleton<Auxiliary>>& get_auxiliary_function_skeleton() const;
    const ActionList& get_actions() const;
    const AxiomList& get_axioms() const;

    /* Additional members */

    const ToObjectMap<std::string> get_name_to_constant() const;
    template<StaticOrFluentOrDerived P>
    const ToPredicateMap<std::string, P>& get_name_to_predicate() const;
    const ToPredicateMaps<std::string, Static, Fluent, Derived>& get_hana_name_to_predicate();

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const
    {
        return std::forward_as_tuple(std::as_const(get_name()),
                                     std::as_const(get_requirements()),
                                     std::as_const(get_constants()),
                                     std::as_const(get_predicates<Static>()),
                                     std::as_const(get_predicates<Fluent>()),
                                     std::as_const(get_predicates<Derived>()),
                                     std::as_const(get_function_skeletons<Static>()),
                                     std::as_const(get_function_skeletons<Fluent>()),
                                     std::as_const(get_auxiliary_function_skeleton()),
                                     std::as_const(get_actions()),
                                     std::as_const(get_axioms()));
    }
};

extern std::ostream& operator<<(std::ostream& out, const DomainImpl& element);

extern std::ostream& operator<<(std::ostream& out, Domain element);

}

#endif
