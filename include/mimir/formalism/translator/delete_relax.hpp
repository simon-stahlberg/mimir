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

#ifndef MIMIR_FORMALISM_TRANSLATOR_DELETE_RELAX_HPP_
#define MIMIR_FORMALISM_TRANSLATOR_DELETE_RELAX_HPP_

#include "mimir/formalism/translator/recursive_cached_base.hpp"

namespace mimir::formalism
{

class DeleteRelaxTranslator : public RecursiveCachedBaseTranslator<DeleteRelaxTranslator>
{
private:
    // There can be a one to many relationship between relaxed and unrelaxed.
    std::unordered_map<Action, ActionList> m_delete_to_normal_actions;
    std::unordered_map<Axiom, AxiomList> m_delete_to_normal_axioms;

    /* Implement RecursiveCachedBaseTranslator interface. */
    friend class RecursiveCachedBaseTranslator<DeleteRelaxTranslator>;

    // Provide default implementations
    using RecursiveCachedBaseTranslator<DeleteRelaxTranslator>::translate_level_2;

    template<StaticOrFluentOrDerived P>
    LiteralList<P> translate_level_2(const LiteralList<P>& literals, PDDLRepositories& repositories);
    ConditionalEffectList translate_level_2(const ConditionalEffectList& effects, PDDLRepositories& repositories);
    ActionList translate_level_2(const ActionList& actions, PDDLRepositories& repositories);
    AxiomList translate_level_2(const AxiomList& axioms, PDDLRepositories& repositories);

    template<StaticOrFluentOrDerived P>
    Literal<P> translate_level_2(Literal<P> literal, PDDLRepositories& repositories);
    ConjunctiveCondition translate_level_2(ConjunctiveCondition condition, PDDLRepositories& repositories);
    ConjunctiveEffect translate_level_2(ConjunctiveEffect effect, PDDLRepositories& repositories);
    ConditionalEffect translate_level_2(ConditionalEffect effect, PDDLRepositories& repositories);
    Action translate_level_2(Action action, PDDLRepositories& repositories);
    Axiom translate_level_2(Axiom axiom, PDDLRepositories& repositories);

public:
    const ActionList& get_unrelaxed_actions(Action action) const;

    const AxiomList& get_unrelaxed_axioms(Axiom axiom) const;
};

}

#endif