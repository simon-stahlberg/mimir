
#include "bindings.hpp"

using namespace mimir;

namespace py = pybind11;

void init_graphs(py::module_& m)
{
    bind_static_graph<EmptyVertex, EmptyEdge>(m, "BasicStaticGraph");
    bind_static_graph<ColoredVertex, EmptyEdge>(m, "VertexColoredStaticGraph");
    bind_static_graph<ColoredVertex, ColoredEdge>(m, "EdgeColoredStaticGraph");
}
