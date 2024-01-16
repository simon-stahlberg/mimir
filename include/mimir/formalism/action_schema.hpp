#ifndef MIMIR_FORMALISM_ACTION_SCHEMA_HPP_
#define MIMIR_FORMALISM_ACTION_SCHEMA_HPP_

#include "function.hpp"
#include "implication.hpp"
#include "literal.hpp"
#include "term.hpp"

#include "../common/mixins.hpp"

#include <loki/common/pddl/types.hpp>
#include <loki/domain/pddl/action.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace mimir
{
    class ActionSchema : public FormattingMixin<ActionSchema>
    {
      private:
        loki::pddl::Action external_;

        explicit ActionSchema(loki::pddl::Action external_action);

        LiteralList parse_conjunctive_literals(loki::pddl::Condition condition) const;

      public:
        explicit ActionSchema();

        uint32_t get_id() const;
        const std::string& get_name() const;
        std::size_t get_arity() const;
        TermList get_parameters() const;
        LiteralList get_precondition() const;
        LiteralList get_effect() const;
        ImplicationList get_conditional_effect() const;
        Function get_cost() const;
        bool is_valid() const;

        // bool affects_predicate(const mimir::Predicate& predicate) const;  // TODO: Is this one used?

        std::size_t hash() const;

        bool operator<(const ActionSchema& other) const;
        bool operator>(const ActionSchema& other) const;
        bool operator==(const ActionSchema& other) const;
        bool operator!=(const ActionSchema& other) const;
        bool operator<=(const ActionSchema& other) const;
    };

    // bool affect_predicate(const mimir::ActionSchemaList& action_schemas, const mimir::Predicate& predicate);

    using ActionSchemaList = std::vector<ActionSchema>;

}  // namespace mimir

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::ActionSchema>
    {
        std::size_t operator()(const mimir::ActionSchema& action_schema) const;
    };

    template<>
    struct hash<mimir::ActionSchemaList>
    {
        std::size_t operator()(const mimir::ActionSchemaList& action_schemas) const;
    };

    template<>
    struct less<mimir::ActionSchema>
    {
        bool operator()(const mimir::ActionSchema& left_action_schema, const mimir::ActionSchema& right_action_schema) const;
    };

    template<>
    struct equal_to<mimir::ActionSchema>
    {
        bool operator()(const mimir::ActionSchema& left_action_schema, const mimir::ActionSchema& right_action_schema) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_ACTION_SCHEMA_HPP_
