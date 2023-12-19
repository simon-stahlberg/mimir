#ifndef MIMIR_FORMALISM_LITERAL_HPP_
#define MIMIR_FORMALISM_LITERAL_HPP_

#include "atom.hpp"
#include "declarations.hpp"

#include <loki/domain/pddl/literal.hpp>
#include <memory>
#include <vector>

namespace mimir::formalism
{
    class Literal
    {
      private:
        loki::pddl::Literal external_;

      public:
        explicit Literal(loki::pddl::Literal external_literal);

        explicit Literal(Atom atom, bool negated);

        Literal ground_literal(const ParameterAssignment& assignment) const;

        Atom as_atom() const;

        bool contains_predicate(const Predicate& predicate);

        std::size_t hash() const;

        friend std::ostream& operator<<(std::ostream& os, const Literal& literal);
    };

    // bool contains_predicate(const mimir::formalism::LiteralList& literals, const mimir::formalism::Predicate& predicate);

    // std::ostream& operator<<(std::ostream& os, const mimir::formalism::LiteralList& literals);

}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::Literal>
    {
        std::size_t operator()(const mimir::formalism::Literal& literal) const;
    };

    template<>
    struct hash<mimir::formalism::LiteralList>
    {
        std::size_t operator()(const mimir::formalism::LiteralList& literals) const;
    };

    template<>
    struct less<mimir::formalism::Literal>
    {
        bool operator()(const mimir::formalism::Literal& left_literal, const mimir::formalism::Literal& right_literal) const;
    };

    template<>
    struct equal_to<mimir::formalism::Literal>
    {
        bool operator()(const mimir::formalism::Literal& left_literal, const mimir::formalism::Literal& right_literal) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_LITERAL_HPP_
