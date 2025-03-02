
#ifndef MIMIR_GRAPHS_BINDINGS_HPP
#define MIMIR_GRAPHS_BINDINGS_HPP

#include "init_declarations.hpp"
#include "mimir/graphs.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace mm = mimir;
namespace py = pybind11;

template<mm::IsVertex V>
struct PyVertexProperties
{
};

template<>
struct PyVertexProperties<mm::EmptyVertex>
{
    static constexpr std::string name = "EmptyVertex";
};

template<>
struct PyVertexProperties<mm::ColoredVertex>
{
    static constexpr std::string name = "ColoredVertex";
};

template<mm::IsEdge E>
struct PyEdgeProperties
{
};

template<>
struct PyEdgeProperties<mm::EmptyEdge>
{
    static constexpr std::string name = "EmptyEdge";
};

template<>
struct PyEdgeProperties<mm::ColoredEdge>
{
    static constexpr std::string name = "ColoredEdge";
};

template<typename T>
struct PyImmutable
{
    explicit PyImmutable(const T& obj) : obj_(obj) {}

    const T& obj_;  // Read-only reference
};

template<typename TranslatedGraphType, typename GraphType>
void bind_translated_static_graph(py::module_& m, const std::string& name, const std::string& prefix)
{
    using PyImmutableGraph = PyImmutable<GraphType>;

    py::class_<TranslatedGraphType>(m, (prefix + name).c_str())
        .def(py::init<GraphType>())  // From mutable graph
        .def("__init__",
             [](TranslatedGraphType& self, const PyImmutableGraph& immutable) { new (&self) TranslatedGraphType(immutable.obj_); })  // From immutable graph
        .def("get_num_vertices", &TranslatedGraphType::get_num_vertices)
        .def("get_num_edges", &TranslatedGraphType::get_num_edges);
}

template<mm::IsVertex V, mm::IsEdge E>
void bind_static_graph(py::module_& m, const std::string& name)
{
    /**
     * Mutable version
     */

    using GraphType = mm::StaticGraph<V, E>;

    py::class_<GraphType>(m, name.c_str())
        .def(py::init<>())
        .def("add_vertex",
             [](GraphType& self, py::args args)
             {
                 using PropertiesTuple = typename V::VertexPropertiesTypes;
                 return std::apply([&](auto&&... unpacked_args) { return self.add_vertex(std::forward<decltype(unpacked_args)>(unpacked_args)...); },
                                   args.cast<PropertiesTuple>());
             })
        .def("add_vertex", [](GraphType& self, const V& vertex) { return self.add_vertex(vertex); })
        .def("add_directed_edge",
             [](GraphType& self, mm::VertexIndex source, mm::VertexIndex target, py::args args)
             {
                 using PropertiesTuple = typename E::EdgePropertiesTypes;
                 return std::apply([&](auto&&... unpacked_args)
                                   { return self.add_directed_edge(source, target, std::forward<decltype(unpacked_args)>(unpacked_args)...); },
                                   args.cast<PropertiesTuple>());
             })
        .def("add_directed_edge",
             [](GraphType& self, mm::VertexIndex source, mm::VertexIndex target, const E& edge) { return self.add_directed_edge(source, target, edge); })
        .def("get_num_vertices", &GraphType::get_num_vertices)
        .def("get_num_edges", &GraphType::get_num_edges);

    /**
     * Immutable version
     */

    py::class_<PyImmutable<GraphType>>(m, (name + "Immutable").c_str())  //
        .def(py::init<const GraphType&>())
        .def("get_num_vertices", [](const PyImmutable<GraphType>& self) { return self.obj_.get_num_vertices(); })
        .def("get_num_edges", [](const PyImmutable<GraphType>& self) { return self.obj_.get_num_edges(); });

    /**
     * Immutable forward version
     */

    using ForwardGraphType = mm::StaticForwardGraph<GraphType>;

    bind_translated_static_graph<ForwardGraphType, GraphType>(m, name, "Forward");

    /**
     * Immutable bidirectional version
     */

    using BidirectionalGraphType = mm::StaticBidirectionalGraph<GraphType>;

    bind_translated_static_graph<BidirectionalGraphType, GraphType>(m, name, "Bidirectional");
}

#endif