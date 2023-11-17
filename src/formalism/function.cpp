#include "../../include/mimir/formalism/domain.hpp"
#include "../../include/mimir/formalism/function.hpp"

namespace mimir::formalism
{
    FunctionImpl::FunctionImpl(FunctionOperation operation, const mimir::formalism::Predicate& variable, const mimir::formalism::Atom& atom) :
        operation_(operation),
        variable_(variable),
        atom_(atom),
        constant_(0.0),
        is_constant_(false)
    {
    }

    FunctionImpl::FunctionImpl(FunctionOperation operation, const mimir::formalism::Predicate& variable, double constant) :
        operation_(operation),
        variable_(variable),
        atom_(),
        constant_(constant),
        is_constant_(true)
    {
    }

    FunctionOperation FunctionImpl::get_operation() { return operation_; }

    mimir::formalism::Predicate FunctionImpl::get_variable() { return variable_; }

    bool FunctionImpl::is_constant() { return is_constant_; }

    double FunctionImpl::get_constant() { return constant_; }

    bool FunctionImpl::has_atom() { return !is_constant_; }

    mimir::formalism::Atom FunctionImpl::get_atom() { return atom_; }

    Function create_function(FunctionOperation operation, const mimir::formalism::Predicate& variable, const mimir::formalism::Atom& atom)
    {
        return std::make_shared<FunctionImpl>(operation, variable, atom);
    }

    Function create_function(FunctionOperation operation, const mimir::formalism::Predicate& variable, double constant)
    {
        return std::make_shared<FunctionImpl>(operation, variable, constant);
    }

    Function create_unit_cost_function(const mimir::formalism::DomainDescription& domain)
    {
        for (const auto& function : domain->functions)
        {
            if (function->name == "total-cost")
            {
                return create_function(FunctionOperation::INCREASE, function, 1.0);
            }
        }

        throw std::invalid_argument("expected \"total-cost\" to exist");
    }
}  // namespace formalism
