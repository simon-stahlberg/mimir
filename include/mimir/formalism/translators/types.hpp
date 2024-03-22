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

    std::unordered_map<loki::pddl::Type, loki::pddl::Predicate> m_type_to_predicate;

    void prepare_impl(const loki::pddl::DomainImpl& domain)
    {
        this->prepare(*domain.get_requirements());
        for (const auto& type : domain.get_types())
        {
            auto predicate = this->m_pddl_factories.predicates.get_or_create<loki::pddl::PredicateImpl>(
                type->get_name(),
                loki::pddl::ParameterList { this->m_pddl_factories.parameters.get_or_create<loki::pddl::ParameterImpl>(
                    this->m_pddl_factories.variables.get_or_create<loki::pddl::VariableImpl>("?arg1"),
                    loki::pddl::TypeList {}) });
            m_type_to_predicate.emplace(type, predicate);
        }
        this->prepare(domain.get_constants());
        this->prepare(domain.get_predicates());
        this->prepare(domain.get_functions());
        this->prepare(domain.get_actions());
    }

    loki::pddl::Domain translate_impl(const loki::pddl::DomainImpl& domain)
    {
        auto predicates = this->translate(domain.get_predicates());
        for (const auto [type, predicate] : m_type_to_predicate)
        {
            predicates.push_back(predicate);
        }
        return this->m_pddl_factories.domains.get_or_create<loki::pddl::DomainImpl>(domain.get_name(),
                                                                                    this->translate(*domain.get_requirements()),
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
