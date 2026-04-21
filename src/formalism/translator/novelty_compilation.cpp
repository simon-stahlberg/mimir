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

#include "mimir/formalism/translator/novelty_compilation.hpp"

#include "mimir/common/collections.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>

using namespace boost::hana;

namespace mimir::formalism
{
namespace
{

constexpr std::string_view NOT_NOVEL_PREDICATE_PREFIX = "not-novel-";
constexpr std::string_view WITNESS_PREDICATE_PREFIX = "novel-witness-";

std::string make_prefixed_name(std::string_view prefix, std::string_view name)
{
    auto result = std::string {};
    result.reserve(prefix.size() + name.size());
    result.append(prefix);
    result.append(name);
    return result;
}

std::string get_not_novel_predicate_name(std::string_view predicate_name) { return make_prefixed_name(NOT_NOVEL_PREDICATE_PREFIX, predicate_name); }

std::string get_witness_predicate_name(std::string_view action_name) { return make_prefixed_name(WITNESS_PREDICATE_PREFIX, action_name); }

template<typename T>
void sort_by_index(std::vector<T>& entities)
{
    std::sort(entities.begin(), entities.end(), [](const auto& lhs, const auto& rhs) { return lhs->get_index() < rhs->get_index(); });
}

template<typename T>
std::vector<T> uniquify_and_sort(const std::vector<T>& entities)
{
    auto result = uniquify_elements(entities);
    sort_by_index(result);
    return result;
}

ParameterList merge_parameters(const ParameterList& action_parameters, const ParameterList& condition_parameters)
{
    auto parameters = action_parameters;

    for (const auto& parameter : condition_parameters)
    {
        const auto it = std::find(parameters.begin(), parameters.end(), parameter);
        if (it == parameters.end())
        {
            parameters.push_back(parameter);
        }
    }

    return parameters;
}

TermList make_terms(const ParameterList& parameters, Repositories& repositories)
{
    auto terms = TermList {};
    terms.reserve(parameters.size());
    for (const auto& parameter : parameters)
    {
        terms.push_back(repositories.get_or_create_term(parameter->get_variable()));
    }
    return terms;
}

ConjunctiveCondition create_conjunctive_condition(ParameterList parameters,
                                                  LiteralList<StaticTag> static_literals,
                                                  LiteralList<FluentTag> fluent_literals,
                                                  LiteralList<DerivedTag> derived_literals,
                                                  NumericConstraintList numeric_constraints,
                                                  Repositories& repositories)
{
    auto literals = LiteralLists<StaticTag, FluentTag, DerivedTag> {};
    at_key(literals, type<StaticTag> {}) = uniquify_and_sort(static_literals);
    at_key(literals, type<FluentTag> {}) = uniquify_and_sort(fluent_literals);
    at_key(literals, type<DerivedTag> {}) = uniquify_and_sort(derived_literals);
    numeric_constraints = uniquify_and_sort(numeric_constraints);

    return repositories.get_or_create_conjunctive_condition(std::move(parameters), std::move(literals), std::move(numeric_constraints));
}

ConjunctiveEffect create_conjunctive_effect(ParameterList parameters,
                                            LiteralList<FluentTag> fluent_literals,
                                            NumericEffectList<FluentTag> fluent_numeric_effects,
                                            std::optional<NumericEffect<AuxiliaryTag>> auxiliary_numeric_effect,
                                            Repositories& repositories)
{
    fluent_literals = uniquify_and_sort(fluent_literals);
    sort_by_index(fluent_numeric_effects);

    return repositories.get_or_create_conjunctive_effect(std::move(parameters),
                                                         std::move(fluent_literals),
                                                         std::move(fluent_numeric_effects),
                                                         std::move(auxiliary_numeric_effect));
}

}

NoveltyProblemWrapper::NoveltyProblemWrapper(Problem compiled_problem, std::size_t k,
                                             std::unordered_map<Object, Object, loki::Hash<Object>, loki::EqualTo<Object>> object_cache,
                                             std::unordered_map<Predicate<FluentTag>, Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> predicate_cache,
                                             std::unordered_map<Predicate<FluentTag>, Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> not_novel_predicate_cache,
                                             std::unordered_set<Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> not_novel_predicates,
                                             std::unordered_map<Action, Action, loki::Hash<Action>, loki::EqualTo<Action>> action_cache) :
    m_compiled_problem(std::move(compiled_problem)),
    m_k(k),
    m_object_cache(std::move(object_cache)),
    m_predicate_cache(std::move(predicate_cache)),
    m_not_novel_predicate_cache(std::move(not_novel_predicate_cache)),
    m_not_novel_predicates(std::move(not_novel_predicates)),
    m_action_cache(std::move(action_cache))
{
}

const Problem& NoveltyProblemWrapper::get_problem() const { return m_compiled_problem; }

std::size_t NoveltyProblemWrapper::get_k() const { return m_k; }

GroundAtom<FluentTag> NoveltyProblemWrapper::get_compiled_atom(GroundAtom<FluentTag> original_atom) const
{
    auto compiled_objects = ObjectList {};
    compiled_objects.reserve(original_atom->get_objects().size());
    for (const auto& obj : original_atom->get_objects())
    {
        compiled_objects.push_back(m_object_cache.at(obj));
    }
    return m_compiled_problem->get_or_create_ground_atom(m_predicate_cache.at(original_atom->get_predicate()), compiled_objects);
}

GroundAtom<FluentTag> NoveltyProblemWrapper::get_not_novel_atom(GroundAtom<FluentTag> original_atom) const
{
    auto compiled_objects = ObjectList {};
    compiled_objects.reserve(original_atom->get_objects().size());
    for (const auto& obj : original_atom->get_objects())
    {
        compiled_objects.push_back(m_object_cache.at(obj));
    }
    return m_compiled_problem->get_or_create_ground_atom(m_not_novel_predicate_cache.at(original_atom->get_predicate()), compiled_objects);
}

bool NoveltyProblemWrapper::is_not_novel_predicate(Predicate<FluentTag> predicate) const
{
    return m_not_novel_predicates.contains(predicate);
}

Object NoveltyProblemWrapper::get_compiled_object(Object original_object) const
{
    return m_object_cache.at(original_object);
}

Action NoveltyProblemWrapper::get_compiled_action(Action original_action) const
{
    return m_action_cache.at(original_action);
}

GroundAtomList<FluentTag>
NoveltyProblemWrapper::create_compiled_state_atoms(const GroundAtomList<FluentTag>& original_state_atoms,
                                                   const GroundAtomList<FluentTag>& additional_not_novel_atoms) const
{
    auto compiled_atoms = GroundAtomList<FluentTag> {};
    compiled_atoms.reserve(original_state_atoms.size() * 2 + additional_not_novel_atoms.size());

    for (const auto& atom : original_state_atoms)
    {
        compiled_atoms.push_back(get_compiled_atom(atom));
        compiled_atoms.push_back(get_not_novel_atom(atom));
    }
    for (const auto& atom : additional_not_novel_atoms)
    {
        compiled_atoms.push_back(get_not_novel_atom(atom));
    }

    return uniquify_and_sort(compiled_atoms);
}

NoveltyTranslator::NoveltyTranslator(std::size_t k) :
    m_k(k),
    m_generated_action_axioms(),
    m_has_translated(false)
{
}

std::size_t NoveltyTranslator::get_k() const { return m_k; }

void NoveltyTranslator::validate_k() const
{
    if (m_k != 1)
    {
        throw std::runtime_error("NoveltyTranslator currently only supports k = 1.");
    }
}

void NoveltyTranslator::validate_domain(const Domain& domain) const
{
    auto predicate_names = std::unordered_set<std::string> {};

    for (const auto& predicate : domain->get_predicates<StaticTag>())
    {
        predicate_names.insert(predicate->get_name());
    }
    for (const auto& predicate : domain->get_predicates<FluentTag>())
    {
        predicate_names.insert(predicate->get_name());
    }
    for (const auto& predicate : domain->get_predicates<DerivedTag>())
    {
        predicate_names.insert(predicate->get_name());
    }

    for (const auto& predicate : domain->get_predicates<FluentTag>())
    {
        const auto compiled_name = get_not_novel_predicate_name(predicate->get_name());
        if (predicate_names.contains(compiled_name))
        {
            throw std::runtime_error("NoveltyTranslator: novelty predicate name collision for '" + compiled_name + "'.");
        }
    }
    for (const auto& action : domain->get_actions())
    {
        const auto compiled_name = get_witness_predicate_name(action->get_name());
        if (predicate_names.contains(compiled_name))
        {
            throw std::runtime_error("NoveltyTranslator: witness predicate name collision for '" + compiled_name + "'.");
        }
    }
}

Requirements NoveltyTranslator::translate_level_2(Requirements requirements, Repositories& repositories)
{
    auto compiled_requirements = requirements->get_requirements();
    compiled_requirements.insert(loki::RequirementEnum::NEGATIVE_PRECONDITIONS);
    compiled_requirements.insert(loki::RequirementEnum::DERIVED_PREDICATES);

    return repositories.get_or_create_requirements(compiled_requirements);
}

Predicate<FluentTag> NoveltyTranslator::get_or_create_not_novel_predicate(Predicate<FluentTag> predicate, Repositories& repositories)
{
    if (m_not_novel_predicate_cache.contains(predicate))
    {
        return m_not_novel_predicate_cache.at(predicate);
    }
    const auto compiled_predicate = repositories.get_or_create_predicate<FluentTag>(get_not_novel_predicate_name(predicate->get_name()),
                                                                                    this->translate_level_0(predicate->get_parameters(), repositories));
    m_not_novel_predicate_cache.emplace(predicate, compiled_predicate);
    m_not_novel_predicates.insert(compiled_predicate);
    return compiled_predicate;
}

Predicate<DerivedTag> NoveltyTranslator::get_or_create_action_witness_predicate(Action action, Repositories& repositories)
{
    return repositories.get_or_create_predicate<DerivedTag>(get_witness_predicate_name(action->get_name()),
                                                            this->translate_level_0(action->get_parameters(), repositories));
}

Domain NoveltyTranslator::translate_level_2(const Domain& domain, DomainBuilder& builder)
{
    validate_k();
    validate_domain(domain);
    m_generated_action_axioms.clear();

    auto& repositories = builder.get_repositories();

    builder.get_name() = domain->get_name();
    builder.get_filepath() = domain->get_filepath();
    builder.get_requirements() = this->translate_level_0(domain->get_requirements(), repositories);
    builder.get_types() = this->translate_level_0(domain->get_types(), repositories);
    builder.get_constants() = this->translate_level_0(domain->get_constants(), repositories);
    builder.get_predicates<StaticTag>() = this->translate_level_0(domain->get_predicates<StaticTag>(), repositories);

    auto compiled_fluent_predicates = this->translate_level_0(domain->get_predicates<FluentTag>(), repositories);
    for (const auto& predicate : domain->get_predicates<FluentTag>())
    {
        compiled_fluent_predicates.push_back(get_or_create_not_novel_predicate(predicate, repositories));
    }
    builder.get_predicates<FluentTag>() = uniquify_and_sort(compiled_fluent_predicates);

    auto compiled_derived_predicates = this->translate_level_0(domain->get_predicates<DerivedTag>(), repositories);
    for (const auto& action : domain->get_actions())
    {
        compiled_derived_predicates.push_back(get_or_create_action_witness_predicate(action, repositories));
    }
    builder.get_predicates<DerivedTag>() = uniquify_and_sort(compiled_derived_predicates);

    builder.get_function_skeletons<StaticTag>() = this->translate_level_0(domain->get_function_skeletons<StaticTag>(), repositories);
    builder.get_function_skeletons<FluentTag>() = this->translate_level_0(domain->get_function_skeletons<FluentTag>(), repositories);
    builder.get_auxiliary_function_skeleton() = this->translate_level_0(domain->get_auxiliary_function_skeleton(), repositories);
    builder.get_actions() = uniquify_and_sort(this->translate_level_0(domain->get_actions(), repositories));

    auto compiled_axioms = this->translate_level_0(domain->get_axioms(), repositories);
    compiled_axioms.insert(compiled_axioms.end(), m_generated_action_axioms.begin(), m_generated_action_axioms.end());
    builder.get_axioms() = uniquify_and_sort(compiled_axioms);

    return builder.get_result();
}

Problem NoveltyTranslator::translate_level_2(const Problem& problem, ProblemBuilder& builder)
{
    validate_k();

    auto& repositories = builder.get_repositories();

    builder.get_filepath() = problem->get_filepath();
    builder.get_name() = problem->get_name();
    builder.get_requirements() = this->translate_level_0(problem->get_requirements(), repositories);
    builder.get_objects() = this->translate_level_0(problem->get_objects(), repositories);
    builder.get_derived_predicates() = this->translate_level_0(problem->get_derived_predicates(), repositories);
    builder.get_initial_literals<StaticTag>() = this->translate_level_0(problem->get_initial_literals<StaticTag>(), repositories);

    auto compiled_initial_fluent_literals = this->translate_level_0(problem->get_initial_literals<FluentTag>(), repositories);
    for (const auto& literal : problem->get_initial_literals<FluentTag>())
    {
        if (!literal->get_polarity())
        {
            continue;
        }

        const auto compiled_not_novel_predicate = get_or_create_not_novel_predicate(literal->get_atom()->get_predicate(), repositories);
        const auto compiled_objects = this->translate_level_0(literal->get_atom()->get_objects(), repositories);
        const auto compiled_not_novel_atom = repositories.get_or_create_ground_atom(compiled_not_novel_predicate, compiled_objects);
        compiled_initial_fluent_literals.push_back(repositories.get_or_create_ground_literal(true, compiled_not_novel_atom));
    }
    builder.get_initial_literals<FluentTag>() = uniquify_and_sort(compiled_initial_fluent_literals);

    builder.get_initial_function_values<StaticTag>() = this->translate_level_0(problem->get_initial_function_values<StaticTag>(), repositories);
    builder.get_initial_function_values<FluentTag>() = this->translate_level_0(problem->get_initial_function_values<FluentTag>(), repositories);
    builder.get_auxiliary_function_value() = this->translate_level_0(problem->get_auxiliary_function_value(), repositories);
    builder.get_goal_literals<StaticTag>() = this->translate_level_0(problem->get_goal_literals<StaticTag>(), repositories);
    builder.get_goal_literals<FluentTag>() = this->translate_level_0(problem->get_goal_literals<FluentTag>(), repositories);
    builder.get_goal_literals<DerivedTag>() = this->translate_level_0(problem->get_goal_literals<DerivedTag>(), repositories);
    builder.get_goal_numeric_constraints() = this->translate_level_0(problem->get_goal_numeric_constraints(), repositories);
    builder.get_optimization_metric() = this->translate_level_0(problem->get_optimization_metric(), repositories);
    builder.get_axioms() = this->translate_level_0(problem->get_axioms(), repositories);

    return builder.get_result(problem->get_index());
}

ConjunctiveCondition NoveltyTranslator::translate_level_2(ConjunctiveCondition condition, Repositories& repositories)
{
    return create_conjunctive_condition(this->translate_level_0(condition->get_parameters(), repositories),
                                        this->translate_level_0(condition->get_literals<StaticTag>(), repositories),
                                        this->translate_level_0(condition->get_literals<FluentTag>(), repositories),
                                        this->translate_level_0(condition->get_literals<DerivedTag>(), repositories),
                                        this->translate_level_0(condition->get_numeric_constraints(), repositories),
                                        repositories);
}

Action NoveltyTranslator::translate_level_2(Action action, Repositories& repositories)
{
    auto translated_action_parameters = this->translate_level_0(action->get_parameters(), repositories);
    const auto witness_predicate = get_or_create_action_witness_predicate(action, repositories);
    const auto witness_terms = make_terms(translated_action_parameters, repositories);
    const auto witness_atom = repositories.get_or_create_atom(witness_predicate, witness_terms);
    const auto witness_literal = repositories.get_or_create_literal(true, witness_atom);

    const auto translated_precondition = this->translate_level_0(action->get_conjunctive_condition(), repositories);
    auto static_precondition_literals = translated_precondition->get_literals<StaticTag>();
    auto fluent_precondition_literals = translated_precondition->get_literals<FluentTag>();
    auto derived_precondition_literals = translated_precondition->get_literals<DerivedTag>();
    derived_precondition_literals.push_back(witness_literal);
    const auto compiled_precondition = create_conjunctive_condition(translated_precondition->get_parameters(),
                                                                   std::move(static_precondition_literals),
                                                                   std::move(fluent_precondition_literals),
                                                                   std::move(derived_precondition_literals),
                                                                   translated_precondition->get_numeric_constraints(),
                                                                   repositories);

    auto compiled_conditional_effects = ConditionalEffectList {};
    compiled_conditional_effects.reserve(action->get_conditional_effects().size());

    for (const auto& conditional_effect : action->get_conditional_effects())
    {
        const auto translated_condition = this->translate_level_0(conditional_effect->get_conjunctive_condition(), repositories);
        const auto translated_effect = this->translate_level_0(conditional_effect->get_conjunctive_effect(), repositories);

        auto compiled_effect_literals = translated_effect->get_literals();

        for (const auto& effect_literal : conditional_effect->get_conjunctive_effect()->get_literals())
        {
            if (!effect_literal->get_polarity())
            {
                continue;
            }

            const auto translated_effect_literal = this->translate_level_0(effect_literal, repositories);
            const auto compiled_not_novel_predicate = get_or_create_not_novel_predicate(effect_literal->get_atom()->get_predicate(), repositories);
            const auto compiled_not_novel_atom = repositories.get_or_create_atom(compiled_not_novel_predicate,
                                                                                 translated_effect_literal->get_atom()->get_terms());
            const auto positive_compiled_not_novel_literal = repositories.get_or_create_literal(true, compiled_not_novel_atom);
            const auto negative_compiled_not_novel_literal = repositories.get_or_create_literal(false, compiled_not_novel_atom);
            const auto negative_original_effect_literal = repositories.get_or_create_literal(false, translated_effect_literal->get_atom());

            compiled_effect_literals.push_back(positive_compiled_not_novel_literal);

            auto static_witness_literals = translated_condition->get_literals<StaticTag>();
            auto fluent_witness_literals = translated_condition->get_literals<FluentTag>();
            auto derived_witness_literals = translated_condition->get_literals<DerivedTag>();
            fluent_witness_literals.push_back(negative_original_effect_literal);
            fluent_witness_literals.push_back(negative_compiled_not_novel_literal);

            const auto witness_condition = create_conjunctive_condition(merge_parameters(translated_action_parameters,
                                                                                         translated_condition->get_parameters()),
                                                                       std::move(static_witness_literals),
                                                                       std::move(fluent_witness_literals),
                                                                       std::move(derived_witness_literals),
                                                                       translated_condition->get_numeric_constraints(),
                                                                       repositories);
            m_generated_action_axioms.push_back(repositories.get_or_create_axiom(witness_condition, witness_literal));
        }

        const auto compiled_effect = create_conjunctive_effect(translated_effect->get_parameters(),
                                                               std::move(compiled_effect_literals),
                                                               translated_effect->get_fluent_numeric_effects(),
                                                               translated_effect->get_auxiliary_numeric_effect(),
                                                               repositories);
        compiled_conditional_effects.push_back(repositories.get_or_create_conditional_effect(translated_condition, compiled_effect));
    }

    compiled_conditional_effects = uniquify_and_sort(compiled_conditional_effects);

    return repositories.get_or_create_action(action->get_name(), action->get_original_arity(), compiled_precondition, compiled_conditional_effects);
}

NoveltyProblemWrapper NoveltyTranslator::translate(const Problem& problem)
{
    validate_k();

    if (m_has_translated)
    {
        throw std::runtime_error("NoveltyTranslator instances are single-use; create a fresh translator for each problem.");
    }
    m_has_translated = true;

    auto domain_builder = DomainBuilder {};
    auto compiled_domain = this->translate_level_0(problem->get_domain(), domain_builder);

    auto problem_builder = ProblemBuilder(compiled_domain);
    auto compiled_problem = this->translate_level_0(problem, problem_builder);

    return NoveltyProblemWrapper(std::move(compiled_problem), m_k,
                                 this->get_cache<Object>(),
                                 this->get_cache<Predicate<FluentTag>>(),
                                 std::move(m_not_novel_predicate_cache),
                                 std::move(m_not_novel_predicates),
                                 this->get_cache<Action>());
}

}
