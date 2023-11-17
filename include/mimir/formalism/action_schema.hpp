#if !defined(FORMALISM_ACTION_SCHEMA_HPP_)
#define FORMALISM_ACTION_SCHEMA_HPP_

#include "declarations.hpp"
#include "function.hpp"
#include "implication.hpp"
#include "literal.hpp"
#include "object.hpp"

#include <string>

namespace formalism
{
    class ActionSchemaImpl
    {
      public:
        std::string name;
        int32_t arity;
        bool complete;
        formalism::ParameterList parameters;
        formalism::LiteralList precondition;
        formalism::LiteralList unconditional_effect;
        formalism::ImplicationList conditional_effect;
        formalism::Function cost;

        /**
         * @brief Construct a new action achema object.
         *
         * @param name The name of the action schema.
         * @param parameters The parameters (free variables) of the action schema.
         * @param precondition The precondition of the action schema (interpreted as conjunction of literals).
         * @param effect The effect of the action schema (interpreted as conjunction of literals).
         */
        ActionSchemaImpl(const std::string& name,
                         const formalism::ParameterList& parameters,
                         const formalism::LiteralList& precondition,
                         const formalism::LiteralList& effect,
                         const formalism::ImplicationList& conditional_effect,
                         const formalism::Function& cost);
    };

    ActionSchema create_action_schema(const std::string& name,
                                      const formalism::ParameterList& parameters,
                                      const formalism::LiteralList& precondition,
                                      const formalism::LiteralList& effect,
                                      const formalism::ImplicationList& conditional_effect,
                                      const formalism::Function& cost);

    ActionSchema relax(const formalism::ActionSchema& action_schema, bool remove_negative_preconditions, bool remove_delete_list);

    bool affects_predicate(const formalism::ActionSchema& action_schema, const formalism::Predicate& predicate);

    bool affect_predicate(const formalism::ActionSchemaList& action_schemas, const formalism::Predicate& predicate);

    std::ostream& operator<<(std::ostream& os, const formalism::ActionSchema& action_schema);

    std::ostream& operator<<(std::ostream& os, const formalism::ActionSchemaList& action_schemas);
}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<formalism::ActionSchema>
    {
        std::size_t operator()(const formalism::ActionSchema& action_schema) const;
    };

    template<>
    struct hash<formalism::ActionSchemaList>
    {
        std::size_t operator()(const formalism::ActionSchemaList& action_schemas) const;
    };

    template<>
    struct less<formalism::ActionSchema>
    {
        bool operator()(const formalism::ActionSchema& left_action_schema, const formalism::ActionSchema& right_action_schema) const;
    };

    template<>
    struct equal_to<formalism::ActionSchema>
    {
        bool operator()(const formalism::ActionSchema& left_action_schema, const formalism::ActionSchema& right_action_schema) const;
    };

}  // namespace std

#endif  // FORMALISM_ACTION_SCHEMA_HPP_
