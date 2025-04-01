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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_VISITOR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_VISITOR_INTERFACE_HPP_

#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::languages::general_policies
{

class IVisitor
{
public:
    virtual ~IVisitor() = default;

    virtual void visit(PositiveBooleanCondition effect) = 0;
    virtual void visit(NegativeBooleanCondition effect) = 0;
    virtual void visit(GreaterNumericalCondition effect) = 0;
    virtual void visit(EqualNumericalCondition effect) = 0;

    virtual void visit(PositiveBooleanEffect effect) = 0;
    virtual void visit(NegativeBooleanEffect effect) = 0;
    virtual void visit(UnchangedBooleanEffect effect) = 0;
    virtual void visit(IncreaseNumericalEffect effect) = 0;
    virtual void visit(DecreaseNumericalEffect effect) = 0;
    virtual void visit(UnchangedNumericalEffect effect) = 0;

    virtual void visit(NamedFeature<dl::ConceptTag> feature) = 0;
    virtual void visit(NamedFeature<dl::RoleTag> feature) = 0;
    virtual void visit(NamedFeature<dl::BooleanTag> feature) = 0;
    virtual void visit(NamedFeature<dl::NumericalTag> feature) = 0;

    virtual void visit(Rule rule) = 0;

    virtual void visit(GeneralPolicy policy) = 0;
};
}

#endif