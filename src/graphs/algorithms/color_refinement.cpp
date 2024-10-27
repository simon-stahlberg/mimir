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

namespace mimir::color_refinement
{

/* Certificate */
Certificate::Certificate(CompressionFunction f, ColorList hash_to_color) :
    m_hash_to_color(std::move(hash_to_color)),
    m_f(f.begin(), f.end()),
    m_canonical_coloring(m_hash_to_color.begin(), m_hash_to_color.end())
{
    std::sort(m_canonical_coloring.begin(), m_canonical_coloring.end());
}

const ColorList& Certificate::get_hash_to_color() const { return m_hash_to_color; }

const Certificate::CanonicalCompressionFunction& Certificate::get_canonical_compression_function() const { return m_f; }

const ColorList& Certificate::get_canonical_coloring() const { return m_canonical_coloring; }

bool operator==(const Certificate& lhs, const Certificate& rhs)
{
    if (&lhs != &rhs)
    {
        return (lhs.get_canonical_coloring() == rhs.get_canonical_coloring())
               && (lhs.get_canonical_compression_function() == rhs.get_canonical_compression_function());
    }
    return true;
}

std::ostream& operator<<(std::ostream& out, const Certificate& element)
{
    out << "CertificateColorRefinement("
        << "canonical_coloring=" << element.get_canonical_coloring() << ", "
        << "canonical_compression_function=" << element.get_canonical_compression_function() << ")";
    return out;
}

}

size_t std::hash<mimir::color_refinement::Certificate>::operator()(const mimir::color_refinement::Certificate& element) const
{
    return mimir::hash_combine(element.get_canonical_coloring(), element.get_canonical_compression_function());
}
