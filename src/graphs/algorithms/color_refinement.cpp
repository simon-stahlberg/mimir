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
CertificateImpl::CertificateImpl(CanonicalColorCompressionFunction c, ConfigurationCompressionFunction f, ColorIndexList hash_to_color) :
    m_c(std::move(c)),
    m_f(f.begin(), f.end()),
    m_hash_to_color(std::move(hash_to_color))
{
}

const CertificateImpl::CanonicalColorCompressionFunction& CertificateImpl::get_canonical_color_compression_function() const { return m_c; }

const CertificateImpl::CanonicalConfigurationCompressionFunction& CertificateImpl::get_canonical_configuration_compression_function() const { return m_f; }

const ColorIndexList& CertificateImpl::get_hash_to_color() const { return m_hash_to_color; }

bool operator==(const CertificateImpl& lhs, const CertificateImpl& rhs) { return loki::EqualTo<CertificateImpl>()(lhs, rhs); }

std::ostream& operator<<(std::ostream& out, const CertificateImpl& element)
{
    out << "CertificateColorRefinement(" << "abstract_color_compression_function=";
    mimir::operator<<(out, element.get_canonical_color_compression_function());
    out << ", " << "canonical_compression_function=";
    mimir::operator<<(out, element.get_canonical_configuration_compression_function());
    out << ", hash_to_color=";
    mimir::operator<<(out, element.get_hash_to_color());
    out << ")";
    return out;
}

}
