#ifndef MIMIR_FORMALISM_FUNCTION_HPP_
#define MIMIR_FORMALISM_FUNCTION_HPP_

#include "declarations.hpp"

namespace mimir::formalism
{
    enum FunctionOperation
    {
        INCREASE,
        DECREASE
    };

    class FunctionImpl
    {
      private:
        FunctionOperation operation_;
        mimir::formalism::Predicate variable_;
        mimir::formalism::Atom atom_;
        double constant_;
        bool is_constant_;

      public:
        FunctionImpl(FunctionOperation operation, const mimir::formalism::Predicate& variable, const mimir::formalism::Atom& atom);

        FunctionImpl(FunctionOperation operation, const mimir::formalism::Predicate& variable, double constant);

        FunctionOperation get_operation();

        mimir::formalism::Predicate get_variable();

        bool is_constant();

        double get_constant();

        bool has_atom();

        mimir::formalism::Atom get_atom();
    };

    Function create_function(FunctionOperation operation, const mimir::formalism::Predicate& variable, const mimir::formalism::Atom& atom);

    Function create_function(FunctionOperation operation, const mimir::formalism::Predicate& variable, double constant);

    Function create_unit_cost_function(const mimir::formalism::DomainDescription& domain);
}  // namespace formalism

#endif  // MIMIR_FORMALISM_FUNCTION_HPP_
