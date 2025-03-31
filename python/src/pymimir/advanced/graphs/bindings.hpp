
#ifndef MIMIR_PYTHON_GRAPHS_BINDINGS_HPP
#define MIMIR_PYTHON_GRAPHS_BINDINGS_HPP

#include "../init_declarations.hpp"

namespace mimir::bindings
{

template<mm::graphs::IsVertex V>
struct PyVertexProperties
{
};

template<>
struct PyVertexProperties<mm::graphs::EmptyVertex>
{
    static constexpr const char* name = "EmptyVertex";
};

template<>
struct PyVertexProperties<mm::graphs::ColoredVertex>
{
    static constexpr const char* name = "ColoredVertex";
};

template<mm::graphs::IsEdge E>
struct PyEdgeProperties
{
};

template<>
struct PyEdgeProperties<mm::graphs::EmptyEdge>
{
    static constexpr const char* name = "EmptyEdge";
};

template<>
struct PyEdgeProperties<mm::graphs::ColoredEdge>
{
    static constexpr const char* name = "ColoredEdge";
};

///////////////////////////////////////////////////////////////////////////////
/// Vertex
///////////////////////////////////////////////////////////////////////////////

template<mm::graphs::IsVertex V, std::size_t... Is>
void bind_vertex_get_properties(nb::class_<V>& cls, std::index_sequence<Is...>)
{
    (cls.def(("get_property_" + std::to_string(Is)).c_str(), [](const V& v) { return v.template get_property<Is>(); }, nb::rv_policy::reference_internal), ...);
}

template<mm::graphs::IsVertex V>
void bind_vertex(nb::module_& m, const std::string& name)
{
    auto cls = nb::class_<V>(m, name.c_str())  //
                   .def("get_index", &V::get_index);

    constexpr std::size_t N = std::tuple_size<typename V::VertexPropertiesTypes>::value;
    bind_vertex_get_properties<V>(cls, std::make_index_sequence<N> {});
}

///////////////////////////////////////////////////////////////////////////////
/// Edge
///////////////////////////////////////////////////////////////////////////////

template<mm::graphs::IsEdge E, std::size_t... Is>
void bind_edge_get_properties(nb::class_<E>& cls, std::index_sequence<Is...>)
{
    (cls.def(("get_property_" + std::to_string(Is)).c_str(), [](const E& v) { return v.template get_property<Is>(); }, nb::rv_policy::reference_internal), ...);
}

template<mm::graphs::IsEdge E>
void bind_edge(nb::module_& m, const std::string& name)
{
    auto cls = nb::class_<E>(m, name.c_str())  //
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
void bind_translated_static_graph(nb::module_& m, const std::string& name, const std::string& prefix)
{
    using PyImmutableGraph = PyImmutable<GraphType>;

    nb::class_<TranslatedGraphType>(m, (prefix + name).c_str())
        .def(nb::init<GraphType>())
        .def("__init__", [](TranslatedGraphType* self, const PyImmutableGraph& immutable) { new (self) TranslatedGraphType(immutable.obj_); })
        .def(
            "get_vertex_indices",
            [](const TranslatedGraphType& self)
            {
                auto range = self.get_vertex_indices();
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_edge_indices",
            [](const TranslatedGraphType& self)
            {
                auto range = self.get_edge_indices();
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_forward_adjacent_vertices",
            [](const TranslatedGraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertices",
            [](const TranslatedGraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const TranslatedGraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const TranslatedGraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward backward vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const TranslatedGraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edges",
            [](const TranslatedGraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const TranslatedGraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const TranslatedGraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def("get_vertices", &TranslatedGraphType::get_vertices)
        .def("get_vertex", &TranslatedGraphType::get_vertex, nb::arg("vertex_index"))
        .def("get_edges", &TranslatedGraphType::get_edges)
        .def("get_edge", &TranslatedGraphType::get_vertex)
        .def("get_num_vertices", &TranslatedGraphType::get_num_vertices)
        .def("get_num_edges", &TranslatedGraphType::get_num_edges)
        .def("get_forward_source", &TranslatedGraphType::template get_source<mm::graphs::ForwardTag>, nb::arg("edge_index"))
        .def("get_backward_source", &TranslatedGraphType::template get_source<mm::graphs::BackwardTag>, nb::arg("edge_index"))
        .def("get_forward_target", &TranslatedGraphType::template get_target<mm::graphs::ForwardTag>, nb::arg("edge_index"))
        .def("get_backward_target", &TranslatedGraphType::template get_target<mm::graphs::BackwardTag>, nb::arg("edge_index"))
        .def("get_forward_degrees", &TranslatedGraphType::template get_degrees<mm::graphs::ForwardTag>)
        .def("get_backward_degrees", &TranslatedGraphType::template get_degrees<mm::graphs::BackwardTag>)
        .def("get_forward_degree", &TranslatedGraphType::template get_degree<mm::graphs::ForwardTag>, nb::arg("vertex_index"))
        .def("get_backward_degree", &TranslatedGraphType::template get_degree<mm::graphs::BackwardTag>, nb::arg("vertex_index"))
        .def("compute_forward_topological_sort",
             [](const TranslatedGraphType& self)
             { return mm::graphs::bgl::topological_sort(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {})); })
        .def("compute_backward_topological_sort",
             [](const TranslatedGraphType& self)
             { return mm::graphs::bgl::topological_sort(mimir::graphs::DirectionTaggedType(self, mm::graphs::BackwardTag {})); })
        .def("compute_forward_floyd_warshall_all_pairs_shortest_paths",
             [](const TranslatedGraphType& self, const ContinuousCostList& w)
             { return mm::graphs::bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {}), w); })
        .def("compute_backward_floyd_warshall_all_pairs_shortest_paths",
             [](const TranslatedGraphType& self, const ContinuousCostList& w)
             { return mm::graphs::bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self, mm::graphs::BackwardTag {}), w); })
        .def("compute_forward_breadth_first_search",
             [](const TranslatedGraphType& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {}),
                                                              source_vertex_indices.begin(),
                                                              source_vertex_indices.end());
             })
        .def("compute_backward_breadth_first_search",
             [](const TranslatedGraphType& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self, mm::graphs::BackwardTag {}),
                                                              source_vertex_indices.begin(),
                                                              source_vertex_indices.end());
             })
        .def("compute_forward_depth_first_search",
             [](const TranslatedGraphType& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {}),
                                                            source_vertex_indices.begin(),
                                                            source_vertex_indices.end());
             })
        .def("compute_backward_depth_first_search",
             [](const TranslatedGraphType& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self, mm::graphs::BackwardTag {}),
                                                            source_vertex_indices.begin(),
                                                            source_vertex_indices.end());
             })
        .def("compute_forward_dijkstra_shortest_paths",
             [](const TranslatedGraphType& self, const ContinuousCostList& w, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {}),
                                                                 w,
                                                                 source_vertex_indices.begin(),
                                                                 source_vertex_indices.end());
             })
        .def("compute_backward_dijkstra_shortest_paths",
             [](const TranslatedGraphType& self, const ContinuousCostList& w, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self, mm::graphs::BackwardTag {}),
                                                                 w,
                                                                 source_vertex_indices.begin(),
                                                                 source_vertex_indices.end());
             })
        .def("compute_strong_components",
             [](const TranslatedGraphType& self, const ContinuousCostList& w, const mm::graphs::VertexIndexList& source_vertex_indices)
             { return mm::graphs::bgl::strong_components(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {})); });
}

template<mm::graphs::IsVertex V, mm::graphs::IsEdge E>
void bind_static_graph(nb::module_& m, const std::string& name)
{
    /**
     * Mutable version
     */

    using GraphType = mm::graphs::StaticGraph<V, E>;

    nb::class_<GraphType>(m, name.c_str())
        .def(nb::init<>())
        .def("clear", &GraphType::clear)
        .def("add_vertex",
             [](GraphType& self, nb::args args)
             {
                 using PropertiesTuple = typename V::VertexPropertiesTypes;
                 return std::apply([&](auto&&... unpacked_args) { return self.add_vertex(std::forward<decltype(unpacked_args)>(unpacked_args)...); },
                                   cast<PropertiesTuple>(args));
             })
        .def("add_vertex", [](GraphType& self, const V& vertex) { return self.add_vertex(vertex); })
        .def("add_directed_edge",
             [](GraphType& self, mm::graphs::VertexIndex source, mm::graphs::VertexIndex target, nb::args args)
             {
                 using PropertiesTuple = typename E::EdgePropertiesTypes;
                 return std::apply([&](auto&&... unpacked_args)
                                   { return self.add_directed_edge(source, target, std::forward<decltype(unpacked_args)>(unpacked_args)...); },
                                   cast<PropertiesTuple>(args));
             })
        .def("add_directed_edge",
             [](GraphType& self, mm::graphs::VertexIndex source, mm::graphs::VertexIndex target, const E& edge)
             { return self.add_directed_edge(source, target, edge); })
        .def(
            "get_vertex_indices",
            [](const GraphType& self)
            {
                auto range = self.get_vertex_indices();
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_edge_indices",
            [](const GraphType& self)
            {
                auto range = self.get_edge_indices();
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_forward_adjacent_vertices",
            [](const GraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertices",
            [](const GraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const GraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const GraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const GraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edges",
            [](const GraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const GraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const GraphType& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward backward edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def("get_vertices", &GraphType::get_vertices)
        .def("get_vertex", &GraphType::get_vertex, nb::arg("vertex_index"))
        .def("get_edges", &GraphType::get_edges)
        .def("get_edge", &GraphType::get_vertex)
        .def("get_num_vertices", &GraphType::get_num_vertices)
        .def("get_num_edges", &GraphType::get_num_edges)
        .def("get_forward_source", &GraphType::template get_source<mm::graphs::ForwardTag>, nb::arg("edge_index"))
        .def("get_backward_source", &GraphType::template get_source<mm::graphs::BackwardTag>, nb::arg("edge_index"))
        .def("get_forward_target", &GraphType::template get_target<mm::graphs::ForwardTag>, nb::arg("edge_index"))
        .def("get_backward_target", &GraphType::template get_target<mm::graphs::BackwardTag>, nb::arg("edge_index"))
        .def("get_forward_degrees", &GraphType::template get_degrees<mm::graphs::ForwardTag>)
        .def("get_backward_degrees", &GraphType::template get_degrees<mm::graphs::BackwardTag>)
        .def("get_forward_degree", &GraphType::template get_degree<mm::graphs::ForwardTag>, nb::arg("vertex_index"))
        .def("get_backward_degree", &GraphType::template get_degree<mm::graphs::BackwardTag>, nb::arg("vertex_index"))
        .def("compute_forward_topological_sort",
             [](const GraphType& self) { return mm::graphs::bgl::topological_sort(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {})); })
        .def("compute_backward_topological_sort",
             [](const GraphType& self) { return mm::graphs::bgl::topological_sort(mimir::graphs::DirectionTaggedType(self, mm::graphs::BackwardTag {})); })
        .def("compute_forward_floyd_warshall_all_pairs_shortest_paths",
             [](const GraphType& self, const ContinuousCostList& w)
             { return mm::graphs::bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {}), w); })
        .def("compute_backward_floyd_warshall_all_pairs_shortest_paths",
             [](const GraphType& self, const ContinuousCostList& w)
             { return mm::graphs::bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self, mm::graphs::BackwardTag {}), w); })
        .def("compute_forward_breadth_first_search",
             [](const GraphType& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {}),
                                                              source_vertex_indices.begin(),
                                                              source_vertex_indices.end());
             })
        .def("compute_backward_breadth_first_search",
             [](const GraphType& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self, mm::graphs::BackwardTag {}),
                                                              source_vertex_indices.begin(),
                                                              source_vertex_indices.end());
             })
        .def("compute_forward_depth_first_search",
             [](const GraphType& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {}),
                                                            source_vertex_indices.begin(),
                                                            source_vertex_indices.end());
             })
        .def("compute_backward_depth_first_search",
             [](const GraphType& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self, mm::graphs::BackwardTag {}),
                                                            source_vertex_indices.begin(),
                                                            source_vertex_indices.end());
             })
        .def("compute_forward_dijkstra_shortest_paths",
             [](const GraphType& self, const ContinuousCostList& w, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {}),
                                                                 w,
                                                                 source_vertex_indices.begin(),
                                                                 source_vertex_indices.end());
             })
        .def("compute_backward_dijkstra_shortest_paths",
             [](const GraphType& self, const ContinuousCostList& w, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self, mm::graphs::BackwardTag {}),
                                                                 w,
                                                                 source_vertex_indices.begin(),
                                                                 source_vertex_indices.end());
             })
        .def("compute_strong_components",
             [](const GraphType& self, const ContinuousCostList& w, const mm::graphs::VertexIndexList& source_vertex_indices)
             { return mm::graphs::bgl::strong_components(mimir::graphs::DirectionTaggedType(self, mm::graphs::ForwardTag {})); });

    /**
     * Immutable version
     */

    nb::class_<PyImmutable<GraphType>>(m, ("Immutable" + name).c_str())  //
        .def(nb::init<const GraphType&>())
        .def(
            "get_vertex_indices",
            [](const PyImmutable<GraphType>& self)
            {
                auto range = self.obj_.get_vertex_indices();
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_edge_indices",
            [](const PyImmutable<GraphType>& self)
            {
                auto range = self.obj_.get_edge_indices();
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_forward_adjacent_vertices",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertices<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertices",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertices<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertex_indices<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertex_indices<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edges<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edges",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edges<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edge_indices<mm::graphs::ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edge_indices<mm::graphs::BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            nb::arg("vertex_index"))
        .def("get_vertices", [](const PyImmutable<GraphType>& self) { return self.obj_.get_vertices(); })
        .def(
            "get_vertex",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex) { return self.obj_.get_vertex(vertex); },
            nb::arg("vertex_index"))
        .def("get_edges", [](const PyImmutable<GraphType>& self) { return self.obj_.get_edges(); })
        .def("get_edge", [](const PyImmutable<GraphType>& self, mm::graphs::EdgeIndex edge) { return self.obj_.get_edge(edge); })
        .def("get_num_vertices", [](const PyImmutable<GraphType>& self) { return self.obj_.get_num_vertices(); })
        .def("get_num_edges", [](const PyImmutable<GraphType>& self) { return self.obj_.get_num_edges(); })
        .def(
            "get_forward_source",
            [](const PyImmutable<GraphType>& self, mm::graphs::EdgeIndex edge) { return self.obj_.template get_source<mm::graphs::ForwardTag>(edge); },
            nb::arg("edge_index"))
        .def(
            "get_backward_source",
            [](const PyImmutable<GraphType>& self, mm::graphs::EdgeIndex edge) { return self.obj_.template get_source<mm::graphs::BackwardTag>(edge); },
            nb::arg("edge_index"))
        .def(
            "get_forward_target",
            [](const PyImmutable<GraphType>& self, mm::graphs::EdgeIndex edge) { return self.obj_.template get_target<mm::graphs::ForwardTag>(edge); },
            nb::arg("edge_index"))
        .def(
            "get_backward_target",
            [](const PyImmutable<GraphType>& self, mm::graphs::EdgeIndex edge) { return self.obj_.template get_target<mm::graphs::BackwardTag>(edge); },
            nb::arg("edge_index"))
        .def("get_forward_degrees", [](const PyImmutable<GraphType>& self) { return self.obj_.template get_degrees<mm::graphs::ForwardTag>(); })
        .def("get_backward_degrees", [](const PyImmutable<GraphType>& self) { return self.obj_.template get_degrees<mm::graphs::BackwardTag>(); })
        .def(
            "get_forward_degree",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex) { return self.obj_.template get_degree<mm::graphs::ForwardTag>(vertex); },
            nb::arg("vertex_index"))
        .def(
            "get_backward_degree",
            [](const PyImmutable<GraphType>& self, mm::graphs::VertexIndex vertex) { return self.obj_.template get_degree<mm::graphs::BackwardTag>(vertex); },
            nb::arg("vertex_index"))
        .def("compute_forward_topological_sort",
             [](const PyImmutable<GraphType>& self)
             { return mm::graphs::bgl::topological_sort(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::ForwardTag {})); })
        .def("compute_backward_topological_sort",
             [](const PyImmutable<GraphType>& self)
             { return mm::graphs::bgl::topological_sort(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::BackwardTag {})); })
        .def("compute_forward_floyd_warshall_all_pairs_shortest_paths",
             [](const PyImmutable<GraphType>& self, const ContinuousCostList& w)
             { return mm::graphs::bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::ForwardTag {}), w); })
        .def("compute_backward_floyd_warshall_all_pairs_shortest_paths",
             [](const PyImmutable<GraphType>& self, const ContinuousCostList& w)
             { return mm::graphs::bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::BackwardTag {}), w); })
        .def("compute_forward_breadth_first_search",
             [](const PyImmutable<GraphType>& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::ForwardTag {}),
                                                              source_vertex_indices.begin(),
                                                              source_vertex_indices.end());
             })
        .def("compute_backward_breadth_first_search",
             [](const PyImmutable<GraphType>& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::BackwardTag {}),
                                                              source_vertex_indices.begin(),
                                                              source_vertex_indices.end());
             })
        .def("compute_forward_depth_first_search",
             [](const PyImmutable<GraphType>& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::ForwardTag {}),
                                                            source_vertex_indices.begin(),
                                                            source_vertex_indices.end());
             })
        .def("compute_backward_depth_first_search",
             [](const PyImmutable<GraphType>& self, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::BackwardTag {}),
                                                            source_vertex_indices.begin(),
                                                            source_vertex_indices.end());
             })
        .def("compute_forward_dijkstra_shortest_paths",
             [](const PyImmutable<GraphType>& self, const ContinuousCostList& w, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::ForwardTag {}),
                                                                 w,
                                                                 source_vertex_indices.begin(),
                                                                 source_vertex_indices.end());
             })
        .def("compute_backward_dijkstra_shortest_paths",
             [](const PyImmutable<GraphType>& self, const ContinuousCostList& w, const mm::graphs::VertexIndexList& source_vertex_indices)
             {
                 return mm::graphs::bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::BackwardTag {}),
                                                                 w,
                                                                 source_vertex_indices.begin(),
                                                                 source_vertex_indices.end());
             })
        .def("compute_strong_components",
             [](const PyImmutable<GraphType>& self, const ContinuousCostList& w, const mm::graphs::VertexIndexList& source_vertex_indices)
             { return mm::graphs::bgl::strong_components(mimir::graphs::DirectionTaggedType(self.obj_, mm::graphs::ForwardTag {})); });

    /**
     * Immutable forward version
     */

    using ForwardGraphType = mm::graphs::StaticForwardGraph<GraphType>;

    bind_translated_static_graph<ForwardGraphType, GraphType>(m, name, "Forward");

    /**
     * Immutable bidirectional version
     */

    using BidirectionalGraphType = mm::graphs::StaticBidirectionalGraph<GraphType>;

    bind_translated_static_graph<BidirectionalGraphType, GraphType>(m, name, "Bidirectional");
}

}

#endif