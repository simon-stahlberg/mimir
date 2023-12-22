#ifndef MIMIR_FORMALISM_DOMAIN_HPP_
#define MIMIR_FORMALISM_DOMAIN_HPP_

#include "declarations.hpp"

#include <loki/domain/pddl/domain.hpp>
#include <map>
#include <string>
#include <vector>

namespace mimir::formalism
{
    class Domain
    {
      private:
        loki::pddl::Domain external_;

        explicit Domain(loki::pddl::Domain external_domain);

      public:
        static Domain parse(const std::string& path);

        const std::string& get_name() const;

        TypeList get_types() const;
        TermList get_constants() const;
        PredicateList get_predicates() const;
        PredicateList get_static_predicates() const;
        ActionSchemaList get_action_schemas() const;
        std::map<std::string, Type> get_type_map() const;
        std::map<std::string, Predicate> get_predicate_name_map() const;
        std::map<std::string, Predicate> get_function_name_map() const;
        std::map<uint32_t, Predicate> get_predicate_id_map() const;
        std::map<std::string, Term> get_constant_map() const;
        // const Requirements& get_requirements() const;
        // const FunctionSkeletonList& get_functions() const;

        bool operator<(const Domain& other) const;
        bool operator>(const Domain& other) const;
        bool operator==(const Domain& other) const;
        bool operator!=(const Domain& other) const;
        bool operator<=(const Domain& other) const;

        friend std::ostream& operator<<(std::ostream& os, const Domain& domain);
        friend class Problem;
    };

}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::Domain>
    {
        std::size_t operator()(const mimir::formalism::Domain& domain) const;
    };

    template<>
    struct less<mimir::formalism::Domain>
    {
        bool operator()(const mimir::formalism::Domain& left_domain, const mimir::formalism::Domain& right_domain) const;
    };

    template<>
    struct equal_to<mimir::formalism::Domain>
    {
        bool operator()(const mimir::formalism::Domain& left_domain, const mimir::formalism::Domain& right_domain) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_DOMAIN_HPP_
