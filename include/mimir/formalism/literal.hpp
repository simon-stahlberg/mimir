#ifndef MIMIR_FORMALISM_LITERAL_HPP_
#define MIMIR_FORMALISM_LITERAL_HPP_

#include "atom.hpp"
#include "declarations.hpp"

#include <memory>
#include <vector>

namespace mimir::formalism
{
    class LiteralImpl
    {
      private:
        std::size_t hash_;

      public:
        const Atom atom;
        const bool negated;

        LiteralImpl(const Atom& atom, const bool negated);

        template<typename T>
        friend class std::hash;
    };

    mimir::formalism::Literal create_literal(const Atom& atom, const bool negated);

    mimir::formalism::Literal ground_literal(const mimir::formalism::Literal& literal, const mimir::formalism::ParameterAssignment& assignment);

    mimir::formalism::LiteralList ground_literal_list(const mimir::formalism::LiteralList& literal_list,
                                                      const mimir::formalism::ParameterAssignment& assignment);

    mimir::formalism::AtomList as_atoms(const mimir::formalism::LiteralList& literals);

    bool contains_predicate(const mimir::formalism::LiteralList& literals, const mimir::formalism::Predicate& predicate);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Literal& literal);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::LiteralList& literals);

}  // namespace formalism

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
