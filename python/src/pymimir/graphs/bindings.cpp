
#include "bindings.hpp"

using namespace mimir;

namespace py = pybind11;

void init_graphs(py::module_& m)
{
    bind_vertex<EmptyVertex>(m, "EmptyVertex");
    bind_vertex<ColoredVertex>(m, "ColoredVertex");
    bind_edge<EmptyEdge>(m, "EmptyEdge");
    bind_edge<ColoredEdge>(m, "ColoredEdge");
    bind_static_graph<EmptyVertex, EmptyEdge>(m, "EmptyStaticGraph");
    bind_static_graph<ColoredVertex, EmptyEdge>(m, "VertexColoredStaticGraph");
    bind_static_graph<ColoredVertex, ColoredEdge>(m, "EdgeColoredStaticGraph");
}
