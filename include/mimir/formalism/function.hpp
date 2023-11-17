#if !defined(FORMALISM_FUNCTION_HPP_)
#define FORMALISM_FUNCTION_HPP_

#include "declarations.hpp"

namespace formalism
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
        formalism::Predicate variable_;
        formalism::Atom atom_;
        double constant_;
        bool is_constant_;

      public:
        FunctionImpl(FunctionOperation operation, const formalism::Predicate& variable, const formalism::Atom& atom);

        FunctionImpl(FunctionOperation operation, const formalism::Predicate& variable, double constant);

        FunctionOperation get_operation();

        formalism::Predicate get_variable();

        bool is_constant();

        double get_constant();

        bool has_atom();

        formalism::Atom get_atom();
    };

    Function create_function(FunctionOperation operation, const formalism::Predicate& variable, const formalism::Atom& atom);

    Function create_function(FunctionOperation operation, const formalism::Predicate& variable, double constant);

    Function create_unit_cost_function(const formalism::DomainDescription& domain);
}  // namespace formalism

#endif  // FORMALISM_FUNCTION_HPP_
