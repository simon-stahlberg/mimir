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

    loki::pddl::Predicate translate_type_to_predicate(const loki::pddl::TypeImpl& type)
    {
        return this->m_pddl_factories.predicates.get_or_create<loki::pddl::PredicateImpl>(
            type.get_name(),
            loki::pddl::ParameterList { this->m_pddl_factories.parameters.get_or_create<loki::pddl::ParameterImpl>(
                this->m_pddl_factories.variables.get_or_create<loki::pddl::VariableImpl>("?arg"),
                loki::pddl::TypeList {}) });
    }

    loki::pddl::Object translate_object_to_untyped_object(const loki::pddl::ObjectImpl& object) {}

    void collect_types_from_type_hierarchy_recursively(const loki::pddl::Type& type, std::unordered_set<loki::pddl::Type>& ref_type_list)
    {
        ref_type_list.insert(type);
        for (const auto& base_type : type->get_bases())
        {
            collect_types_from_type_hierarchy_recursively(base_type, ref_type_list);
        }
    }

    loki::pddl::TypeList collect_types_from_type_hierarchy(const loki::pddl::TypeList& type_list)
    {
        std::unordered_set<loki::pddl::Type> flat_type_set;
        for (const auto& type : type_list)
        {
            collect_types_from_type_hierarchy_recursively(type, flat_type_set);
        }
        return loki::pddl::TypeList(flat_type_set.begin(), flat_type_set.end());
    }

    loki::pddl::Domain translate_impl(const loki::pddl::DomainImpl& domain)
    {
        // Remove requirement
        auto requirements_enum_set = domain.get_requirements()->get_requirements();
        requirements_enum_set.erase(loki::pddl::RequirementEnum::TYPING);
        auto requirements = this->m_pddl_factories.requirements.get_or_create<loki::pddl::RequirementsImpl>(requirements_enum_set);

        // Add type predicates
        auto predicates = this->translate(domain.get_predicates());
        for (const auto type : collect_types_from_type_hierarchy(domain.get_types()))
        {
            predicates.push_back(translate_type_to_predicate(*type));
        }
        return this->m_pddl_factories.domains.get_or_create<loki::pddl::DomainImpl>(domain.get_name(),
                                                                                    requirements,
                                                                                    loki::pddl::TypeList {},
                                                                                    this->translate(domain.get_constants()),
                                                                                    predicates,
                                                                                    this->translate(domain.get_functions()),
                                                                                    this->translate(domain.get_actions()));
    }

public:
    explicit TypeTranslator(loki::PDDLFactories& pddl_factories) : BaseTranslator<TypeTranslator>(pddl_factories) {}
};

}
#endif
