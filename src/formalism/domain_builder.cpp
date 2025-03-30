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

namespace mimir::formalism
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

    std::sort(get_predicates<StaticTag>().begin(),
              get_predicates<StaticTag>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_predicates<StaticTag>(), "DomainBuilder::get_result: static predicates must follow and indexing scheme.");

    std::sort(get_predicates<FluentTag>().begin(),
              get_predicates<FluentTag>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_predicates<FluentTag>(), "DomainBuilder::get_result: fluent predicates must follow and indexing scheme.");

    std::sort(get_predicates<DerivedTag>().begin(),
              get_predicates<DerivedTag>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_predicates<DerivedTag>(), "DomainBuilder::get_result: derived predicates must follow and indexing scheme.");

    std::sort(get_function_skeletons<StaticTag>().begin(),
              get_function_skeletons<StaticTag>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_function_skeletons<StaticTag>(), "DomainBuilder::get_result: functions must follow and indexing scheme.");

    std::sort(get_function_skeletons<FluentTag>().begin(),
              get_function_skeletons<FluentTag>().end(),
              [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_function_skeletons<FluentTag>(), "DomainBuilder::get_result: functions must follow and indexing scheme.");

    std::sort(get_actions().begin(), get_actions().end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_actions(), "DomainBuilder::get_result: actions must follow and indexing scheme.");

    std::sort(get_axioms().begin(), get_axioms().end(), [](auto&& lhs, auto&& rhs) { return lhs->get_index() < rhs->get_index(); });
    verify_indexing_scheme(get_axioms(), "DomainBuilder::get_result: axioms must follow and indexing scheme.");

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

Repositories& DomainBuilder::get_repositories() { return m_repositories; }
std::optional<fs::path>& DomainBuilder::get_filepath() { return m_filepath; }
std::string& DomainBuilder::get_name() { return m_name; }
Requirements& DomainBuilder::get_requirements() { return m_requirements; }
ObjectList& DomainBuilder::get_constants() { return m_constants; }
template<IsStaticOrFluentOrDerivedTag P>
PredicateList<P>& DomainBuilder::get_predicates()
{
    return boost::hana::at_key(m_predicates, boost::hana::type<P> {});
}

template PredicateList<StaticTag>& DomainBuilder::get_predicates();
template PredicateList<FluentTag>& DomainBuilder::get_predicates();
template PredicateList<DerivedTag>& DomainBuilder::get_predicates();

PredicateLists<StaticTag, FluentTag, DerivedTag>& DomainBuilder::get_hana_predicates() { return m_predicates; }

template<IsStaticOrFluentTag F>
FunctionSkeletonList<F>& DomainBuilder::get_function_skeletons()
{
    return boost::hana::at_key(m_function_skeletons, boost::hana::type<F> {});
}

template FunctionSkeletonList<StaticTag>& DomainBuilder::get_function_skeletons();
template FunctionSkeletonList<FluentTag>& DomainBuilder::get_function_skeletons();

FunctionSkeletonLists<StaticTag, FluentTag>& DomainBuilder::get_hana_function_skeletons() { return m_function_skeletons; }

std::optional<FunctionSkeleton<AuxiliaryTag>>& DomainBuilder::get_auxiliary_function_skeleton() { return m_auxiliary_function_skeleton; }
ActionList& DomainBuilder::get_actions() { return m_actions; }
AxiomList& DomainBuilder::get_axioms() { return m_axioms; }
}
