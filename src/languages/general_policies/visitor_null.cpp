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

#include "mimir/languages/general_policies/visitor_null.hpp"

namespace mimir::languages::general_policies
{
void NullVisitor::visit(PositiveBooleanCondition effect) {}
void NullVisitor::visit(NegativeBooleanCondition effect) {}
void NullVisitor::visit(GreaterNumericalCondition effect) {}
void NullVisitor::visit(EqualNumericalCondition effect) {}

void NullVisitor::visit(PositiveBooleanEffect effect) {}
void NullVisitor::visit(NegativeBooleanEffect effect) {}
void NullVisitor::visit(UnchangedBooleanEffect effect) {}
void NullVisitor::visit(IncreaseNumericalEffect effect) {}
void NullVisitor::visit(DecreaseNumericalEffect effect) {}
void NullVisitor::visit(UnchangedNumericalEffect effect) {}

void NullVisitor::visit(NamedFeature<dl::ConceptTag> feature) {}
void NullVisitor::visit(NamedFeature<dl::RoleTag> feature) {}
void NullVisitor::visit(NamedFeature<dl::BooleanTag> feature) {}
void NullVisitor::visit(NamedFeature<dl::NumericalTag> feature) {}

void NullVisitor::visit(Rule rule) {}

void NullVisitor::visit(GeneralPolicy policy) {}
}
