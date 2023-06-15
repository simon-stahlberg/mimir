#if !defined(FORMALISM_PREDICATE_HPP_)
#define FORMALISM_PREDICATE_HPP_

#include "declarations.hpp"

#include <string>

namespace formalism
{
    class PredicateImpl
    {
      private:
        std::size_t hash_;

      public:
        const uint32_t id;
        const std::string name;
        const formalism::ObjectList parameters;
        const uint32_t arity;

        PredicateImpl(const uint32_t id, const std::string& name, const formalism::ObjectList& parameters);

        template<typename T>
        friend class std::hash;
    };

    Predicate create_predicate(const uint32_t id, const std::string& name, const formalism::ObjectList& parameters);

    std::ostream& operator<<(std::ostream& os, const formalism::Predicate& predicate);

    std::ostream& operator<<(std::ostream& os, const formalism::PredicateList& predicates);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<formalism::Predicate>
    {
        std::size_t operator()(const formalism::Predicate& predicate) const;
    };

    template<>
    struct hash<formalism::PredicateList>
    {
        std::size_t operator()(const formalism::PredicateList& predicates) const;
    };

    template<>
    struct less<formalism::Predicate>
    {
        bool operator()(const formalism::Predicate& left_predicate, const formalism::Predicate& right_predicate) const;
    };

    template<>
    struct equal_to<formalism::Predicate>
    {
        bool operator()(const formalism::Predicate& left_predicate, const formalism::Predicate& right_predicate) const;
    };

}  // namespace std

#endif  // FORMALISM_PREDICATE_HPP_
