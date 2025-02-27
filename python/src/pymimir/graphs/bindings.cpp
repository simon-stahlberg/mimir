
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

template<typename T>
struct PyImmutable
{
    explicit PyImmutable(const T& obj) : obj_(obj) {}

    const T& obj_;  // Read-only reference
};

template<IsVertex V, IsEdge E>
void bind_static_graph(py::module_& m, const std::string& name)
{
    using GraphType = StaticGraph<V, E>;

    py::class_<GraphType>(m, name.c_str())
        .def(py::init<>())
        .def("add_vertex",
             [](GraphType& self, py::args args)
             {
                 using PropertiesTuple = typename V::VertexPropertiesTypes;  // Extract the property types
                 return std::apply([&](auto&&... unpacked_args) { return self.add_vertex(std::forward<decltype(unpacked_args)>(unpacked_args)...); },
                                   args.cast<PropertiesTuple>()  // Cast Python arguments to the expected tuple
                 );
             })
        .def("add_vertex", [](const GraphType& self, py::args args) { throw std::runtime_error("ERROR"); })
        .def("add_vertex", [](GraphType& self, const V& vertex) { return self.add_vertex(vertex); })
        .def("get_num_vertices", &GraphType::get_num_vertices);

    py::class_<PyImmutable<GraphType>>(m, (name + "Immutable").c_str())  //
        .def(py::init<const GraphType&>())
        .def("get_num_vertices", [](const PyImmutable<GraphType>& self) { return self.obj_.get_num_vertices(); });
}

void init_graphs(py::module_& m)
{
    bind_static_graph<EmptyVertex, EmptyEdge>(m, "BasicStaticGraph");
    bind_static_graph<ColoredVertex, EmptyEdge>(m, "VertexColoredStaticGraph");
    bind_static_graph<ColoredVertex, ColoredEdge>(m, "EdgeColoredStaticGraph");
}
