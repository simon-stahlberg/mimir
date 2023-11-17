#ifndef MIMIR_FORMALISM_ACTION_SCHEMA_HPP_
#define MIMIR_FORMALISM_ACTION_SCHEMA_HPP_

#include "declarations.hpp"
#include "function.hpp"
#include "implication.hpp"
#include "literal.hpp"
#include "object.hpp"

#include <string>

namespace mimir::formalism
{
    class ActionSchemaImpl
    {
      public:
        std::string name;
        int32_t arity;
        bool complete;
        mimir::formalism::ParameterList parameters;
        mimir::formalism::LiteralList precondition;
        mimir::formalism::LiteralList unconditional_effect;
        mimir::formalism::ImplicationList conditional_effect;
        mimir::formalism::Function cost;

        /**
         * @brief Construct a new action achema object.
         *
         * @param name The name of the action schema.
         * @param parameters The parameters (free variables) of the action schema.
         * @param precondition The precondition of the action schema (interpreted as conjunction of literals).
         * @param effect The effect of the action schema (interpreted as conjunction of literals).
         */
        ActionSchemaImpl(const std::string& name,
                         const mimir::formalism::ParameterList& parameters,
                         const mimir::formalism::LiteralList& precondition,
                         const mimir::formalism::LiteralList& effect,
                         const mimir::formalism::ImplicationList& conditional_effect,
                         const mimir::formalism::Function& cost);
    };

    ActionSchema create_action_schema(const std::string& name,
                                      const mimir::formalism::ParameterList& parameters,
                                      const mimir::formalism::LiteralList& precondition,
                                      const mimir::formalism::LiteralList& effect,
                                      const mimir::formalism::ImplicationList& conditional_effect,
                                      const mimir::formalism::Function& cost);

    ActionSchema relax(const mimir::formalism::ActionSchema& action_schema, bool remove_negative_preconditions, bool remove_delete_list);

    bool affects_predicate(const mimir::formalism::ActionSchema& action_schema, const mimir::formalism::Predicate& predicate);

    bool affect_predicate(const mimir::formalism::ActionSchemaList& action_schemas, const mimir::formalism::Predicate& predicate);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ActionSchema& action_schema);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ActionSchemaList& action_schemas);
}  // namespace formalism

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
