#ifndef MIMIR_FORMALISM_DOMAIN_HPP_
#define MIMIR_FORMALISM_DOMAIN_HPP_

#include "action_schema.hpp"
#include "declarations.hpp"
#include "object.hpp"
#include "predicate.hpp"
#include "type.hpp"

#include <map>
#include <string>

namespace mimir::formalism
{
    class DomainImpl
    {
      public:
        std::string name;
        mimir::formalism::RequirementList requirements;
        mimir::formalism::TypeList types;
        mimir::formalism::ObjectList constants;
        mimir::formalism::PredicateList predicates;
        mimir::formalism::PredicateList static_predicates;
        mimir::formalism::PredicateList functions;
        mimir::formalism::ActionSchemaList action_schemas;

        DomainImpl(const std::string& name,
                   const mimir::formalism::RequirementList& requirements,
                   const mimir::formalism::TypeList& types,
                   const mimir::formalism::ObjectList& constants,
                   const mimir::formalism::PredicateList& predicates,
                   const mimir::formalism::PredicateList& functions,
                   const mimir::formalism::ActionSchemaList& action_schemas);

        std::map<std::string, mimir::formalism::Type> get_type_map() const;

        std::map<std::string, mimir::formalism::Predicate> get_predicate_name_map() const;

        std::map<std::string, mimir::formalism::Predicate> get_function_name_map() const;

        std::map<uint32_t, mimir::formalism::Predicate> get_predicate_id_map() const;

        std::map<std::string, mimir::formalism::Object> get_constant_map() const;
    };

    DomainDescription create_domain(const std::string& name,
                                    const mimir::formalism::RequirementList& requirements,
                                    const mimir::formalism::TypeList& types,
                                    const mimir::formalism::ObjectList& constants,
                                    const mimir::formalism::PredicateList& predicates,
                                    const mimir::formalism::PredicateList& functions,
                                    const mimir::formalism::ActionSchemaList& action_schemas);

    DomainDescription relax(const mimir::formalism::DomainDescription& domain, bool remove_negative_preconditions, bool remove_delete_list);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::DomainDescription& domain);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::DomainDescription>
    {
        std::size_t operator()(const mimir::formalism::DomainDescription& domain) const;
    };

    template<>
    struct less<mimir::formalism::DomainDescription>
    {
        bool operator()(const mimir::formalism::DomainDescription& left_domain, const mimir::formalism::DomainDescription& right_domain) const;
    };

    template<>
    struct equal_to<mimir::formalism::DomainDescription>
    {
        bool operator()(const mimir::formalism::DomainDescription& left_domain, const mimir::formalism::DomainDescription& right_domain) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_DOMAIN_HPP_
