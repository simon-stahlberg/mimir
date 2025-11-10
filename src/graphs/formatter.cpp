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

#include "mimir/common/formatter.hpp"

#include "algorithms/nauty_sparse_impl.hpp"
#include "mimir/formalism/formatter.hpp"
#include "mimir/graphs/algorithms/color_refinement.hpp"
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/formatter.hpp"
#include "mimir/search/formatter.hpp"

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

namespace mimir
{

namespace graphs
{
std::ostream& operator<<(std::ostream& out, const IProperty& property) { return mimir::print(out, property); }

std::ostream& operator<<(std::ostream& out, const PropertyValue& property) { return mimir::print(out, property); }

namespace color_refinement
{
std::ostream& operator<<(std::ostream& out, const CertificateImpl& element) { return mimir::print(out, element); }
}  // end color_refinement

namespace nauty
{
std::ostream& operator<<(std::ostream& out, const SparseGraph& element) { return mimir::print(out, element); }
}  // end nauty
}  // end graphs

std::ostream& print(std::ostream& out, const mimir::graphs::color_refinement::CertificateImpl& element)
{
    fmt::print(out,
               "CertificateColorRefinement("
               "abstract color compression function={}"
               ", canonical compression function={}"
               ", hash to color={})",
               to_string(element.get_canonical_color_compression_function()),
               to_string(element.get_canonical_configuration_compression_function()),
               to_string(element.get_hash_to_color()));

    return out;
}

std::ostream& print(std::ostream& out, const mimir::graphs::nauty::SparseGraph& element) { return mimir::print(out, *element.m_impl); }

std::ostream& print(std::ostream& out, const mimir::graphs::IProperty& element)
{
    out << element.str();
    return out;
}

std::ostream& print(std::ostream& out, const mimir::graphs::PropertyValue& element)
{
    out << element.str();
    return out;
}
}
