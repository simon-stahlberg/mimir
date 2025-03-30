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

#ifndef MIMIR_FORMALISM_TRANSLATOR_ENCODE_PARAMETER_INDEX_IN_VARIABLES_HPP_
#define MIMIR_FORMALISM_TRANSLATOR_ENCODE_PARAMETER_INDEX_IN_VARIABLES_HPP_

#include "mimir/formalism/translator/recursive_base.hpp"

#include <unordered_map>

namespace mimir::formalism
{

/**
 * Encode parameter indices to variables needed in the lifted applicable action generator for checking vertex and edge consistency of literals.
 */
class EncodeParameterIndexInVariables : public RecursiveBaseTranslator<EncodeParameterIndexInVariables>
{
private:
    /* Implement RecursiveBaseTranslator interface. */
    friend class RecursiveBaseTranslator<EncodeParameterIndexInVariables>;

    // Provide default implementations
    using RecursiveBaseTranslator<EncodeParameterIndexInVariables>::translate_level_2;

    std::unordered_map<Variable, size_t> m_variable_to_parameter_index;
    bool m_enable_encoding;

    Variable translate_level_2(Variable variable, Repositories& repositories);

    // Do not rename within predicates or function skeleton
    template<IsStaticOrFluentOrDerivedTag P>
    Predicate<P> translate_level_2(Predicate<P> predicate, Repositories& repositories);
    template<IsStaticOrFluentOrAuxiliaryTag F>
    FunctionSkeleton<F> translate_level_2(FunctionSkeleton<F> function_skeleton, Repositories& repositories);

    ConditionalEffect translate_level_2(ConditionalEffect effect, Repositories& repositories);
    Axiom translate_level_2(Axiom axiom, Repositories& repositories);
    Action translate_level_2(Action action, Repositories& repositories);
};
}

#endif