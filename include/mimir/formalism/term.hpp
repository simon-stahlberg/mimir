#ifndef MIMIR_FORMALISM_TERM_HPP_
#define MIMIR_FORMALISM_TERM_HPP_

#include "type.hpp"

#include "../common/mixins.hpp"

#include <loki/domain/pddl/parameter.hpp>
#include <loki/domain/pddl/variable.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace mimir::formalism
{
    class Term : public FormattingMixin<Term>
    {
      private:
        // TODO (Dominik): why not loki::pddl::Term?
        loki::pddl::Variable external_parameter_;
        loki::pddl::Object external_object_;

        explicit Term(loki::pddl::Variable variable);
        explicit Term(loki::pddl::Object object);

      public:
        explicit Term();

        const std::string& get_name() const;
        uint32_t get_id() const;
        const TypeList& get_bases() const;

        bool is_subtype_of(const Type& type) const;
        bool is_subtype_of_one(const TypeList& types) const;
        bool is_variable() const;
        bool is_constant() const;
        bool is_valid() const;

        std::size_t hash() const;

        bool operator<(const Term& other) const;
        bool operator>(const Term& other) const;
        bool operator==(const Term& other) const;
        bool operator!=(const Term& other) const;
        bool operator<=(const Term& other) const;

        friend std::ostream& operator<<(std::ostream& os, const Term& object);
        friend class ActionSchema;
        friend class Atom;
        friend class Domain;
        friend class Predicate;
        friend class Problem;
    };

    // mimir::formalism::ObjectList filter(const mimir::formalism::ObjectList& object_list, const mimir::formalism::Type& type);

    // std::ostream& operator<<(std::ostream& os, const mimir::formalism::Object& object);

    // std::ostream& operator<<(std::ostream& os, const mimir::formalism::ObjectList& objects);

    using TermList = std::vector<Term>;
    using ParameterAssignment = std::unordered_map<Term, Term>;

}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::Term>
    {
        std::size_t operator()(const mimir::formalism::Term& term) const;
    };

    template<>
    struct hash<mimir::formalism::TermList>
    {
        std::size_t operator()(const mimir::formalism::TermList& terms) const;
    };

    template<>
    struct less<mimir::formalism::Term>
    {
        bool operator()(const mimir::formalism::Term& left_term, const mimir::formalism::Term& right_term) const;
    };

    template<>
    struct equal_to<mimir::formalism::Term>
    {
        bool operator()(const mimir::formalism::Term& left_term, const mimir::formalism::Term& right_term) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_TERM_HPP_
