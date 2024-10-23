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

#include "mimir/graphs/algorithms/color_refinement.hpp"

#include "mimir/common/hash.hpp"

namespace mimir
{

/* ColorRefinementCertificate */
ColorRefinementCertificate::ColorRefinementCertificate(CanonicalCompressionFunction canonical_compression_function, CanonicalColoring canonical_coloring) :
    m_canonical_compression_function(std::move(canonical_compression_function)),
    m_canonical_coloring(std::move(canonical_coloring))
{
}

const ColorRefinementCertificate::CanonicalCompressionFunction& ColorRefinementCertificate::get_canonical_compression_function() const
{
    return m_canonical_compression_function;
}

const ColorRefinementCertificate::CanonicalColoring& ColorRefinementCertificate::get_canonical_coloring() const { return m_canonical_coloring; }

bool operator==(const ColorRefinementCertificate& lhs, const ColorRefinementCertificate& rhs)
{
    if (&lhs != &rhs)
    {
        return (lhs.get_canonical_coloring() == rhs.get_canonical_coloring())
               && (lhs.get_canonical_compression_function() == rhs.get_canonical_compression_function());
    }
    return true;
}

}

size_t std::hash<mimir::ColorRefinementCertificate>::operator()(const mimir::ColorRefinementCertificate& element) const
{
    return mimir::hash_combine(element.get_canonical_coloring(), element.get_canonical_compression_function());
}
