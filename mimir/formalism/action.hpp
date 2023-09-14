#if !defined(FORMALISM_ACTION_HPP_)
#define FORMALISM_ACTION_HPP_

#include "action_schema.hpp"
#include "bitset.hpp"
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
        formalism::Bitset positive_precondition_bitset_;
        formalism::Bitset negative_precondition_bitset_;
        formalism::Bitset positive_effect_bitset_;
        formalism::Bitset negative_effect_bitset_;
        formalism::ObjectList arguments_;
        formalism::LiteralList precondition_;
        formalism::LiteralList effect_;

        void initialize_precondition();

        void initialize_effect();

      public:
        formalism::ProblemDescription problem;
        formalism::ActionSchema schema;
        double cost;

        ActionImpl(const formalism::ProblemDescription& problem,
                   const formalism::ActionSchema& schema,
                   formalism::ObjectList&& arguments,
                   formalism::LiteralList&& precondition,
                   formalism::LiteralList&& effect,
                   double cost = 1);

        ActionImpl(const formalism::ProblemDescription& problem, const formalism::ActionSchema& schema, formalism::ObjectList&& arguments, int32_t cost = 1);

        ActionImpl(const formalism::ProblemDescription& problem, const formalism::ActionSchema& schema, const formalism::ParameterAssignment& assignment);

        friend bool is_applicable(const formalism::Action& action, const formalism::State& state);

        friend formalism::State apply(const formalism::Action& action, const formalism::State& state);

        const formalism::ObjectList& get_arguments() const;

        const formalism::LiteralList& get_precondition() const;

        const formalism::LiteralList& get_effect() const;
    };

    Action create_action(const formalism::ProblemDescription& problem,
                         const formalism::ActionSchema& schema,
                         formalism::ObjectList&& arguments,
                         formalism::LiteralList&& precondition,
                         formalism::LiteralList&& effect,
                         double cost);

    Action create_action(const formalism::ProblemDescription& problem, const formalism::ActionSchema& schema, formalism::ObjectList&& arguments, double cost);

    Action create_action(const formalism::ProblemDescription& problem,
                         const formalism::ActionSchema& schema,
                         const formalism::ParameterAssignment& assignment,
                         double cost);

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
