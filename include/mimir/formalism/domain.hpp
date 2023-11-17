#if !defined(FORMALISM_DOMAIN_HPP_)
#define FORMALISM_DOMAIN_HPP_

#include "action_schema.hpp"
#include "declarations.hpp"
#include "object.hpp"
#include "predicate.hpp"
#include "type.hpp"

#include <map>
#include <string>

namespace formalism
{
    class DomainImpl
    {
      public:
        std::string name;
        formalism::RequirementList requirements;
        formalism::TypeList types;
        formalism::ObjectList constants;
        formalism::PredicateList predicates;
        formalism::PredicateList static_predicates;
        formalism::PredicateList functions;
        formalism::ActionSchemaList action_schemas;

        DomainImpl(const std::string& name,
                   const formalism::RequirementList& requirements,
                   const formalism::TypeList& types,
                   const formalism::ObjectList& constants,
                   const formalism::PredicateList& predicates,
                   const formalism::PredicateList& functions,
                   const formalism::ActionSchemaList& action_schemas);

        std::map<std::string, formalism::Type> get_type_map() const;

        std::map<std::string, formalism::Predicate> get_predicate_name_map() const;

        std::map<std::string, formalism::Predicate> get_function_name_map() const;

        std::map<uint32_t, formalism::Predicate> get_predicate_id_map() const;

        std::map<std::string, formalism::Object> get_constant_map() const;
    };

    DomainDescription create_domain(const std::string& name,
                                    const formalism::RequirementList& requirements,
                                    const formalism::TypeList& types,
                                    const formalism::ObjectList& constants,
                                    const formalism::PredicateList& predicates,
                                    const formalism::PredicateList& functions,
                                    const formalism::ActionSchemaList& action_schemas);

    DomainDescription relax(const formalism::DomainDescription& domain, bool remove_negative_preconditions, bool remove_delete_list);

    std::ostream& operator<<(std::ostream& os, const formalism::DomainDescription& domain);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<formalism::DomainDescription>
    {
        std::size_t operator()(const formalism::DomainDescription& domain) const;
    };

    template<>
    struct less<formalism::DomainDescription>
    {
        bool operator()(const formalism::DomainDescription& left_domain, const formalism::DomainDescription& right_domain) const;
    };

    template<>
    struct equal_to<formalism::DomainDescription>
    {
        bool operator()(const formalism::DomainDescription& left_domain, const formalism::DomainDescription& right_domain) const;
    };

}  // namespace std

#endif  // FORMALISM_DOMAIN_HPP_
