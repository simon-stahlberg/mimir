#include "bindings.hpp"

#include "../init_declarations.hpp"

using namespace mimir;

namespace mimir::bindings
{

void bind_graphs(nb::module_& m)
{
    bind_vertex<graphs::EmptyVertex>(m, PyVertexProperties<graphs::EmptyVertex>::name);
    bind_vertex<graphs::ColoredVertex>(m, PyVertexProperties<graphs::ColoredVertex>::name);
    bind_edge<graphs::EmptyEdge>(m, PyEdgeProperties<graphs::EmptyEdge>::name);
    bind_edge<graphs::ColoredEdge>(m, PyEdgeProperties<graphs::ColoredEdge>::name);
    bind_static_graph<graphs::EmptyVertex, graphs::EmptyEdge>(m, "StaticEmptyGraph");
    bind_static_graph<graphs::ColoredVertex, graphs::EmptyEdge>(m, "StaticVertexColoredGraph");
    bind_static_graph<graphs::ColoredVertex, graphs::ColoredEdge>(m, "StaticEdgeColoredGraph");
}

}
