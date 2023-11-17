#ifndef MIMIR_FORMALISM_ATOM_HPP_
#define MIMIR_FORMALISM_ATOM_HPP_

#include "declarations.hpp"
#include "object.hpp"
#include "predicate.hpp"

namespace mimir::formalism
{
    class AtomImpl
    {
      private:
        std::size_t hash_;

        void validate() const;

      public:
        const mimir::formalism::Predicate predicate;
        const mimir::formalism::ObjectList arguments;

        AtomImpl(const mimir::formalism::Predicate& predicate, mimir::formalism::ObjectList&& arguments);

        AtomImpl(const mimir::formalism::Predicate& predicate, const mimir::formalism::ObjectList& arguments);

        bool operator==(const AtomImpl& other) const;

        bool operator!=(const AtomImpl& other) const;

        template<typename T>
        friend class std::hash;
    };

    mimir::formalism::Atom ground_predicate(const mimir::formalism::Predicate& predicate, const mimir::formalism::ParameterAssignment& assignment);

    bool matches(const Atom& first_atom, const Atom& second_atom);

    Atom replace_term(const Atom& atom, uint32_t index, const mimir::formalism::Object& object);

    Atom create_atom(const mimir::formalism::Predicate& predicate, mimir::formalism::ObjectList&& arguments);

    Atom create_atom(const mimir::formalism::Predicate& predicate, const mimir::formalism::ObjectList& arguments);

    mimir::formalism::AtomList filter(const mimir::formalism::AtomList& atom_list, const mimir::formalism::Object& obj, int32_t argument_index);

    mimir::formalism::AtomList filter(const mimir::formalism::AtomList& atom_list, const mimir::formalism::ObjectList& object_list, int32_t argument_index);

    mimir::formalism::AtomList exclude(const mimir::formalism::AtomList& atom_list, const mimir::formalism::AtomList& other_list);

    mimir::formalism::ObjectList get_objects(const mimir::formalism::AtomList& atom_list, int32_t argument_index);

    mimir::formalism::ObjectList get_unique_objects(const mimir::formalism::AtomList& atom_list, int32_t argument_index);

    mimir::formalism::ObjectList concatenate(const mimir::formalism::ObjectList& left_list, const mimir::formalism::ObjectList& right_list);

    mimir::formalism::ObjectList difference(const mimir::formalism::ObjectList& left_list, const mimir::formalism::ObjectList& right_list);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Atom& atom);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::AtomList& atoms);

}  // namespace formalism

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
