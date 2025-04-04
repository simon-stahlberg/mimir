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

namespace mimir::graphs::color_refinement
{

/* CertificateImpl */
CertificateImpl::CertificateImpl(CompressionFunction f, ColorList hash_to_color) :
    m_hash_to_color(std::move(hash_to_color)),
    m_f(f.begin(), f.end()),
    m_canonical_coloring(m_hash_to_color.begin(), m_hash_to_color.end())
{
    std::sort(m_canonical_coloring.begin(), m_canonical_coloring.end());
}

const ColorList& CertificateImpl::get_hash_to_color() const { return m_hash_to_color; }

const CertificateImpl::CanonicalCompressionFunction& CertificateImpl::get_canonical_compression_function() const { return m_f; }

const ColorList& CertificateImpl::get_canonical_coloring() const { return m_canonical_coloring; }

bool operator==(const CertificateImpl& lhs, const CertificateImpl& rhs) { return loki::EqualTo<CertificateImpl>()(lhs, rhs); }

std::ostream& operator<<(std::ostream& out, const CertificateImpl& element)
{
    out << "CertificateColorRefinement(" << "canonical_coloring=" << element.get_canonical_coloring() << ", "
        << "canonical_compression_function=" << element.get_canonical_compression_function() << ")";
    return out;
}

}
