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

#include "mimir/formalism/domain.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/requirements.hpp"

#include <cassert>
#include <iostream>
#include <unordered_map>

using namespace std;

namespace mimir
{
DomainImpl::DomainImpl(PDDLRepositories repositories,
                       std::optional<fs::path> filepath,
                       std::string name,
                       Requirements requirements,
                       ObjectList constants,
                       PredicateLists<Static, Fluent, Derived> predicates,
                       FunctionSkeletonLists<Static, Fluent> function_skeletons,
                       std::optional<FunctionSkeleton<Auxiliary>> auxiliary_function_skeleton,
                       ActionList actions,
                       AxiomList axioms) :
    m_repositories(std::move(repositories)),
    m_filepath(std::move(filepath)),
    m_name(std::move(name)),
    m_requirements(std::move(requirements)),
    m_constants(std::move(constants)),
    m_predicates(std::move(predicates)),
    m_function_skeletons(std::move(function_skeletons)),
    m_auxiliary_function_skeleton(std::move(auxiliary_function_skeleton)),
    m_actions(std::move(actions)),
    m_axioms(std::move(axioms)),
    m_name_to_constant(),
    m_name_to_predicate()
{
    assert(is_all_unique(get_constants()));
    assert(is_all_unique(get_predicates<Static>()));
    assert(is_all_unique(get_predicates<Fluent>()));
    assert(is_all_unique(get_predicates<Derived>()));
    assert(is_all_unique(get_function_skeletons<Static>()));
    assert(is_all_unique(get_function_skeletons<Fluent>()));
    assert(is_all_unique(get_actions()));
    assert(is_all_unique(get_axioms()));
    assert(std::is_sorted(get_constants().begin(), get_constants().end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_predicates<Static>().begin(),
                          get_predicates<Static>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_predicates<Fluent>().begin(),
                          get_predicates<Fluent>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_predicates<Derived>().begin(),
                          get_predicates<Derived>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_function_skeletons<Static>().begin(),
                          get_function_skeletons<Static>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_function_skeletons<Fluent>().begin(),
                          get_function_skeletons<Fluent>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_actions().begin(), get_actions().end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_axioms().begin(), get_axioms().end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));

    /* Additional */
    for (const auto& object : get_constants())
    {
        m_name_to_constant.emplace(object->get_name(), object);
    }

    boost::hana::for_each(get_hana_predicates(),
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& value = boost::hana::second(pair);
                              for (const auto& predicate : value)
                              {
                                  boost::hana::at_key(m_name_to_predicate, key).emplace(predicate->get_name(), predicate);
                              }
                          });
}

const PDDLRepositories& DomainImpl::get_repositories() const { return m_repositories; }

const std::optional<fs::path>& DomainImpl::get_filepath() const { return m_filepath; }

const std::string& DomainImpl::get_name() const { return m_name; }

const Requirements& DomainImpl::get_requirements() const { return m_requirements; }

const ObjectList& DomainImpl::get_constants() const { return m_constants; }

template<StaticOrFluentOrDerived P>
const PredicateList<P>& DomainImpl::get_predicates() const
{
    return boost::hana::at_key(m_predicates, boost::hana::type<P> {});
}

template const PredicateList<Static>& DomainImpl::get_predicates<Static>() const;
template const PredicateList<Fluent>& DomainImpl::get_predicates<Fluent>() const;
template const PredicateList<Derived>& DomainImpl::get_predicates<Derived>() const;

const PredicateLists<Static, Fluent, Derived>& DomainImpl::get_hana_predicates() const { return m_predicates; }

template<StaticOrFluent F>
const FunctionSkeletonList<F>& DomainImpl::get_function_skeletons() const
{
    return boost::hana::at_key(m_function_skeletons, boost::hana::type<F> {});
}

template const FunctionSkeletonList<Static>& DomainImpl::get_function_skeletons<Static>() const;
template const FunctionSkeletonList<Fluent>& DomainImpl::get_function_skeletons<Fluent>() const;

const FunctionSkeletonLists<Static, Fluent>& DomainImpl::get_hana_function_skeletons() const { return m_function_skeletons; }

const std::optional<FunctionSkeleton<Auxiliary>>& DomainImpl::get_auxiliary_function_skeleton() const { return m_auxiliary_function_skeleton; }

const ActionList& DomainImpl::get_actions() const { return m_actions; }

const AxiomList& DomainImpl::get_axioms() const { return m_axioms; }

const ToObjectMap<std::string> DomainImpl::get_name_to_constant() const { return m_name_to_constant; }

template<StaticOrFluentOrDerived P>
const ToPredicateMap<std::string, P>& DomainImpl::get_name_to_predicate() const
{
    return boost::hana::at_key(m_name_to_predicate, boost::hana::type<P> {});
}

template const ToPredicateMap<std::string, Static>& DomainImpl::get_name_to_predicate<Static>() const;
template const ToPredicateMap<std::string, Fluent>& DomainImpl::get_name_to_predicate<Fluent>() const;
template const ToPredicateMap<std::string, Derived>& DomainImpl::get_name_to_predicate<Derived>() const;

const ToPredicateMaps<std::string, Static, Fluent, Derived>& DomainImpl::get_hana_name_to_predicate() { return m_name_to_predicate; }

std::ostream& operator<<(std::ostream& out, const DomainImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Domain element)
{
    out << *element;
    return out;
}

}
