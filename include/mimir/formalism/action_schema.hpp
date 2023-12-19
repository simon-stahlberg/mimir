#ifndef MIMIR_FORMALISM_ACTION_SCHEMA_HPP_
#define MIMIR_FORMALISM_ACTION_SCHEMA_HPP_

#include "declarations.hpp"
#include "function.hpp"
#include "implication.hpp"
#include "literal.hpp"
#include "object.hpp"

#include <loki/common/pddl/types.hpp>
#include <loki/domain/pddl/action.hpp>
#include <string>

namespace mimir::formalism
{

    class ActionSchema
    {
      private:
        loki::pddl::Action external_;

      public:
        explicit ActionSchema(loki::pddl::Action external_action);

        ActionSchema delete_relax(ActionSchemaFactory& factory) const;

        bool affects_predicate(const mimir::formalism::Predicate& predicate) const;  // TODO: Is this one used?

        friend std::ostream& operator<<(std::ostream& os, const ActionSchema& action_schema);
    };

    bool affect_predicate(const mimir::formalism::ActionSchemaList& action_schemas, const mimir::formalism::Predicate& predicate);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ActionSchemaList& action_schemas);

    using ActionSchemaList = std::vector<ActionSchema>;
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
