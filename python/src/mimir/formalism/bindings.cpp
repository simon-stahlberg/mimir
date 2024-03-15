#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

#include "mimir/mimir.hpp"

#include <variant>

using namespace mimir;
namespace py = pybind11;

/* Variant type casters */
namespace PYBIND11_NAMESPACE
{
namespace detail
{
template<>
struct type_caster<std::variant<TermObjectImpl, TermVariableImpl>> : variant_caster<std::variant<TermObjectImpl, TermVariableImpl>>
{
};

template<>
struct type_caster<
    std::variant<ConditionLiteralImpl, ConditionAndImpl, ConditionOrImpl, ConditionNotImpl, ConditionImplyImpl, ConditionExistsImpl, ConditionForallImpl>> :
    variant_caster<
        std::variant<ConditionLiteralImpl, ConditionAndImpl, ConditionOrImpl, ConditionNotImpl, ConditionImplyImpl, ConditionExistsImpl, ConditionForallImpl>>
{
};

template<>
struct type_caster<std::variant<EffectLiteralImpl, EffectAndImpl, EffectNumericImpl, EffectConditionalForallImpl, EffectConditionalWhenImpl>> :
    variant_caster<std::variant<EffectLiteralImpl, EffectAndImpl, EffectNumericImpl, EffectConditionalForallImpl, EffectConditionalWhenImpl>>
{
};

template<>
struct type_caster<std::variant<FunctionExpressionNumberImpl,
                                FunctionExpressionBinaryOperatorImpl,
                                FunctionExpressionMultiOperatorImpl,
                                FunctionExpressionMinusImpl,
                                FunctionExpressionFunctionImpl>> :
    variant_caster<std::variant<FunctionExpressionNumberImpl,
                                FunctionExpressionBinaryOperatorImpl,
                                FunctionExpressionMultiOperatorImpl,
                                FunctionExpressionMinusImpl,
                                FunctionExpressionFunctionImpl>>
{
};

template<>
struct visit_helper<std::variant>
{
    template<typename... Args>
    static auto call(Args&&... args) -> decltype(std::visit(args...))
    {
        return std::visit(args...);
    }
};
}
}  // namespace PYBIND11_NAMESPACE::detail

void init_formalism(py::module_& m_formalism)
{
    py::class_<Action>(m_formalism, "Action");

    py::class_<Atom>(m_formalism, "Atom");

    py::class_<Condition>(m_formalism, "Condition");

    py::class_<Domain>(m_formalism, "Domain");

    py::class_<Effect>(m_formalism, "Effect");

    py::class_<FunctionExpression>(m_formalism, "FunctionExpression");

    py::class_<FunctionSkeleton>(m_formalism, "FunctionSkeleton");

    py::class_<Function>(m_formalism, "Function");

    py::class_<GroundAtom>(m_formalism, "GroundAtom");

    py::class_<GroundLiteral>(m_formalism, "GroundLiteral");

    py::class_<Literal>(m_formalism, "Literal");

    py::class_<OptimizationMetric>(m_formalism, "OptimizationMetric");

    py::class_<NumericFluent>(m_formalism, "NumericFluent");

    py::class_<Object>(m_formalism, "Object");

    py::class_<Parameter>(m_formalism, "Parameter");

    py::class_<PDDLFactories>(m_formalism, "PDDLFactories");

    py::class_<PDDLParser>(m_formalism, "PDDLParser")  //
        .def(py::init<std::string, std::string>())
        .def("get_domain", &PDDLParser::get_domain)
        .def("get_problem", &PDDLParser::get_problem)
        .def("get_factories", &PDDLParser::get_factories);

    py::class_<Predicate>(m_formalism, "Predicate");

    py::class_<Problem>(m_formalism, "Problem");

    py::class_<Requirements>(m_formalism, "Requirements");

    py::class_<Term>(m_formalism, "Term")
        .def("get_object", &TermObjectImpl::get_object)         //
        .def("get_variable", &TermVariableImpl::get_variable);  //

    py::class_<Type>(m_formalism, "Type");

    py::class_<Variable>(m_formalism, "Variable");
}
