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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_VISITOR_FORMATTER_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_VISITOR_FORMATTER_HPP_

#include "mimir/languages/general_policies/visitor_interface.hpp"

namespace mimir::languages::general_policies
{
class FormatterVisitor : public IVisitor
{
private:
    std::ostream& m_out;

public:
    explicit FormatterVisitor(std::ostream& out);

    void visit(PositiveBooleanCondition effect) override;
    void visit(NegativeBooleanCondition effect) override;
    void visit(GreaterNumericalCondition effect) override;
    void visit(EqualNumericalCondition effect) override;

    void visit(PositiveBooleanEffect effect) override;
    void visit(NegativeBooleanEffect effect) override;
    void visit(UnchangedBooleanEffect effect) override;
    void visit(IncreaseNumericalEffect effect) override;
    void visit(DecreaseNumericalEffect effect) override;
    void visit(UnchangedNumericalEffect effect) override;

    void visit(NamedFeature<dl::ConceptTag> feature) override;
    void visit(NamedFeature<dl::RoleTag> feature) override;
    void visit(NamedFeature<dl::BooleanTag> feature) override;
    void visit(NamedFeature<dl::NumericalTag> feature) override;

    void visit(Rule rule) override;

    void visit(GeneralPolicy policy) override;
};

extern std::ostream& operator<<(std::ostream& out, Condition condition);

extern std::ostream& operator<<(std::ostream& out, Effect effect);

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
extern std::ostream& operator<<(std::ostream& out, NamedFeature<D> feature);

extern std::ostream& operator<<(std::ostream& out, Rule rule);

extern std::ostream& operator<<(std::ostream& out, GeneralPolicy policy);
}

#endif