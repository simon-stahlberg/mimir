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

#include "mimir/formalism/domain_builder.hpp"

#include "mimir/common/collections.hpp"
#include "mimir/formalism/domain.hpp"

namespace mimir
{
template<typename T>
static void verify_indexing_scheme(const std::vector<const T*>& elements, const std::string& error_message)
{
    for (size_t i = 0; i < elements.size(); ++i)
    {
        if (elements[i]->get_index() != i)
        {
            throw std::runtime_error(error_message);
        }
    }
}

Domain DomainBuilder::get_result()
{
    std::sort(get_constants().begin(), get_constants().end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_constants(), "DomainBuilder::get_result: constants must follow and indexing scheme.");

    std::sort(get_predicates<Static>().begin(), get_predicates<Static>().end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_predicates<Static>(), "DomainBuilder::get_result: static predicates must follow and indexing scheme.");

    std::sort(get_predicates<Fluent>().begin(), get_predicates<Fluent>().end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_predicates<Fluent>(), "DomainBuilder::get_result: fluent predicates must follow and indexing scheme.");

    std::sort(get_predicates<Derived>().begin(), get_predicates<Derived>().end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_predicates<Derived>(), "DomainBuilder::get_result: derived predicates must follow and indexing scheme.");

    std::sort(get_function_skeletons<Static>().begin(),
              get_function_skeletons<Static>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_function_skeletons<Static>(), "DomainBuilder::get_result: functions must follow and indexing scheme.");

    std::sort(get_function_skeletons<Fluent>().begin(),
              get_function_skeletons<Fluent>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_function_skeletons<Fluent>(), "DomainBuilder::get_result: functions must follow and indexing scheme.");

    std::sort(m_actions.begin(), m_actions.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(m_actions, "DomainBuilder::get_result: actions must follow and indexing scheme.");

    std::sort(m_axioms.begin(), m_axioms.end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(m_axioms, "DomainBuilder::get_result: axioms must follow and indexing scheme.");

    m_requirements = (m_requirements) ? m_requirements : m_repositories.get_or_create_requirements(loki::RequirementEnumSet { loki::RequirementEnum::STRIPS });

    return std::shared_ptr<const DomainImpl>(new DomainImpl(std::move(m_repositories),
                                                            std::move(m_filepath),
                                                            std::move(m_name),
                                                            std::move(m_requirements),
                                                            std::move(m_constants),
                                                            std::move(m_predicates),
                                                            std::move(m_function_skeletons),
                                                            std::move(m_auxiliary_function_skeleton),
                                                            std::move(m_actions),
                                                            std::move(m_axioms)));
}

PDDLRepositories& DomainBuilder::get_repositories() { return m_repositories; }
std::optional<fs::path>& DomainBuilder::get_filepath() { return m_filepath; }
std::string& DomainBuilder::get_name() { return m_name; }
Requirements& DomainBuilder::get_requirements() { return m_requirements; }
ObjectList& DomainBuilder::get_constants() { return m_constants; }
template<StaticOrFluentOrDerived P>
PredicateList<P>& DomainBuilder::get_predicates()
{
    return boost::hana::at_key(m_predicates, boost::hana::type<P> {});
}

template PredicateList<Static>& DomainBuilder::get_predicates();
template PredicateList<Fluent>& DomainBuilder::get_predicates();
template PredicateList<Derived>& DomainBuilder::get_predicates();

PredicateLists<Static, Fluent, Derived>& DomainBuilder::get_hana_predicates() { return m_predicates; }

template<StaticOrFluent F>
FunctionSkeletonList<F>& DomainBuilder::get_function_skeletons()
{
    return boost::hana::at_key(m_function_skeletons, boost::hana::type<F> {});
}

template FunctionSkeletonList<Static>& DomainBuilder::get_function_skeletons();
template FunctionSkeletonList<Fluent>& DomainBuilder::get_function_skeletons();

FunctionSkeletonLists<Static, Fluent>& DomainBuilder::get_hana_function_skeletons() { return m_function_skeletons; }

std::optional<FunctionSkeleton<Auxiliary>>& DomainBuilder::get_auxiliary_function_skeleton() { return m_auxiliary_function_skeleton; }
ActionList& DomainBuilder::get_actions() { return m_actions; }
AxiomList& DomainBuilder::get_axioms() { return m_axioms; }
}
