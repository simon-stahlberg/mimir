#ifndef MIMIR_FORMALISM_PREDICATE_HPP_
#define MIMIR_FORMALISM_PREDICATE_HPP_

#include "term.hpp"

#include <loki/domain/pddl/predicate.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace mimir::formalism
{
    class Predicate
    {
      private:
        loki::pddl::Predicate external_;

        explicit Predicate(loki::pddl::Predicate external_predicate);

      public:
        uint32_t get_id() const;
        const std::string& get_name() const;
        TermList get_parameters() const;
        std::size_t get_arity() const;

        std::size_t hash() const;

        bool operator<(const Predicate& other) const;
        bool operator>(const Predicate& other) const;
        bool operator==(const Predicate& other) const;
        bool operator!=(const Predicate& other) const;
        bool operator<=(const Predicate& other) const;

        friend std::ostream& operator<<(std::ostream& os, const Predicate& predicate);
        friend class Atom;
        friend class Domain;
        friend class Literal;
    };

    // Predicate create_predicate(const uint32_t id, const std::string& name, const ObjectList& parameters);

    using PredicateList = std::vector<Predicate>;
    using PredicateSet = std::unordered_set<Predicate>;

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::Predicate>
    {
        std::size_t operator()(const mimir::formalism::Predicate& predicate) const;
    };

    template<>
    struct hash<mimir::formalism::PredicateList>
    {
        std::size_t operator()(const mimir::formalism::PredicateList& predicates) const;
    };

    template<>
    struct less<mimir::formalism::Predicate>
    {
        bool operator()(const mimir::formalism::Predicate& left_predicate, const mimir::formalism::Predicate& right_predicate) const;
    };

    template<>
    struct equal_to<mimir::formalism::Predicate>
    {
        bool operator()(const mimir::formalism::Predicate& left_predicate, const mimir::formalism::Predicate& right_predicate) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_PREDICATE_HPP_
