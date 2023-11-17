#ifndef MIMIR_FORMALISM_ACTION_HPP_
#define MIMIR_FORMALISM_ACTION_HPP_

#include "action_schema.hpp"
#include "bitset.hpp"
#include "declarations.hpp"
#include "literal.hpp"
#include "object.hpp"
#include "problem.hpp"
#include "state.hpp"

#include <iostream>

namespace mimir::formalism
{
    class ActionImpl
    {
      private:
        mimir::formalism::Bitset applicability_positive_precondition_bitset_;
        mimir::formalism::Bitset applicability_negative_precondition_bitset_;
        mimir::formalism::Bitset unconditional_positive_effect_bitset_;
        mimir::formalism::Bitset unconditional_negative_effect_bitset_;
        std::vector<mimir::formalism::Bitset> conditional_positive_precondition_bitsets_;
        std::vector<mimir::formalism::Bitset> conditional_negative_precondition_bitsets_;
        std::vector<mimir::formalism::Bitset> conditional_positive_effect_bitsets_;
        std::vector<mimir::formalism::Bitset> conditional_negative_effect_bitsets_;
        mimir::formalism::ObjectList arguments_;
        mimir::formalism::LiteralList applicability_precondition_;
        mimir::formalism::LiteralList unconditional_effect_;
        mimir::formalism::ImplicationList conditional_effect_;

        void initialize_precondition();

        void initialize_effect();

      public:
        mimir::formalism::ProblemDescription problem;
        mimir::formalism::ActionSchema schema;
        double cost;

        ActionImpl(const mimir::formalism::ProblemDescription& problem,
                   const mimir::formalism::ActionSchema& schema,
                   mimir::formalism::ObjectList&& arguments,
                   mimir::formalism::LiteralList&& precondition,
                   mimir::formalism::LiteralList&& unconditional_effect,
                   mimir::formalism::ImplicationList&& conditional_effect,
                   double cost);

        ActionImpl(const mimir::formalism::ProblemDescription& problem,
                   const mimir::formalism::ActionSchema& schema,
                   mimir::formalism::ObjectList&& arguments,
                   int32_t cost = 1);

        ActionImpl(const mimir::formalism::ProblemDescription& problem,
                   const mimir::formalism::ActionSchema& schema,
                   const mimir::formalism::ParameterAssignment& assignment);

        friend bool is_applicable(const mimir::formalism::Action& action, const mimir::formalism::State& state);

        friend mimir::formalism::State apply(const mimir::formalism::Action& action, const mimir::formalism::State& state);

        const mimir::formalism::ObjectList& get_arguments() const;

        const mimir::formalism::LiteralList& get_precondition() const;

        const mimir::formalism::LiteralList& get_unconditional_effect() const;

        const mimir::formalism::ImplicationList& get_conditional_effect() const;
    };

    Action create_action(const mimir::formalism::ProblemDescription& problem,
                         const mimir::formalism::ActionSchema& schema,
                         mimir::formalism::ObjectList&& arguments,
                         mimir::formalism::LiteralList&& precondition,
                         mimir::formalism::LiteralList&& unconditional_effect,
                         mimir::formalism::ImplicationList&& conditional_effect,
                         double cost);

    Action create_action(const mimir::formalism::ProblemDescription& problem,
                         const mimir::formalism::ActionSchema& schema,
                         mimir::formalism::ObjectList&& arguments,
                         double cost);

    Action create_action(const mimir::formalism::ProblemDescription& problem,
                         const mimir::formalism::ActionSchema& schema,
                         const mimir::formalism::ParameterAssignment& assignment,
                         double cost);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ActionImpl& action);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Action& action);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::Action>
    {
        std::size_t operator()(const mimir::formalism::Action& action) const;
    };

    template<>
    struct less<mimir::formalism::Action>
    {
        bool operator()(const mimir::formalism::Action& left_action, const mimir::formalism::Action& right_action) const;
    };

    template<>
    struct equal_to<mimir::formalism::Action>
    {
        bool operator()(const mimir::formalism::Action& left_action, const mimir::formalism::Action& right_action) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_ACTION_HPP_
