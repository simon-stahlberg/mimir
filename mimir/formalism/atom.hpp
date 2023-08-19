#if !defined(FORMALISM_ATOM_HPP_)
#define FORMALISM_ATOM_HPP_

#include "declarations.hpp"
#include "object.hpp"
#include "predicate.hpp"

namespace formalism
{
    class AtomImpl
    {
      private:
        std::size_t hash_;

      public:
        const formalism::Predicate predicate;
        const formalism::ObjectList arguments;

        AtomImpl(const formalism::Predicate& predicate, formalism::ObjectList&& arguments);

        AtomImpl(const formalism::Predicate& predicate, const formalism::ObjectList& arguments);

        bool operator==(const AtomImpl& other) const;

        bool operator!=(const AtomImpl& other) const;

        template<typename T>
        friend class std::hash;
    };

    formalism::Atom ground_predicate(const formalism::Predicate& predicate, const formalism::ParameterAssignment& assignment);

    bool matches(const Atom& first_atom, const Atom& second_atom);

    Atom replace_term(const Atom& atom, uint32_t index, const formalism::Object& object);

    Atom create_atom(const formalism::Predicate& predicate, formalism::ObjectList&& arguments);

    Atom create_atom(const formalism::Predicate& predicate, const formalism::ObjectList& arguments);

    formalism::AtomList filter(const formalism::AtomList& atom_list, const formalism::Object& obj, int32_t argument_index);

    formalism::AtomList filter(const formalism::AtomList& atom_list, const formalism::ObjectList& object_list, int32_t argument_index);

    formalism::AtomList exclude(const formalism::AtomList& atom_list, const formalism::AtomList& other_list);

    formalism::ObjectList get_objects(const formalism::AtomList& atom_list, int32_t argument_index);

    formalism::ObjectList get_unique_objects(const formalism::AtomList& atom_list, int32_t argument_index);

    formalism::ObjectList concatenate(const formalism::ObjectList& left_list, const formalism::ObjectList& right_list);

    formalism::ObjectList difference(const formalism::ObjectList& left_list, const formalism::ObjectList& right_list);

    std::ostream& operator<<(std::ostream& os, const formalism::Atom& atom);

    std::ostream& operator<<(std::ostream& os, const formalism::AtomList& atoms);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<formalism::Atom>
    {
        std::size_t operator()(const formalism::Atom& atom) const;
    };

    template<>
    struct hash<formalism::AtomList>
    {
        std::size_t operator()(const formalism::AtomList& atoms) const;
    };

    template<>
    struct less<formalism::Atom>
    {
        bool operator()(const formalism::Atom& left_atom, const formalism::Atom& right_atom) const;
    };

    template<>
    struct equal_to<formalism::Atom>
    {
        bool operator()(const formalism::Atom& left_atom, const formalism::Atom& right_atom) const;
    };

}  // namespace std

#endif  // FORMALISM_ATOM_HPP_
