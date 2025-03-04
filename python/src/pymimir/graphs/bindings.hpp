
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

///////////////////////////////////////////////////////////////////////////////
/// Vertex
///////////////////////////////////////////////////////////////////////////////

template<mm::IsVertex V, std::size_t... Is>
void bind_vertex_get_properties(py::class_<V>& cls, std::index_sequence<Is...>)
{
    (cls.def(("get_property_" + std::to_string(Is)).c_str(), [](const V& v) { return v.template get_property<Is>(); }), ...);
}

template<mm::IsVertex V>
void bind_vertex(py::module_& m, const std::string& name)
{
    auto cls = py::class_<V>(m, name.c_str())  //
                   .def("get_index", &V::get_index);

    constexpr std::size_t N = std::tuple_size<typename V::VertexPropertiesTypes>::value;
    bind_vertex_get_properties<V>(cls, std::make_index_sequence<N> {});
}

///////////////////////////////////////////////////////////////////////////////
/// Edge
///////////////////////////////////////////////////////////////////////////////

template<mm::IsEdge E, std::size_t... Is>
void bind_edge_get_properties(py::class_<E>& cls, std::index_sequence<Is...>)
{
    (cls.def(("get_property_" + std::to_string(Is)).c_str(), [](const E& v) { return v.template get_property<Is>(); }), ...);
}

template<mm::IsEdge E>
void bind_edge(py::module_& m, const std::string& name)
{
    auto cls = py::class_<E>(m, name.c_str())  //
                   .def("get_index", &E::get_index)
                   .def("get_source", &E::get_source)
                   .def("get_target", &E::get_target);

    constexpr std::size_t N = std::tuple_size<typename E::EdgePropertiesTypes>::value;
    bind_edge_get_properties<E>(cls, std::make_index_sequence<N> {});
}

///////////////////////////////////////////////////////////////////////////////
/// StaticGraph
///////////////////////////////////////////////////////////////////////////////

template<typename TranslatedGraphType, typename GraphType>
void bind_translated_static_graph(py::module_& m, const std::string& name, const std::string& prefix)
{
    using PyImmutableGraph = PyImmutable<GraphType>;

    py::class_<TranslatedGraphType>(m, (prefix + name).c_str())
        .def(py::init<GraphType>())
        .def(py::init([](const PyImmutableGraph& immutable) { return TranslatedGraphType(immutable.obj_); }))
        .def(
            "get_edge_indices",
            [](const TranslatedGraphType& self)
            {
                auto range = self.get_edge_indices();
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_forward_adjacent_vertices",
            [](const TranslatedGraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertices",
            [](const TranslatedGraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const TranslatedGraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const TranslatedGraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const TranslatedGraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edges",
            [](const TranslatedGraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const TranslatedGraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const TranslatedGraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_vertices", &TranslatedGraphType::get_vertices)
        .def("get_vertex", &TranslatedGraphType::get_vertex, py::arg("vertex_index"))
        .def("get_edges", &TranslatedGraphType::get_vertices)
        .def("get_edge", &TranslatedGraphType::get_vertex)
        .def("get_num_vertices", &TranslatedGraphType::get_num_vertices)
        .def("get_num_edges", &TranslatedGraphType::get_num_edges)
        .def("get_forward_source", &TranslatedGraphType::template get_source<mm::ForwardTraversal>, py::arg("edge_index"))
        .def("get_backward_source", &TranslatedGraphType::template get_source<mm::BackwardTraversal>, py::arg("edge_index"))
        .def("get_forward_target", &TranslatedGraphType::template get_target<mm::ForwardTraversal>, py::arg("edge_index"))
        .def("get_backward_target", &TranslatedGraphType::template get_target<mm::BackwardTraversal>, py::arg("edge_index"))
        .def("get_forward_degrees", &TranslatedGraphType::template get_degrees<mm::ForwardTraversal>)
        .def("get_backward_degrees", &TranslatedGraphType::template get_degrees<mm::BackwardTraversal>)
        .def("get_forward_degree", &TranslatedGraphType::template get_degree<mm::ForwardTraversal>, py::arg("vertex_index"))
        .def("get_backward_degree", &TranslatedGraphType::template get_degree<mm::BackwardTraversal>, py::arg("vertex_index"));
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
        .def("clear", &GraphType::clear)
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
        .def(
            "get_vertex_indices",
            [](const GraphType& self)
            {
                auto range = self.get_vertex_indices();
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_edge_indices",
            [](const GraphType& self)
            {
                auto range = self.get_edge_indices();
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_forward_adjacent_vertices",
            [](const GraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertices",
            [](const GraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const GraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const GraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const GraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edges",
            [](const GraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const GraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const GraphType& self, mm::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_vertices", &GraphType::get_vertices)
        .def("get_vertex", &GraphType::get_vertex, py::arg("vertex_index"))
        .def("get_edges", &GraphType::get_vertices)
        .def("get_edge", &GraphType::get_vertex)
        .def("get_num_vertices", &GraphType::get_num_vertices)
        .def("get_num_edges", &GraphType::get_num_edges)
        .def("get_forward_source", &GraphType::template get_source<mm::ForwardTraversal>, py::arg("edge_index"))
        .def("get_backward_source", &GraphType::template get_source<mm::BackwardTraversal>, py::arg("edge_index"))
        .def("get_forward_target", &GraphType::template get_target<mm::ForwardTraversal>, py::arg("edge_index"))
        .def("get_backward_target", &GraphType::template get_target<mm::BackwardTraversal>, py::arg("edge_index"))
        .def("get_forward_degrees", &GraphType::template get_degrees<mm::ForwardTraversal>)
        .def("get_backward_degrees", &GraphType::template get_degrees<mm::BackwardTraversal>)
        .def("get_forward_degree", &GraphType::template get_degree<mm::ForwardTraversal>, py::arg("vertex_index"))
        .def("get_backward_degree", &GraphType::template get_degree<mm::BackwardTraversal>, py::arg("vertex_index"));

    /**
     * Immutable version
     */

    py::class_<PyImmutable<GraphType>>(m, ("Immutable" + name).c_str())  //
        .def(py::init<const GraphType&>())
        .def(
            "get_vertex_indices",
            [](const PyImmutable<GraphType>& self)
            {
                auto range = self.obj_.get_vertex_indices();
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_edge_indices",
            [](const PyImmutable<GraphType>& self)
            {
                auto range = self.obj_.get_edge_indices();
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_forward_adjacent_vertices",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertices<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertices",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertices<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertex_indices<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertex_indices<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edges<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edges",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edges<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edge_indices<mm::ForwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edge_indices<mm::BackwardTraversal>(vertex);
                return py::make_iterator(range.begin(), range.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_vertices", [](const PyImmutable<GraphType>& self) { return self.obj_.get_vertices(); })
        .def(
            "get_vertex",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex) { return self.obj_.get_vertex(vertex); },
            py::arg("vertex_index"))
        .def("get_edges", [](const PyImmutable<GraphType>& self) { return self.obj_.get_edges(); })
        .def("get_edge", [](const PyImmutable<GraphType>& self, mm::EdgeIndex edge) { return self.obj_.get_edge(edge); })
        .def("get_num_vertices", [](const PyImmutable<GraphType>& self) { return self.obj_.get_num_vertices(); })
        .def("get_num_edges", [](const PyImmutable<GraphType>& self) { return self.obj_.get_num_edges(); })
        .def(
            "get_forward_source",
            [](const PyImmutable<GraphType>& self, mm::EdgeIndex edge) { return self.obj_.template get_source<mm::ForwardTraversal>(edge); },
            py::arg("edge_index"))
        .def(
            "get_backward_source",
            [](const PyImmutable<GraphType>& self, mm::EdgeIndex edge) { return self.obj_.template get_source<mm::BackwardTraversal>(edge); },
            py::arg("edge_index"))
        .def(
            "get_forward_target",
            [](const PyImmutable<GraphType>& self, mm::EdgeIndex edge) { return self.obj_.template get_target<mm::ForwardTraversal>(edge); },
            py::arg("edge_index"))
        .def(
            "get_backward_target",
            [](const PyImmutable<GraphType>& self, mm::EdgeIndex edge) { return self.obj_.template get_target<mm::BackwardTraversal>(edge); },
            py::arg("edge_index"))
        .def("get_forward_degrees", [](const PyImmutable<GraphType>& self) { return self.obj_.template get_degrees<mm::ForwardTraversal>(); })
        .def("get_backward_degrees", [](const PyImmutable<GraphType>& self) { return self.obj_.template get_degrees<mm::BackwardTraversal>(); })
        .def(
            "get_forward_degree",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex) { return self.obj_.template get_degree<mm::ForwardTraversal>(vertex); },
            py::arg("vertex_index"))
        .def(
            "get_backward_degree",
            [](const PyImmutable<GraphType>& self, mm::VertexIndex vertex) { return self.obj_.template get_degree<mm::BackwardTraversal>(vertex); },
            py::arg("vertex_index"));

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