#ifndef MIMIR_FORMALISM_PREDICATE_HPP_
#define MIMIR_FORMALISM_PREDICATE_HPP_

#include "term.hpp"

#include "../common/mixins.hpp"

#include <loki/domain/pddl/predicate.hpp>

#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace mimir
{
    class Predicate : public FormattingMixin<Predicate>
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

        friend class Atom;
        friend class Domain;
        friend class Literal;
    };

    // Predicate create_predicate(const uint32_t id, const std::string& name, const ObjectList& parameters);

    using PredicateList = std::vector<Predicate>;
    using PredicateSet = std::unordered_set<Predicate>;

}  // namespace formalism

/*
namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::Predicate>
    {
        std::size_t operator()(const mimir::Predicate& predicate) const;
    };

    template<>
    struct hash<mimir::PredicateList>
    {
        std::size_t operator()(const mimir::PredicateList& predicates) const;
    };

    template<>
    struct less<mimir::Predicate>
    {
        bool operator()(const mimir::Predicate& left_predicate, const mimir::Predicate& right_predicate) const;
    };

    template<>
    struct equal_to<mimir::Predicate>
    {
        bool operator()(const mimir::Predicate& left_predicate, const mimir::Predicate& right_predicate) const;
    };

}  // namespace std
*/

#endif  // MIMIR_FORMALISM_PREDICATE_HPP_
