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

#ifndef MIMIR_GRAPHS_FORMATTER_DECL_HPP_
#define MIMIR_GRAPHS_FORMATTER_DECL_HPP_

#include "mimir/graphs/declarations.hpp"

namespace mimir
{
namespace graphs
{
/**
 * Declarations
 */

template<Property P>
std::ostream& operator<<(std::ostream& out, const Vertex<P>& element);

template<Property P>
std::ostream& operator<<(std::ostream& out, const Edge<P>& element);

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const StaticGraph<V, E>& element);

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticForwardGraph<G>& element);

template<IsStaticGraph G>
std::ostream& operator<<(std::ostream& out, const StaticBidirectionalGraph<G>& element);

template<IsVertex V, IsEdge E>
std::ostream& operator<<(std::ostream& out, const DynamicGraph<V, E>& element);

extern std::ostream& operator<<(std::ostream& out, const IProperty& element);

extern std::ostream& operator<<(std::ostream& out, const PropertyValue& element);

namespace color_refinement
{
extern std::ostream& operator<<(std::ostream& out, const CertificateImpl& element);
}  // end color_refinement

namespace kfwl
{
template<size_t K>
std::ostream& operator<<(std::ostream& out, const CertificateImpl<K>& element);
}  // end kfwl

namespace nauty
{
extern std::ostream& operator<<(std::ostream& out, const SparseGraph& graph);
}  // end nauty
}  // end graphs

template<mimir::graphs::IsVertex V, mimir::graphs::IsEdge E>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticGraph<V, E>& element);

template<mimir::graphs::IsStaticGraph G>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticForwardGraph<G>& element);

template<mimir::graphs::IsStaticGraph G>
std::ostream& print(std::ostream& out, const mimir::graphs::StaticBidirectionalGraph<G>& element);

template<mimir::graphs::IsVertex V, mimir::graphs::IsEdge E>
std::ostream& print(std::ostream& out, const mimir::graphs::DynamicGraph<V, E>& element);

template<mimir::graphs::Property P>
std::ostream& print(std::ostream& out, const mimir::graphs::Vertex<P>& element);

template<mimir::graphs::Property P>
std::ostream& print(std::ostream& out, const mimir::graphs::Edge<P>& element);

extern std::ostream& print(std::ostream& out, const mimir::graphs::color_refinement::CertificateImpl& element);

template<size_t K>
std::ostream& print(std::ostream& out, const mimir::graphs::kfwl::CertificateImpl<K>& element);

extern std::ostream& print(std::ostream& out, const mimir::graphs::nauty::SparseGraph& graph);

extern std::ostream& print(std::ostream& out, const mimir::graphs::IProperty& element);

extern std::ostream& print(std::ostream& out, const mimir::graphs::PropertyValue& element);
}

#endif