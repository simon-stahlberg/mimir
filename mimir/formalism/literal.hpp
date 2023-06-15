#if !defined(FORMALISM_LITERAL_HPP_)
#define FORMALISM_LITERAL_HPP_

#include "atom.hpp"
#include "declarations.hpp"

#include <memory>
#include <vector>

namespace formalism
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

    formalism::Literal create_literal(const Atom& atom, const bool negated);

    formalism::Literal ground_literal(const formalism::Literal& literal, const formalism::ParameterAssignment& assignment);

    formalism::LiteralList ground_literal_list(const formalism::LiteralList& literal_list, const formalism::ParameterAssignment& assignment);

    formalism::AtomList as_atoms(const formalism::LiteralList& literals);

    bool contains_predicate(const formalism::LiteralList& literals, const formalism::Predicate& predicate);

    std::ostream& operator<<(std::ostream& os, const formalism::Literal& literal);

    std::ostream& operator<<(std::ostream& os, const formalism::LiteralList& literals);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<formalism::Literal>
    {
        std::size_t operator()(const formalism::Literal& literal) const;
    };

    template<>
    struct hash<formalism::LiteralList>
    {
        std::size_t operator()(const formalism::LiteralList& literals) const;
    };

    template<>
    struct less<formalism::Literal>
    {
        bool operator()(const formalism::Literal& left_literal, const formalism::Literal& right_literal) const;
    };

    template<>
    struct equal_to<formalism::Literal>
    {
        bool operator()(const formalism::Literal& left_literal, const formalism::Literal& right_literal) const;
    };

}  // namespace std

#endif  // FORMALISM_LITERAL_HPP_
