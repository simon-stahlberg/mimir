#if !defined(FORMALISM_ACTION_HPP_)
#define FORMALISM_ACTION_HPP_

#include "action_schema.hpp"
#include "declarations.hpp"
#include "literal.hpp"
#include "object.hpp"
#include "problem.hpp"
#include "state.hpp"

#include <iostream>

namespace formalism
{
    class ActionImpl
    {
      private:
        std::vector<uint32_t> positive_precondition_ranks_;
        std::vector<uint32_t> negative_precondition_ranks_;
        std::vector<uint32_t> positive_effect_ranks_;
        std::vector<uint32_t> negative_effect_ranks_;

      public:
        formalism::ProblemDescription problem;
        formalism::ActionSchema schema;
        formalism::ObjectList arguments;
        int32_t cost;

        ActionImpl(const formalism::ProblemDescription& problem,
                   const formalism::ActionSchema& schema,
                   const formalism::ObjectList& arguments,
                   const int32_t cost = 1);

        ActionImpl(const formalism::ProblemDescription& problem, const formalism::ActionSchema& schema, const formalism::ParameterAssignment& assignment);

        friend bool is_applicable(const formalism::Action& action, const formalism::State& state);

        friend formalism::State apply(const formalism::Action& action, const formalism::State& state);

        formalism::LiteralList get_precondition() const;

        formalism::LiteralList get_effect() const;
    };

    Action create_action(const formalism::ProblemDescription& problem,
                         const formalism::ActionSchema& schema,
                         const formalism::ObjectList& arguments,
                         const int32_t cost = 1);

    Action create_action(const formalism::ProblemDescription& problem, const formalism::ActionSchema& schema, const formalism::ParameterAssignment& assignment);

    std::ostream& operator<<(std::ostream& os, const formalism::ActionImpl& action);

    std::ostream& operator<<(std::ostream& os, const formalism::Action& action);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<formalism::Action>
    {
        std::size_t operator()(const formalism::Action& action) const;
    };

    template<>
    struct less<formalism::Action>
    {
        bool operator()(const formalism::Action& left_action, const formalism::Action& right_action) const;
    };

    template<>
    struct equal_to<formalism::Action>
    {
        bool operator()(const formalism::Action& left_action, const formalism::Action& right_action) const;
    };

}  // namespace std

#endif  // FORMALISM_ACTION_HPP_
