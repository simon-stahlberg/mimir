#ifndef MIMIR_FORMALISM_FUNCTION_HPP_
#define MIMIR_FORMALISM_FUNCTION_HPP_

#include "../common/mixins.hpp"

#include <stdexcept>
#include <vector>

namespace mimir::formalism
{
    class Function : public BaseMixin<Function>
    {
        // loki::pddl::Function
      public:
        bool is_defined() const { throw std::runtime_error("not implemented"); }
    };

    // enum FunctionOperation
    // {
    //     INCREASE,
    //     DECREASE
    // };

    // class FunctionImpl
    // {
    //   private:
    //     FunctionOperation operation_;
    //     mimir::formalism::Predicate variable_;
    //     mimir::formalism::Atom atom_;
    //     double constant_;
    //     bool is_constant_;

    //   public:
    //     FunctionImpl(FunctionOperation operation, const mimir::formalism::Predicate& variable, const mimir::formalism::Atom& atom);

    //     FunctionImpl(FunctionOperation operation, const mimir::formalism::Predicate& variable, double constant);

    //     FunctionOperation get_operation();

    //     mimir::formalism::Predicate get_variable();

    //     bool is_constant();

    //     double get_constant();

    //     bool has_atom();

    //     mimir::formalism::Atom get_atom();
    // };

    // Function create_function(FunctionOperation operation, const mimir::formalism::Predicate& variable, const mimir::formalism::Atom& atom);

    // Function create_function(FunctionOperation operation, const mimir::formalism::Predicate& variable, double constant);

    // Function create_unit_cost_function(const mimir::formalism::Domain& domain);

    using FunctionList = std::vector<Function>;

}  // namespace formalism

#endif  // MIMIR_FORMALISM_FUNCTION_HPP_
