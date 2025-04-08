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
#include <fmt/core.h>
#include <iostream>
#include <unordered_map>

using namespace std;

namespace mimir::formalism
{
DomainImpl::DomainImpl(Repositories repositories,
                       std::optional<fs::path> filepath,
                       std::string name,
                       Requirements requirements,
                       ObjectList constants,
                       PredicateLists<StaticTag, FluentTag, DerivedTag> predicates,
                       FunctionSkeletonLists<StaticTag, FluentTag> function_skeletons,
                       std::optional<FunctionSkeleton<AuxiliaryTag>> auxiliary_function_skeleton,
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
    m_axioms(std::move(axioms))
{
    assert(is_all_unique(get_constants()));
    assert(is_all_unique(get_predicates<StaticTag>()));
    assert(is_all_unique(get_predicates<FluentTag>()));
    assert(is_all_unique(get_predicates<DerivedTag>()));
    assert(is_all_unique(get_function_skeletons<StaticTag>()));
    assert(is_all_unique(get_function_skeletons<FluentTag>()));
    assert(is_all_unique(get_actions()));
    assert(is_all_unique(get_axioms()));
    assert(std::is_sorted(get_constants().begin(), get_constants().end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_predicates<StaticTag>().begin(),
                          get_predicates<StaticTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_predicates<FluentTag>().begin(),
                          get_predicates<FluentTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_predicates<DerivedTag>().begin(),
                          get_predicates<DerivedTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_function_skeletons<StaticTag>().begin(),
                          get_function_skeletons<StaticTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_function_skeletons<FluentTag>().begin(),
                          get_function_skeletons<FluentTag>().end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_actions().begin(), get_actions().end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
    assert(std::is_sorted(get_axioms().begin(), get_axioms().end(), [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));

    /**
     * Details
     */

    m_details = domain::Details(*this);
}

const Repositories& DomainImpl::get_repositories() const { return m_repositories; }

const std::optional<fs::path>& DomainImpl::get_filepath() const { return m_filepath; }

const std::string& DomainImpl::get_name() const { return m_name; }

Requirements DomainImpl::get_requirements() const { return m_requirements; }

const ObjectList& DomainImpl::get_constants() const { return m_constants; }

template<IsStaticOrFluentOrDerivedTag P>
const PredicateList<P>& DomainImpl::get_predicates() const
{
    return boost::hana::at_key(m_predicates, boost::hana::type<P> {});
}

template const PredicateList<StaticTag>& DomainImpl::get_predicates<StaticTag>() const;
template const PredicateList<FluentTag>& DomainImpl::get_predicates<FluentTag>() const;
template const PredicateList<DerivedTag>& DomainImpl::get_predicates<DerivedTag>() const;

const PredicateLists<StaticTag, FluentTag, DerivedTag>& DomainImpl::get_hana_predicates() const { return m_predicates; }

template<IsStaticOrFluentTag F>
const FunctionSkeletonList<F>& DomainImpl::get_function_skeletons() const
{
    return boost::hana::at_key(m_function_skeletons, boost::hana::type<F> {});
}

template const FunctionSkeletonList<StaticTag>& DomainImpl::get_function_skeletons<StaticTag>() const;
template const FunctionSkeletonList<FluentTag>& DomainImpl::get_function_skeletons<FluentTag>() const;

const FunctionSkeletonLists<StaticTag, FluentTag>& DomainImpl::get_hana_function_skeletons() const { return m_function_skeletons; }

const std::optional<FunctionSkeleton<AuxiliaryTag>>& DomainImpl::get_auxiliary_function_skeleton() const { return m_auxiliary_function_skeleton; }

const ActionList& DomainImpl::get_actions() const { return m_actions; }

const AxiomList& DomainImpl::get_axioms() const { return m_axioms; }

const ToObjectMap<std::string> DomainImpl::get_name_to_constant() const { return m_details.constant.name_to_constant; }

template<IsStaticOrFluentOrDerivedTag P>
const ToPredicateMap<std::string, P>& DomainImpl::get_name_to_predicate() const
{
    return boost::hana::at_key(m_details.predicate.name_to_predicate, boost::hana::type<P> {});
}

template const ToPredicateMap<std::string, StaticTag>& DomainImpl::get_name_to_predicate<StaticTag>() const;
template const ToPredicateMap<std::string, FluentTag>& DomainImpl::get_name_to_predicate<FluentTag>() const;
template const ToPredicateMap<std::string, DerivedTag>& DomainImpl::get_name_to_predicate<DerivedTag>() const;

const ToPredicateMaps<std::string, StaticTag, FluentTag, DerivedTag>& DomainImpl::get_hana_name_to_predicate() { return m_details.predicate.name_to_predicate; }

/**
 * Details
 */

domain::ConstantDetails::ConstantDetails() : parent(nullptr), name_to_constant() {}

domain::ConstantDetails::ConstantDetails(const DomainImpl& domain) : parent(&domain), name_to_constant()
{
    for (const auto& object : domain.get_constants())
    {
        name_to_constant.emplace(object->get_name(), object);
    }
}

domain::PredicateDetails::PredicateDetails() : parent(nullptr), name_to_predicate() {}

domain::PredicateDetails::PredicateDetails(const DomainImpl& domain) : parent(&domain), name_to_predicate()
{
    boost::hana::for_each(domain.get_hana_predicates(),
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& value = boost::hana::second(pair);
                              for (const auto& predicate : value)
                              {
                                  boost::hana::at_key(name_to_predicate, key).emplace(predicate->get_name(), predicate);
                              }
                          });
}

domain::Details::Details() : parent(nullptr), constant(), predicate() {}

domain::Details::Details(const DomainImpl& domain) : parent(&domain), constant(domain), predicate(domain) {}

/**
 * Printing
 */

std::ostream& operator<<(std::ostream& out, const DomainImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Domain element)
{
    write(*element, AddressFormatter(), out);
    return out;
}
}
