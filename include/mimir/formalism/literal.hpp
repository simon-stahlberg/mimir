#ifndef MIMIR_FORMALISM_LITERAL_HPP_
#define MIMIR_FORMALISM_LITERAL_HPP_

#include "atom.hpp"
#include "predicate.hpp"

#include "../common/mixins.hpp"

#include <loki/domain/pddl/literal.hpp>

#include <memory>
#include <stdexcept>
#include <vector>


namespace mimir
{
    class Literal : public FormattingMixin<Literal>
    {
      private:
        loki::pddl::Literal external_;

        explicit Literal(loki::pddl::Literal external_literal);

      public:
        uint32_t get_id() const;
        bool is_negated() const;
        Atom get_atom() const;
        uint32_t get_atom_id() const;
        Predicate get_predicate() const;

        Literal ground_literal(const ParameterAssignment& assignment) const;

        bool contains(const Predicate& predicate) const;

        std::size_t hash() const;

        bool operator<(const Literal& other) const;
        bool operator>(const Literal& other) const;
        bool operator==(const Literal& other) const;
        bool operator!=(const Literal& other) const;
        bool operator<=(const Literal& other) const;

        friend class ActionSchema;
        friend class Problem;
    };

    using LiteralList = std::vector<Literal>;

    AtomList as_atoms(const LiteralList& literals);

    LiteralList ground_literals(const LiteralList& literal_list, const ParameterAssignment& assignment);

    // bool contains_predicate(const mimir::LiteralList& literals, const mimir::Predicate& predicate);

    // std::ostream& operator<<(std::ostream& os, const mimir::LiteralList& literals);

}  // namespace mimir

/*
namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::Literal>
    {
        std::size_t operator()(const mimir::Literal& literal) const;
    };

    template<>
    struct hash<mimir::LiteralList>
    {
        std::size_t operator()(const mimir::LiteralList& literals) const;
    };

    template<>
    struct less<mimir::Literal>
    {
        bool operator()(const mimir::Literal& left_literal, const mimir::Literal& right_literal) const;
    };

    template<>
    struct equal_to<mimir::Literal>
    {
        bool operator()(const mimir::Literal& left_literal, const mimir::Literal& right_literal) const;
    };

}  // namespace std
*/

#endif  // MIMIR_FORMALISM_LITERAL_HPP_
