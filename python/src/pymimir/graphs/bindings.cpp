
#include "bindings.hpp"

using namespace mimir;

namespace py = pybind11;

void init_graphs(py::module_& m)
{
    bind_vertex<EmptyVertex>(m, PyVertexProperties<EmptyVertex>::name);
    bind_vertex<ColoredVertex>(m, PyVertexProperties<ColoredVertex>::name);
    bind_edge<EmptyEdge>(m, PyEdgeProperties<EmptyEdge>::name);
    bind_edge<ColoredEdge>(m, PyEdgeProperties<ColoredEdge>::name);
    bind_static_graph<EmptyVertex, EmptyEdge>(m, "EmptyStaticGraph");
    bind_static_graph<ColoredVertex, EmptyEdge>(m, "VertexColoredStaticGraph");
    bind_static_graph<ColoredVertex, ColoredEdge>(m, "EdgeColoredStaticGraph");
}
