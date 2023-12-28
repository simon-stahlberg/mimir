#ifndef MIMIR_FORMALISM_ACTION_SCHEMA_HPP_
#define MIMIR_FORMALISM_ACTION_SCHEMA_HPP_

#include "declarations.hpp"

#include <loki/common/pddl/types.hpp>
#include <loki/domain/pddl/action.hpp>
#include <string>

namespace mimir::formalism
{
    class ActionSchema
    {
      private:
        loki::pddl::Action external_;

        explicit ActionSchema(loki::pddl::Action external_action);

        LiteralList parse_conjunctive_literals(loki::pddl::Condition condition) const;

      public:
        const std::string& get_name() const;
        std::size_t get_arity() const;
        TermList get_parameters() const;
        LiteralList get_precondition() const;
        LiteralList get_effect() const;
        ImplicationList get_conditional_effect() const;
        Function get_cost() const;

        ActionSchema delete_relax(ActionSchemaFactory& ref_factory) const;

        // bool affects_predicate(const mimir::formalism::Predicate& predicate) const;  // TODO: Is this one used?

        std::size_t hash() const;

        bool operator<(const ActionSchema& other) const;
        bool operator>(const ActionSchema& other) const;
        bool operator==(const ActionSchema& other) const;
        bool operator!=(const ActionSchema& other) const;
        bool operator<=(const ActionSchema& other) const;

        friend std::ostream& operator<<(std::ostream& os, const ActionSchema& action_schema);
    };

    // bool affect_predicate(const mimir::formalism::ActionSchemaList& action_schemas, const mimir::formalism::Predicate& predicate);

    // std::ostream& operator<<(std::ostream& os, const mimir::formalism::ActionSchemaList& action_schemas);

}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::ActionSchema>
    {
        std::size_t operator()(const mimir::formalism::ActionSchema& action_schema) const;
    };

    template<>
    struct hash<mimir::formalism::ActionSchemaList>
    {
        std::size_t operator()(const mimir::formalism::ActionSchemaList& action_schemas) const;
    };

    template<>
    struct less<mimir::formalism::ActionSchema>
    {
        bool operator()(const mimir::formalism::ActionSchema& left_action_schema, const mimir::formalism::ActionSchema& right_action_schema) const;
    };

    template<>
    struct equal_to<mimir::formalism::ActionSchema>
    {
        bool operator()(const mimir::formalism::ActionSchema& left_action_schema, const mimir::formalism::ActionSchema& right_action_schema) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_ACTION_SCHEMA_HPP_
