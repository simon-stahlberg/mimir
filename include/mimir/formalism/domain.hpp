#ifndef MIMIR_FORMALISM_DOMAIN_HPP_
#define MIMIR_FORMALISM_DOMAIN_HPP_

#include "action_schema.hpp"
#include "declarations.hpp"
#include "object.hpp"
#include "predicate.hpp"
#include "type.hpp"

#include <loki/domain/pddl/domain.hpp>
#include <map>
#include <string>

namespace mimir::formalism
{
    class Domain
    {
      private:
        loki::pddl::Domain external_;

      public:
        explicit Domain(loki::pddl::Domain external_domain);

        std::map<std::string, Type> get_type_map() const;

        std::map<std::string, Predicate> get_predicate_name_map() const;

        std::map<std::string, Predicate> get_function_name_map() const;

        std::map<uint32_t, Predicate> get_predicate_id_map() const;

        std::map<std::string, Object> get_constant_map() const;

        friend std::ostream& operator<<(std::ostream& os, const Domain& domain);
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
