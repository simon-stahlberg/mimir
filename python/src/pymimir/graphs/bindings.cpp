
#include "init_declarations.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

using namespace mimir;

namespace py = pybind11;

template<IsVertex V>
struct PyVertexProperties
{
};

template<>
struct PyVertexProperties<EmptyVertex>
{
    static constexpr std::string name = "EmptyVertex";
};

template<>
struct PyVertexProperties<ColoredVertex>
{
    static constexpr std::string name = "ColoredVertex";
};

template<IsEdge E>
struct PyEdgeProperties
{
};

template<>
struct PyEdgeProperties<EmptyEdge>
{
    static constexpr std::string name = "EmptyEdge";
};

template<>
struct PyEdgeProperties<ColoredEdge>
{
    static constexpr std::string name = "ColoredEdge";
};

template<IsVertex V, IsEdge E>
void bind_static_graph(py::module_& m)
{
    std::string class_name = PyVertexProperties<V>::name + PyEdgeProperties<E>::name + "StaticGraph";

    py::class_<StaticGraph<V, E>>(m, class_name.c_str())  //
        .def(py::init<>())
        .def("add_vertex",
             [](StaticGraph<V, E>& self, py::args args)
             {
                 using PropertiesTuple = typename V::VertexPropertiesTypes;  // Extract the property types
                 return std::apply([&](auto&&... unpacked_args) { return self.add_vertex(std::forward<decltype(unpacked_args)>(unpacked_args)...); },
                                   args.cast<PropertiesTuple>()  // Cast Python arguments to the expected tuple
                 );
             })
        .def("add_vertex", [](const StaticGraph<V, E>& self, py::args args) { throw std::runtime_error("ERROR"); })
        .def("add_vertex", [](StaticGraph<V, E>& self, const V& vertex) { return self.add_vertex(vertex); });
}

void init_graphs(py::module_& m)
{
    bind_static_graph<EmptyVertex, EmptyEdge>(m);      ///< EmptyVertexEmptyEdgeStaticGraph
    bind_static_graph<ColoredVertex, EmptyEdge>(m);    ///< ColoredVertexEmptyEdgeStaticGraph
    bind_static_graph<ColoredVertex, ColoredEdge>(m);  ///< ColoredVertexColoredEdgeStaticGraph
}
