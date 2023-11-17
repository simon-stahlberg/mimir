/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "../../include/mimir/formalism/domain.hpp"
#include "help_functions.hpp"

#include <algorithm>

namespace mimir::formalism
{
    DomainImpl::DomainImpl(const std::string& name,
                           const mimir::formalism::RequirementList& requirements,
                           const mimir::formalism::TypeList& types,
                           const mimir::formalism::ObjectList& constants,
                           const mimir::formalism::PredicateList& predicates,
                           const mimir::formalism::PredicateList& functions,
                           const mimir::formalism::ActionSchemaList& action_schemas) :
        name(name),
        requirements(requirements),
        types(types),
        constants(constants),
        predicates(predicates),
        static_predicates(),
        functions(functions),
        action_schemas(action_schemas)
    {
        for (const auto& predicate : predicates)
        {
            if (!affect_predicate(action_schemas, predicate))
            {
                static_predicates.push_back(predicate);
            }
        }
    }

    std::map<std::string, mimir::formalism::Type> DomainImpl::get_type_map() const
    {
        std::map<std::string, mimir::formalism::Type> map;

        for (const auto& type : types)
        {
            map.insert(std::make_pair(type->name, type));
        }

        return map;
    }

    std::map<std::string, mimir::formalism::Predicate> DomainImpl::get_predicate_name_map() const
    {
        std::map<std::string, mimir::formalism::Predicate> map;

        for (const auto& predicate : predicates)
        {
            map.insert(std::make_pair(predicate->name, predicate));
        }

        return map;
    }

    std::map<std::string, mimir::formalism::Predicate> DomainImpl::get_function_name_map() const
    {
        std::map<std::string, mimir::formalism::Predicate> map;

        for (const auto& predicate : functions)
        {
            map.insert(std::make_pair(predicate->name, predicate));
        }

        return map;
    }

    std::map<uint32_t, mimir::formalism::Predicate> DomainImpl::get_predicate_id_map() const
    {
        std::map<uint32_t, mimir::formalism::Predicate> map;

        for (const auto& predicate : predicates)
        {
            map.insert(std::make_pair(predicate->id, predicate));
        }

        return map;
    }

    std::map<std::string, mimir::formalism::Object> DomainImpl::get_constant_map() const
    {
        std::map<std::string, mimir::formalism::Object> map;

        for (const auto& constant : constants)
        {
            map.insert(std::make_pair(constant->name, constant));
        }

        return map;
    }

    DomainDescription create_domain(const std::string& name,
                                    const mimir::formalism::RequirementList& requirements,
                                    const mimir::formalism::TypeList& types,
                                    const mimir::formalism::ObjectList& constants,
                                    const mimir::formalism::PredicateList& predicates,
                                    const mimir::formalism::PredicateList& functions,
                                    const mimir::formalism::ActionSchemaList& action_schemas)
    {
        return std::make_shared<DomainImpl>(name, requirements, types, constants, predicates, functions, action_schemas);
    }

    DomainDescription relax(const mimir::formalism::DomainDescription& domain, bool remove_negative_preconditions, bool remove_delete_list)
    {
        mimir::formalism::ActionSchemaList relaxed_action_schemas;
        relaxed_action_schemas.reserve(domain->action_schemas.size());
        std::transform(domain->action_schemas.cbegin(),
                       domain->action_schemas.cend(),
                       std::back_insert_iterator(relaxed_action_schemas),
                       [&](const mimir::formalism::ActionSchema& action_schema)
                       { return mimir::formalism::relax(action_schema, remove_negative_preconditions, remove_delete_list); });

        return create_domain(domain->name,
                             domain->requirements,
                             domain->types,
                             domain->constants,
                             domain->predicates,
                             domain->functions,
                             relaxed_action_schemas);
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::DomainDescription& domain)
    {
        os << "Domain: " << domain->name << std::endl;
        os << "Requirements: ";
        print_vector(os, domain->requirements);
        os << std::endl;
        os << "Types: ";
        print_vector(os, domain->types);
        os << std::endl;
        os << "Constants: ";
        print_vector(os, domain->constants);
        os << std::endl;
        os << "Predicates: ";
        print_vector(os, domain->predicates);
        os << std::endl;
        os << "Action Schemas: ";
        print_vector(os, domain->action_schemas);
        os << std::endl;
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::DomainDescription>::operator()(const mimir::formalism::DomainDescription& domain) const
    {
        return hash_combine(domain->name, domain->types, domain->constants, domain->predicates, domain->action_schemas);
    }

    bool less<mimir::formalism::DomainDescription>::operator()(const mimir::formalism::DomainDescription& left_domain,
                                                               const mimir::formalism::DomainDescription& right_domain) const
    {
        return less_combine(std::make_tuple(left_domain->name, left_domain->constants, left_domain->predicates, left_domain->action_schemas),
                            std::make_tuple(right_domain->name, right_domain->constants, right_domain->predicates, right_domain->action_schemas));
    }

    bool equal_to<mimir::formalism::DomainDescription>::operator()(const mimir::formalism::DomainDescription& left_domain,
                                                                   const mimir::formalism::DomainDescription& right_domain) const
    {
        return equal_to_combine(std::make_tuple(left_domain->name, left_domain->constants, left_domain->predicates, left_domain->action_schemas),
                                std::make_tuple(right_domain->name, right_domain->constants, right_domain->predicates, right_domain->action_schemas));
    }
}  // namespace std
