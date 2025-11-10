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

#ifndef MIMIR_GRAPHS_FORMATTER_IMPL_HPP_
#define MIMIR_GRAPHS_FORMATTER_IMPL_HPP_

#include "mimir/formalism/formatter.hpp"
#include "mimir/graphs/formatter_decl.hpp"
#include "mimir/search/formatter.hpp"

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

namespace mimir
{
namespace graphs
{
/**
 * Declarations
 */

template<Property P>
std::ostream& operator<<(std::ostream& out, const Vertex<P>& element)
{
    return mimir::print(out, element);
}

template<Property P>
std::ostream& operator<<(std::ostream& out, const Edge<P>& element)
{
    return mimir::print(out, element);
}

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const StaticGraph<V, E>& element)
{
    return mimir::print(out, element);
}

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticForwardGraph<G>& element)
{
    return mimir::print(out, element);
}

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticBidirectionalGraph<G>& element)
{
    return mimir::print(out, element);
}

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const DynamicGraph<V, E>& element)
{
    return mimir::print(out, element);
}

namespace kfwl
{
template<size_t K>
std::ostream& operator<<(std::ostream& out, const CertificateImpl<K>& element)
{
    return mimir::print(out, element);
}
}  // end kfwl

}  // end graphs

template<mimir::graphs::IsVertex V, mimir::graphs::IsEdge E>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticGraph<V, E>& element)
{
    fmt::print(out,
               "digraph Tree {{\nrankdir=TB;\n\n{}\n\n{}\n}}",
               fmt::join(element.get_vertices()
                             | std::views::transform([&](auto&& vertex) { return fmt::format("n{} [label=\"{}\"];", vertex.get_index(), to_string(vertex)); }),
                         "\n"),
               fmt::join(element.get_edges()
                             | std::views::transform(
                                 [&](auto&& edge) { return fmt::format("n{} -> n{} [label=\"{}\"];", edge.get_source(), edge.get_target(), to_string(edge)); }),
                         "\n"));

    return out;
}

template<mimir::graphs::IsStaticGraph G>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticForwardGraph<G>& element)
{
    return out << element.get_graph();
}

template<mimir::graphs::IsStaticGraph G>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticBidirectionalGraph<G>& element)
{
    return out << element.get_graph();
}

template<mimir::graphs::IsVertex V, mimir::graphs::IsEdge E>
std::ostream& print(std::ostream& out, const mimir::graphs::DynamicGraph<V, E>& element)
{
    fmt::print(
        out,
        "digraph Tree {{\nrankdir=TB;\n\n{}\n\n{}\n}}",
        fmt::join(element.get_vertices()
                      | std::views::transform([&](auto&& arg) { return fmt::format("n{} [label=\"{}\"];", arg.second.get_index(), to_string(arg.second)); }),
                  "\n"),
        fmt::join(element.get_edges()
                      | std::views::transform(
                          [&](auto&& arg)
                          { return fmt::format("n{} -> n{} [label=\"{}\"];", arg.second.get_source(), arg.second.get_target(), to_string(arg.second)); }),
                  "\n"));

    return out;
}

template<mimir::graphs::Property P>
std::ostream& print(std::ostream& out, const mimir::graphs::Vertex<P>& element)
{
    fmt::print(out, "index={}, properties={}", element.get_index(), to_string(element.get_properties()));
    return out;
}

template<mimir::graphs::Property P>
std::ostream& print(std::ostream& out, const mimir::graphs::Edge<P>& element)
{
    fmt::print(out, "index={}, properties={}", element.get_index(), to_string(element.get_properties()));
    return out;
}

template<size_t K>
std::ostream& print(std::ostream& out, const mimir::graphs::kfwl::CertificateImpl<K>& element)
{
    fmt::print(out,
               "Certificate{}FWL("
               "abstract color compression function={}"
               ", canonical configuration compression_function={}",
               ", hash to color={})",
               K,
               to_string(element.get_canonical_color_compression_function()),
               to_string(element.get_canonical_configuration_compression_function()),
               to_string(element.get_hash_to_color()));

    return out;
}
}

#endif