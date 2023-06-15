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


#include "domain.hpp"
#include "help_functions.hpp"

namespace formalism
{
    DomainImpl::DomainImpl(const std::string& name,
                           const formalism::RequirementList& requirements,
                           const formalism::TypeList& types,
                           const formalism::ObjectList& constants,
                           const formalism::PredicateList& predicates,
                           const formalism::ActionSchemaList& action_schemas) :
        name(name),
        requirements(requirements),
        types(types),
        constants(constants),
        predicates(predicates),
        static_predicates(),
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

    std::map<std::string, formalism::Type> DomainImpl::get_type_map() const
    {
        std::map<std::string, formalism::Type> map;

        for (const auto& type : types)
        {
            map.insert(std::make_pair(type->name, type));
        }

        return map;
    }

    std::map<std::string, formalism::Predicate> DomainImpl::get_predicate_map() const
    {
        std::map<std::string, formalism::Predicate> map;

        for (const auto& predicate : predicates)
        {
            map.insert(std::make_pair(predicate->name, predicate));
        }

        return map;
    }

    std::map<std::string, formalism::Object> DomainImpl::get_constant_map() const
    {
        std::map<std::string, formalism::Object> map;

        for (const auto& constant : constants)
        {
            map.insert(std::make_pair(constant->name, constant));
        }

        return map;
    }

    DomainDescription create_domain(const std::string& name,
                                    const formalism::RequirementList& requirements,
                                    const formalism::TypeList& types,
                                    const formalism::ObjectList& constants,
                                    const formalism::PredicateList& predicates,
                                    const formalism::ActionSchemaList& action_schemas)
    {
        return std::make_shared<DomainImpl>(name, requirements, types, constants, predicates, action_schemas);
    }

    std::ostream& operator<<(std::ostream& os, const formalism::DomainDescription& domain)
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
}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<formalism::DomainDescription>::operator()(const formalism::DomainDescription& domain) const
    {
        return hash_combine(domain->name, domain->types, domain->constants, domain->predicates, domain->action_schemas);
    }

    bool less<formalism::DomainDescription>::operator()(const formalism::DomainDescription& left_domain, const formalism::DomainDescription& right_domain) const
    {
        return less_combine(std::make_tuple(left_domain->name, left_domain->constants, left_domain->predicates, left_domain->action_schemas),
                            std::make_tuple(right_domain->name, right_domain->constants, right_domain->predicates, right_domain->action_schemas));
    }

    bool equal_to<formalism::DomainDescription>::operator()(const formalism::DomainDescription& left_domain,
                                                            const formalism::DomainDescription& right_domain) const
    {
        return equal_to_combine(std::make_tuple(left_domain->name, left_domain->constants, left_domain->predicates, left_domain->action_schemas),
                                std::make_tuple(right_domain->name, right_domain->constants, right_domain->predicates, right_domain->action_schemas));
    }
}  // namespace std
