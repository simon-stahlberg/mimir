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

    /// @brief Translate a type to a predicate of arity 1.
    loki::pddl::Predicate translate_type_to_predicate(const loki::pddl::TypeImpl& type)
    {
        return this->m_pddl_factories.predicates.get_or_create<loki::pddl::PredicateImpl>(
            type.get_name(),
            loki::pddl::ParameterList { this->m_pddl_factories.parameters.get_or_create<loki::pddl::ParameterImpl>(
                this->m_pddl_factories.variables.get_or_create<loki::pddl::VariableImpl>("?arg"),
                loki::pddl::TypeList {}) });
    }

    /// @brief Translate an typed object to an untyped objects.
    loki::pddl::Object translate_typed_object_to_untyped_object(const loki::pddl::ObjectImpl& object)
    {
        return this->m_pddl_factories.objects.get_or_create<loki::pddl::ObjectImpl>(object.get_name(), loki::pddl::TypeList {});
    }

    /// @brief Translate an typed object to a list of ground literal for all its original types.
    loki::pddl::GroundLiteralList translate_typed_object_to_ground_literals(const loki::pddl::ObjectImpl& object)
    {
        auto additional_literals = loki::pddl::GroundLiteralList {};
        auto translated_object = translate_typed_object_to_untyped_object(object);
        auto types = collect_types_from_type_hierarchy(object.get_bases());
        for (const auto& type : types)
        {
            auto additional_literal = this->m_pddl_factories.ground_literals.get_or_create<loki::pddl::GroundLiteralImpl>(
                false,
                this->m_pddl_factories.ground_atoms.get_or_create<loki::pddl::GroundAtomImpl>(translate_type_to_predicate(*type),
                                                                                              loki::pddl::ObjectList { translated_object }));
            additional_literals.push_back(additional_literal);
        }
        return additional_literals;
    }

    /// @brief Translate a typed parameter to an untyped parameter.
    loki::pddl::Parameter translate_typed_parameter_to_untyped_parameter(const loki::pddl::ParameterImpl& parameter)
    {
        auto translated_parameter =
            this->m_pddl_factories.parameters.get_or_create<loki::pddl::ParameterImpl>(this->translate(*parameter.get_variable()), loki::pddl::TypeList {});
        return translated_parameter;
    }

    /// @brief Helper to compute all types from a hierarchy of types.
    void collect_types_from_type_hierarchy_recursively(const loki::pddl::Type& type, std::unordered_set<loki::pddl::Type>& ref_type_list)
    {
        ref_type_list.insert(type);
        for (const auto& base_type : type->get_bases())
        {
            collect_types_from_type_hierarchy_recursively(base_type, ref_type_list);
        }
    }

    /// @brief Compute all types from a hierarchy of types.
    loki::pddl::TypeList collect_types_from_type_hierarchy(const loki::pddl::TypeList& type_list)
    {
        std::unordered_set<loki::pddl::Type> flat_type_set;
        for (const auto& type : type_list)
        {
            collect_types_from_type_hierarchy_recursively(type, flat_type_set);
        }
        return loki::pddl::TypeList(flat_type_set.begin(), flat_type_set.end());
    }

    /**
     * Translate
     */
    loki::pddl::Object translate_impl(const loki::pddl::ObjectImpl& object) { return translate_typed_object_to_untyped_object(object); }

    loki::pddl::Parameter translate_impl(const loki::pddl::ParameterImpl& parameter) { return translate_typed_parameter_to_untyped_parameter(parameter); }

    loki::pddl::Domain translate_impl(const loki::pddl::DomainImpl& domain)
    {
        // Remove :typing requirement
        auto requirements_enum_set = domain.get_requirements()->get_requirements();
        requirements_enum_set.erase(loki::pddl::RequirementEnum::TYPING);
        auto translated_requirements = this->m_pddl_factories.requirements.get_or_create<loki::pddl::RequirementsImpl>(requirements_enum_set);

        // Make constants untyped
        auto translated_constants = loki::pddl::ObjectList {};
        translated_constants.reserve(domain.get_constants().size());
        for (const auto& object : domain.get_constants())
        {
            auto translated_object = translate_typed_object_to_untyped_object(*object);
            translated_constants.push_back(translated_object);
        }

        // Add type predicates
        auto translated_predicates = this->translate(domain.get_predicates());
        for (const auto type : collect_types_from_type_hierarchy(domain.get_types()))
        {
            translated_predicates.push_back(translate_type_to_predicate(*type));
        }
        return this->m_pddl_factories.domains.get_or_create<loki::pddl::DomainImpl>(domain.get_name(),
                                                                                    translated_requirements,
                                                                                    loki::pddl::TypeList {},
                                                                                    translated_constants,
                                                                                    translated_predicates,
                                                                                    this->translate(domain.get_functions()),
                                                                                    this->translate(domain.get_actions()));
    }

    loki::pddl::Problem translate_impl(const loki::pddl::ProblemImpl& problem)
    {
        // Remove :typing requirement
        auto requirements_enum_set = problem.get_requirements()->get_requirements();
        requirements_enum_set.erase(loki::pddl::RequirementEnum::TYPING);
        auto requirements = this->m_pddl_factories.requirements.get_or_create<loki::pddl::RequirementsImpl>(requirements_enum_set);

        // Make objects untyped
        auto objects = loki::pddl::ObjectList {};
        objects.reserve(problem.get_objects().size());
        auto additional_initial_literals = loki::pddl::GroundLiteralList {};
        for (const auto& object : problem.get_objects())
        {
            auto translated_object = translate_typed_object_to_untyped_object(*object);
            auto additional_literals = translate_typed_object_to_ground_literals(*object);
            objects.push_back(translated_object);
            additional_initial_literals.insert(additional_initial_literals.end(), additional_literals.begin(), additional_literals.end());
        }

        // Make constants untyped
        for (const auto& object : problem.get_domain()->get_constants())
        {
            auto additional_literals = translate_typed_object_to_ground_literals(*object);
            additional_initial_literals.insert(additional_initial_literals.end(), additional_literals.begin(), additional_literals.end());
        }

        // Translate other initial literals and add additional literals
        auto initial_literals = this->translate(problem.get_initial_literals());
        initial_literals.insert(initial_literals.end(), additional_initial_literals.begin(), additional_initial_literals.end());

        return this->m_pddl_factories.problems.get_or_create<loki::pddl::ProblemImpl>(
            this->translate(*problem.get_domain()),
            problem.get_name(),
            requirements,
            this->translate(problem.get_objects()),
            initial_literals,
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
