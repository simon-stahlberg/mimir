/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/languages/description_logics/generator.hpp"

#include "mimir/formalism/problem.hpp"
#include "mimir/search/state.hpp"

namespace mimir::dl
{

VariadicConstructorFactory create_default_variadic_constructor_factory()
{
    return VariadicConstructorFactory(ConceptBotFactory(),
                                      ConceptTopFactory(),
                                      ConceptAtomicStateFactory<Static>(),
                                      ConceptAtomicStateFactory<Fluent>(),
                                      ConceptAtomicStateFactory<Derived>(),
                                      ConceptAtomicGoalFactory<Static>(),
                                      ConceptAtomicGoalFactory<Fluent>(),
                                      ConceptAtomicGoalFactory<Derived>(),
                                      ConceptIntersectionFactory(),
                                      ConceptUnionFactory(),
                                      ConceptNegationFactory(),
                                      ConceptValueRestrictionFactory(),
                                      ConceptExistentialQuantificationFactory(),
                                      ConceptRoleValueMapContainmentFactory(),
                                      ConceptRoleValueMapEqualityFactory(),
                                      ConceptNominalFactory(),
                                      RoleUniversalFactory(),
                                      RolePredicateStateFactory<Static>(),
                                      RolePredicateStateFactory<Fluent>(),
                                      RolePredicateStateFactory<Derived>(),
                                      RolePredicateGoalFactory<Static>(),
                                      RolePredicateGoalFactory<Fluent>(),
                                      RolePredicateGoalFactory<Derived>(),
                                      RoleIntersectionFactory(),
                                      RoleUnionFactory(),
                                      RoleComplementFactory(),
                                      RoleInverseFactory(),
                                      RoleCompositionFactory(),
                                      RoleTransitiveClosureFactory(),
                                      RoleReflexiveTransitiveClosureFactory(),
                                      RoleRestrictionFactory(),
                                      RoleIdentityFactory());
}

/**
 * GeneratorStateListPruningFunction
 */

GeneratorStateListPruningFunction::GeneratorStateListPruningFunction(const PDDLFactories& pddl_factories, Problem problem, StateList states) :
    GeneratorPruningFunction(),
    m_pddl_factories(pddl_factories),
    m_problem(problem),
    m_states(std::move(states)),
    m_concept_denotation_builder(),
    m_role_denotation_builder(),
    m_concept_denotation_repository(),
    m_role_denotation_repository()

{
    m_role_denotation_builder.get_data().resize(m_problem->get_objects().size());
}

bool GeneratorStateListPruningFunction::should_prune(Constructor<Concept> concept_) { return should_prune_impl(concept_); }

bool GeneratorStateListPruningFunction::should_prune(Constructor<Role> role_) { return should_prune_impl(role_); }

template<IsConceptOrRole D>
bool GeneratorStateListPruningFunction::should_prune_impl(Constructor<D> constructor)
{
    auto denotations = DenotationsList();
    denotations.reserve(m_states.size());

    for (const auto& state : m_states)
    {
        auto evaluation_context = EvaluationContext(m_pddl_factories,
                                                    m_problem,
                                                    state,
                                                    m_concept_denotation_builder,
                                                    m_role_denotation_builder,
                                                    m_concept_denotation_repository,
                                                    m_role_denotation_repository);

        const auto eval = constructor->evaluate(evaluation_context);

        denotations.push_back(reinterpret_cast<uintptr_t>(eval));
    }

    return !m_denotations_repository.insert(denotations).second;
}

template bool GeneratorStateListPruningFunction::should_prune_impl(Constructor<Concept> constructor);
template bool GeneratorStateListPruningFunction::should_prune_impl(Constructor<Role> constructor);

std::tuple<ConstructorList<Concept>, ConstructorList<Role>> generate(const grammar::Grammar grammar,
                                                                     const GeneratorOptions& options,
                                                                     VariadicConstructorFactory& ref_constructor_repos,
                                                                     GeneratorPruningFunction& ref_pruning_function)
{
    auto generated_concepts = ConstructorList<Concept>();
    auto generated_roles = ConstructorList<Role>();

    // TODO: implement generator logic

    return std::tie(generated_concepts, generated_roles);
}

}
