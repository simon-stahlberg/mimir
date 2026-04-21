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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef MIMIR_FORMALISM_TRANSLATOR_NOVELTY_COMPILATION_HPP_
#define MIMIR_FORMALISM_TRANSLATOR_NOVELTY_COMPILATION_HPP_

#include "mimir/formalism/translator/recursive_cached_base.hpp"

#include <cstddef>

namespace mimir::formalism
{

class NoveltyProblemWrapper
{
    Problem m_compiled_problem;
    std::size_t m_k;
    std::unordered_map<Object, Object, loki::Hash<Object>, loki::EqualTo<Object>> m_object_cache;
    std::unordered_map<Predicate<FluentTag>, Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> m_predicate_cache;
    std::unordered_map<Predicate<FluentTag>, Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> m_not_novel_predicate_cache;
    std::unordered_set<Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> m_not_novel_predicates;
    std::unordered_map<Action, Action, loki::Hash<Action>, loki::EqualTo<Action>> m_action_cache;

public:
    NoveltyProblemWrapper(Problem compiled_problem, std::size_t k,
                          std::unordered_map<Object, Object, loki::Hash<Object>, loki::EqualTo<Object>> object_cache,
                          std::unordered_map<Predicate<FluentTag>, Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> predicate_cache,
                          std::unordered_map<Predicate<FluentTag>, Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> not_novel_predicate_cache,
                          std::unordered_set<Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> not_novel_predicates,
                          std::unordered_map<Action, Action, loki::Hash<Action>, loki::EqualTo<Action>> action_cache);

    const Problem& get_problem() const;
    std::size_t get_k() const;

    GroundAtom<FluentTag> get_compiled_atom(GroundAtom<FluentTag> original_atom) const;
    GroundAtom<FluentTag> get_not_novel_atom(GroundAtom<FluentTag> original_atom) const;
    GroundAtomList<FluentTag> create_compiled_state_atoms(const GroundAtomList<FluentTag>& original_state_atoms,
                                                          const GroundAtomList<FluentTag>& additional_not_novel_atoms = {}) const;

    bool is_not_novel_predicate(Predicate<FluentTag> predicate) const;
    Object get_compiled_object(Object original_object) const;
    Action get_compiled_action(Action original_action) const;
};

class NoveltyTranslator : public RecursiveCachedBaseTranslator<NoveltyTranslator>
{
private:
    friend class RecursiveCachedBaseTranslator<NoveltyTranslator>;

    using RecursiveCachedBaseTranslator<NoveltyTranslator>::translate_level_2;

    std::size_t m_k;
    AxiomList m_generated_action_axioms;
    bool m_has_translated;
    std::unordered_map<Predicate<FluentTag>, Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> m_not_novel_predicate_cache;
    std::unordered_set<Predicate<FluentTag>, loki::Hash<Predicate<FluentTag>>, loki::EqualTo<Predicate<FluentTag>>> m_not_novel_predicates;

    void validate_k() const;
    void validate_domain(const Domain& domain) const;

    Requirements translate_level_2(Requirements requirements, Repositories& repositories);

    Predicate<FluentTag> get_or_create_not_novel_predicate(Predicate<FluentTag> predicate, Repositories& repositories);
    Predicate<DerivedTag> get_or_create_action_witness_predicate(Action action, Repositories& repositories);

    Domain translate_level_2(const Domain& domain, DomainBuilder& builder);
    Problem translate_level_2(const Problem& problem, ProblemBuilder& builder);
    ConjunctiveCondition translate_level_2(ConjunctiveCondition condition, Repositories& repositories);
    Action translate_level_2(Action action, Repositories& repositories);

public:
    explicit NoveltyTranslator(std::size_t k = 1);

    std::size_t get_k() const;

    NoveltyProblemWrapper translate(const Problem& problem);
};

}

#endif
