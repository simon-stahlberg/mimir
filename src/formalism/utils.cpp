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

#include "mimir/formalism/utils.hpp"

#include "mimir/formalism/ground_literal.hpp"

namespace mimir::formalism
{

template<IsStaticOrFluentOrDerivedTag P>
void to_ground_atoms(const GroundLiteralList<P>& literals, GroundAtomList<P>& out_ground_atoms)
{
    out_ground_atoms.clear();

    for (const auto& literal : literals)
    {
        out_ground_atoms.push_back(literal->get_atom());
    }
}

template void to_ground_atoms(const GroundLiteralList<StaticTag>& literals, GroundAtomList<StaticTag>& out_ground_atoms);
template void to_ground_atoms(const GroundLiteralList<FluentTag>& literals, GroundAtomList<FluentTag>& out_ground_atoms);
template void to_ground_atoms(const GroundLiteralList<DerivedTag>& literals, GroundAtomList<DerivedTag>& out_ground_atoms);

template<IsStaticOrFluentOrDerivedTag P>
GroundAtomList<P> to_ground_atoms(const GroundLiteralList<P>& literals)
{
    GroundAtomList<P> ground_atoms;
    to_ground_atoms(literals, ground_atoms);
    return ground_atoms;
}

template GroundAtomList<StaticTag> to_ground_atoms(const GroundLiteralList<StaticTag>& literals);
template GroundAtomList<FluentTag> to_ground_atoms(const GroundLiteralList<FluentTag>& literals);
template GroundAtomList<DerivedTag> to_ground_atoms(const GroundLiteralList<DerivedTag>& literals);

template<IsStaticOrFluentOrDerivedTag P>
void filter_ground_atoms(const GroundLiteralList<P>& literals, bool polarity, GroundAtomList<P>& out_ground_atoms)
{
    out_ground_atoms.clear();

    for (const auto& literal : literals)
    {
        if (literal->get_polarity() != polarity)
        {
            out_ground_atoms.push_back(literal->get_atom());
        }
    }
}

template void filter_ground_atoms(const GroundLiteralList<StaticTag>& literals, bool polarity, GroundAtomList<StaticTag>& out_ground_atoms);
template void filter_ground_atoms(const GroundLiteralList<FluentTag>& literals, bool polarity, GroundAtomList<FluentTag>& out_ground_atoms);
template void filter_ground_atoms(const GroundLiteralList<DerivedTag>& literals, bool polarity, GroundAtomList<DerivedTag>& out_ground_atoms);

template<IsStaticOrFluentOrDerivedTag P>
GroundAtomList<P> filter_ground_atoms(const GroundLiteralList<P>& literals, bool polarity)
{
    GroundAtomList<P> ground_atoms;
    filter_ground_atoms(literals, polarity, ground_atoms);
    return ground_atoms;
}

template GroundAtomList<StaticTag> filter_ground_atoms(const GroundLiteralList<StaticTag>& literals, bool polarity);
template GroundAtomList<FluentTag> filter_ground_atoms(const GroundLiteralList<FluentTag>& literals, bool polarity);
template GroundAtomList<DerivedTag> filter_ground_atoms(const GroundLiteralList<DerivedTag>& literals, bool polarity);
}