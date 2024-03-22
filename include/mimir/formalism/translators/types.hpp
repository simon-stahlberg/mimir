#ifndef MIMIR_FORMALISM_TRANSLATORS_TYPES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_TYPES_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{
class TypeTranslator : public BaseTranslator<TypeTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<TypeTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;

    loki::pddl::Problem translate_impl(const loki::pddl::ProblemImpl& problem)
    {
        std::cout << "TypeTranslator translate problem" << std::endl;
        return this->m_pddl_factories.problems.get_or_create<loki::pddl::ProblemImpl>(
            this->translate(*problem.get_domain()),
            problem.get_name(),
            this->translate(*problem.get_requirements()),
            this->translate(problem.get_objects()),
            this->translate(problem.get_initial_literals()),
            this->translate(problem.get_numeric_fluents()),
            this->translate(*problem.get_goal_condition()),
            (problem.get_optimization_metric().has_value() ?
                 std::optional<loki::pddl::OptimizationMetric>(this->translate(*problem.get_optimization_metric().value())) :
                 std::nullopt));
    }

public:
    explicit TypeTranslator(loki::PDDLFactories& pddl_factories) : BaseTranslator<TypeTranslator>(pddl_factories) {}
};

}
#endif
