#ifndef MIMIR_FORMALISM_ATOM_HPP_
#define MIMIR_FORMALISM_ATOM_HPP_

#include "declarations.hpp"

#include <loki/domain/pddl/atom.hpp>
#include <vector>

namespace mimir::formalism
{
    class Atom
    {
      private:
        loki::pddl::Atom external_;

        explicit Atom(loki::pddl::Atom external_atom);

      public:
        uint32_t get_id() const;
        Predicate get_predicate() const;
        TermList get_terms() const;

        bool matches(const Atom& first_atom, const Atom& second_atom) const;

        std::size_t hash() const;

        bool operator<(const Atom& other) const;
        bool operator>(const Atom& other) const;
        bool operator==(const Atom& other) const;
        bool operator!=(const Atom& other) const;
        bool operator<=(const Atom& other) const;

        friend std::ostream& operator<<(std::ostream& os, const Atom& atom);
        friend class Literal;
        friend class Problem;
    };

    // std::ostream& operator<<(std::ostream& os, const mimir::formalism::AtomList& atoms);

}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::Atom>
    {
        std::size_t operator()(const mimir::formalism::Atom& atom) const;
    };

    template<>
    struct hash<mimir::formalism::AtomList>
    {
        std::size_t operator()(const mimir::formalism::AtomList& atoms) const;
    };

    template<>
    struct less<mimir::formalism::Atom>
    {
        bool operator()(const mimir::formalism::Atom& left_atom, const mimir::formalism::Atom& right_atom) const;
    };

    template<>
    struct equal_to<mimir::formalism::Atom>
    {
        bool operator()(const mimir::formalism::Atom& left_atom, const mimir::formalism::Atom& right_atom) const;
    };
}  // namespace std

#endif  // MIMIR_FORMALISM_ATOM_HPP_
