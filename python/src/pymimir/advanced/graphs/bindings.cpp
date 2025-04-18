#include "bindings.hpp"

#include "../init_declarations.hpp"

#include <nanobind/trampoline.h>

using namespace mimir;
using namespace mimir::graphs;

class IPyColor : public IColor
{
public:
    NB_TRAMPOLINE(IColor, 4);

    bool operator==(const IColor& other) const override { NB_OVERRIDE_PURE(operator==, other); }
    bool operator<(const IColor& other) const override { NB_OVERRIDE_PURE(operator<, other); }
    std::string str() const override { NB_OVERRIDE_PURE(str); }
    size_t hash() const override { NB_OVERRIDE_PURE(hash); }
};

namespace mimir::bindings
{

void bind_graphs(nb::module_& m)
{
    bind_vertex<EmptyVertex>(m, PyVertexProperties<EmptyVertex>::name);
    bind_vertex<ColoredVertex>(m, PyVertexProperties<ColoredVertex>::name);
    bind_edge<EmptyEdge>(m, PyEdgeProperties<EmptyEdge>::name);
    bind_edge<ColoredEdge>(m, PyEdgeProperties<ColoredEdge>::name);
    bind_static_graph<EmptyVertex, EmptyEdge>(m, "StaticEmptyGraph");
    bind_static_graph<ColoredVertex, EmptyEdge>(m, "StaticVertexColoredGraph");
    bind_static_graph<ColoredVertex, ColoredEdge>(m, "StaticEdgeColoredGraph");

    nb::class_<IColor, IPyColor>(m, "IColor")  //
        .def(nb::init<>())
        .def("__eq__", &IColor::operator==)
        .def("__lt__", &IColor::operator<)
        .def("__str__", &IColor::str)
        .def("__hash__", &IColor::hash);

    nb::class_<Color>(m, "Color")                   //
        .def(nb::init<std::shared_ptr<IColor>>());  // This leaks!

    nb::class_<nauty::SparseGraph>(m, "NautySparseGraph")
        .def(nb::init<StaticGraph<ColoredVertex, EmptyEdge>>())
        .def(nb::init<StaticForwardGraph<StaticGraph<ColoredVertex, EmptyEdge>>>())
        .def(nb::init<StaticBidirectionalGraph<StaticGraph<ColoredVertex, EmptyEdge>>>())
        .def("__eq__", [](const nauty::SparseGraph& lhs, const nauty::SparseGraph& rhs) { return loki::EqualTo<nauty::SparseGraph>()(lhs, rhs); })
        .def("__hash__", [](const nauty::SparseGraph& self) { return loki::Hash<nauty::SparseGraph>()(self); })
        .def("canonize", &nauty::SparseGraph::canonize);
}

}
