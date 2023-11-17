#ifndef MIMIR_FORMALISM_PREDICATE_HPP_
#define MIMIR_FORMALISM_PREDICATE_HPP_

#include "declarations.hpp"

#include <string>

namespace mimir::formalism
{
    class PredicateImpl
    {
      private:
        std::size_t hash_;

      public:
        const uint32_t id;
        const std::string name;
        const mimir::formalism::ObjectList parameters;
        const uint32_t arity;

        PredicateImpl(const uint32_t id, const std::string& name, const mimir::formalism::ObjectList& parameters);

        template<typename T>
        friend class std::hash;
    };

    Predicate create_predicate(const uint32_t id, const std::string& name, const mimir::formalism::ObjectList& parameters);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Predicate& predicate);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::PredicateList& predicates);

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
