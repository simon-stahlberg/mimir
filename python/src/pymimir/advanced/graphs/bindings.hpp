
#ifndef MIMIR_PYTHON_GRAPHS_BINDINGS_HPP
#define MIMIR_PYTHON_GRAPHS_BINDINGS_HPP

#include "../init_declarations.hpp"

namespace mimir::graphs
{

struct with_gil_t
{
    template<class F>
    decltype(auto) operator()(F&& f) const
    {
        nb::gil_scoped_acquire gil;
        return std::forward<F>(f)();
    }
};
inline constexpr with_gil_t with_gil {};

class PyProperty final : public IProperty
{
public:
    explicit PyProperty(nb::object obj) : obj_(std::move(obj)) {}

    bool operator==(const IProperty& o) const override
    {
        if (auto rhs = dynamic_cast<const PyProperty*>(&o))
        {
            return with_gil(
                [&]() -> bool
                {
                    nb::object r = obj_.attr("__eq__")(rhs->obj_);
                    if (r.ptr() == Py_NotImplemented)
                    {
                        throw std::runtime_error("__eq__ returned NotImplemented");
                    }
                    return nb::cast<bool>(r);
                });
        }
        return false;
    }

    bool operator<(const IProperty& o) const override
    {
        if (auto rhs = dynamic_cast<const PyProperty*>(&o))
        {
            return with_gil(
                [&]
                {
                    nb::object r = obj_.attr("__lt__")(rhs->obj_);
                    if (r.ptr() == Py_NotImplemented)
                    {
                        throw std::runtime_error("__lt__ returned NotImplemented");
                    }
                    return nb::cast<bool>(r);
                });
        }
        return typeid(*this).before(typeid(o));
    }

    std::string str() const override
    {
        return with_gil([&] { return std::string(nb::str(obj_).c_str()); });
    }

    std::size_t hash() const override
    {
        return with_gil([&] { return static_cast<std::size_t>(nb::hash(obj_)); });
    }

private:
    nb::object obj_;
};

inline std::ostream& operator<<(std::ostream& out, const PyProperty& property)
{
    std::stringstream ss;
    ss << property.str();
    return out;
}

static_assert(Property<PyProperty>);

template<IsVertex V>
struct PyVertexProperties
{
};

using PyVertex = Vertex<PropertyValue>;

template<>
struct PyVertexProperties<PyVertex>
{
    static constexpr const char* name = "PyVertex";
};

template<IsEdge E>
struct PyEdgeProperties
{
};

using PyEdge = Edge<PropertyValue>;

template<>
struct PyEdgeProperties<PyEdge>
{
    static constexpr const char* name = "PyEdge";
};

///////////////////////////////////////////////////////////////////////////////
/// Vertex
///////////////////////////////////////////////////////////////////////////////

template<IsVertex V>
void bind_vertex(nb::module_& m, const std::string& name)
{
    auto cls = nb::class_<V>(m, name.c_str())  //
                   .def("get_index", &V::get_index)
                   .def("get_properties", &V::get_properties, nb::rv_policy::copy);
}

///////////////////////////////////////////////////////////////////////////////
/// Edge
///////////////////////////////////////////////////////////////////////////////

template<IsEdge E>
void bind_edge(nb::module_& m, const std::string& name)
{
    auto cls = nb::class_<E>(m, name.c_str())  //
                   .def("get_index", &E::get_index)
                   .def("get_source", &E::get_source)
                   .def("get_target", &E::get_target)
                   .def("get_properties", &E::get_properties, nb::rv_policy::copy);
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
        .def("create_induced_subgraph", &TranslatedGraphType::create_induced_subgraph, "vertex_indices")
        .def("create_undirected_graph", &TranslatedGraphType::create_undirected_graph)
        .def("__str__", [](const TranslatedGraphType& self) { return to_string(self); })
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
            [](const TranslatedGraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_vertices",
            [](const TranslatedGraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const TranslatedGraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const TranslatedGraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward backward vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_edges",
            [](const TranslatedGraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_edges",
            [](const TranslatedGraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_edge_indices",
            [](const TranslatedGraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_edge_indices",
            [](const TranslatedGraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def("get_vertices", &TranslatedGraphType::get_vertices)
        .def("get_vertex", &TranslatedGraphType::get_vertex, "vertex_index"_a)
        .def("get_edges", &TranslatedGraphType::get_edges)
        .def("get_edge", &TranslatedGraphType::get_edge, "edge_index"_a)
        .def("get_num_vertices", &TranslatedGraphType::get_num_vertices)
        .def("get_num_edges", &TranslatedGraphType::get_num_edges)
        .def("get_forward_source", &TranslatedGraphType::template get_source<ForwardTag>, "edge_index"_a)
        .def("get_backward_source", &TranslatedGraphType::template get_source<BackwardTag>, "edge_index"_a)
        .def("get_forward_target", &TranslatedGraphType::template get_target<ForwardTag>, "edge_index"_a)
        .def("get_backward_target", &TranslatedGraphType::template get_target<BackwardTag>, "edge_index"_a)
        .def("get_forward_degrees", &TranslatedGraphType::template get_degrees<ForwardTag>)
        .def("get_backward_degrees", &TranslatedGraphType::template get_degrees<BackwardTag>)
        .def("get_forward_degree", &TranslatedGraphType::template get_degree<ForwardTag>, "vertex_index"_a)
        .def("get_backward_degree", &TranslatedGraphType::template get_degree<BackwardTag>, "vertex_index"_a)
        .def("compute_forward_topological_sort",
             [](const TranslatedGraphType& self) { return bgl::topological_sort(mimir::graphs::DirectionTaggedType(self, ForwardTag {})); })
        .def("compute_backward_topological_sort",
             [](const TranslatedGraphType& self) { return bgl::topological_sort(mimir::graphs::DirectionTaggedType(self, BackwardTag {})); })
        .def(
            "compute_forward_floyd_warshall_all_pairs_shortest_paths",
            [](const TranslatedGraphType& self, const ContinuousCostList& w)
            { return bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self, ForwardTag {}), w); },
            "edge_weights"_a)
        .def(
            "compute_backward_floyd_warshall_all_pairs_shortest_paths",
            [](const TranslatedGraphType& self, const ContinuousCostList& w)
            { return bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self, BackwardTag {}), w); },
            "edge_weights"_a)
        .def(
            "compute_forward_breadth_first_search",
            [](const TranslatedGraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self, ForwardTag {}),
                                                 source_vertex_indices.begin(),
                                                 source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_backward_breadth_first_search",
            [](const TranslatedGraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self, BackwardTag {}),
                                                 source_vertex_indices.begin(),
                                                 source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_forward_depth_first_search",
            [](const TranslatedGraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self, ForwardTag {}),
                                               source_vertex_indices.begin(),
                                               source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_backward_depth_first_search",
            [](const TranslatedGraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self, BackwardTag {}),
                                               source_vertex_indices.begin(),
                                               source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_forward_dijkstra_shortest_paths",
            [](const TranslatedGraphType& self, const ContinuousCostList& edge_weights, const VertexIndexList& source_vertex_indices)
            {
                return bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self, ForwardTag {}),
                                                    edge_weights,
                                                    source_vertex_indices.begin(),
                                                    source_vertex_indices.end());
            },
            "edge_weights"_a,
            "source_vertex_indices"_a)
        .def(
            "compute_backward_dijkstra_shortest_paths",
            [](const TranslatedGraphType& self, const ContinuousCostList& edge_weights, const VertexIndexList& source_vertex_indices)
            {
                return bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self, BackwardTag {}),
                                                    edge_weights,
                                                    source_vertex_indices.begin(),
                                                    source_vertex_indices.end());
            },
            "edge_weights"_a,
            "source_vertex_indices"_a)
        .def("compute_strong_components",
             [](const TranslatedGraphType& self) { return bgl::strong_components(mimir::graphs::DirectionTaggedType(self, ForwardTag {})); });

    if constexpr (std::is_same_v<typename GraphType::VertexType::PropertyType, PropertyValue>)
    {
        m.def("compute_color_refinement_certificate", &color_refinement::compute_certificate<TranslatedGraphType>, "vertex_colored_graph"_a);
        m.def("compute_2fwl_certificate",
              &kfwl::compute_certificate<2, TranslatedGraphType>,
              "vertex_colored_graph"_a,
              "isomorphism_type_compression_function"_a);
        m.def("compute_3fwl_certificate",
              &kfwl::compute_certificate<3, TranslatedGraphType>,
              "vertex_colored_graph"_a,
              "isomorphism_type_compression_function"_a);
        m.def("compute_4fwl_certificate",
              &kfwl::compute_certificate<4, TranslatedGraphType>,
              "vertex_colored_graph"_a,
              "isomorphism_type_compression_function"_a);
    }

    /* Graph properties. */
    m.def("is_undirected", [](const TranslatedGraphType& self) { return is_undirected(self); });
    m.def("is_multi", [](const TranslatedGraphType& self) { return is_multi(self); });
    m.def("is_loopless", [](const TranslatedGraphType& self) { return is_loopless(self); });
    m.def("is_acyclic", [](const TranslatedGraphType& self) { return is_acyclic(self); });
}

template<IsVertex V, IsEdge E>
void bind_static_graph(nb::module_& m, const std::string& name)
{
    /**
     * Mutable version
     */

    using GraphType = StaticGraph<V, E>;

    nb::class_<GraphType>(m, name.c_str())
        .def(nb::init<>())
        .def("create_induced_subgraph", &GraphType::create_induced_subgraph, "vertex_indices")
        .def("create_undirected_graph", &GraphType::create_undirected_graph)
        .def("__str__", [](const GraphType& self) { return to_string(self); })
        .def("clear", &GraphType::clear)
        .def("add_vertex", [](GraphType& self, typename V::PropertyType property) { return self.add_vertex(property); })
        .def("add_vertex", [](GraphType& self, const V& vertex) { return self.add_vertex(vertex); })
        .def("add_directed_edge",
             [](GraphType& self, VertexIndex source, VertexIndex target, typename E::PropertyType property)
             { return self.add_directed_edge(source, target, property); })
        .def("add_directed_edge",
             [](GraphType& self, VertexIndex source, VertexIndex target, const E& edge) { return self.add_directed_edge(source, target, edge); })
        .def("add_undirected_edge",
             [](GraphType& self, VertexIndex source, VertexIndex target, typename E::PropertyType property)
             { return self.add_undirected_edge(source, target, property); })
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
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_vertices",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_edges",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_edges",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_edge_indices",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_edge_indices",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward backward edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def("get_vertices", &GraphType::get_vertices)
        .def("get_vertex", &GraphType::get_vertex, "vertex_index"_a)
        .def("get_edges", &GraphType::get_edges)
        .def("get_edge", &GraphType::get_edge, "edge_index"_a)
        .def("get_num_vertices", &GraphType::get_num_vertices)
        .def("get_num_edges", &GraphType::get_num_edges)
        .def("get_forward_source", &GraphType::template get_source<ForwardTag>, "edge_index"_a)
        .def("get_backward_source", &GraphType::template get_source<BackwardTag>, "edge_index"_a)
        .def("get_forward_target", &GraphType::template get_target<ForwardTag>, "edge_index"_a)
        .def("get_backward_target", &GraphType::template get_target<BackwardTag>, "edge_index"_a)
        .def("get_forward_degrees", &GraphType::template get_degrees<ForwardTag>)
        .def("get_backward_degrees", &GraphType::template get_degrees<BackwardTag>)
        .def("get_forward_degree", &GraphType::template get_degree<ForwardTag>, "vertex_index"_a)
        .def("get_backward_degree", &GraphType::template get_degree<BackwardTag>, "vertex_index"_a)
        .def("compute_forward_topological_sort",
             [](const GraphType& self) { return bgl::topological_sort(mimir::graphs::DirectionTaggedType(self, ForwardTag {})); })
        .def("compute_backward_topological_sort",
             [](const GraphType& self) { return bgl::topological_sort(mimir::graphs::DirectionTaggedType(self, BackwardTag {})); })
        .def(
            "compute_forward_floyd_warshall_all_pairs_shortest_paths",
            [](const GraphType& self, const ContinuousCostList& w)
            { return bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self, ForwardTag {}), w); },
            "edge_weights"_a)
        .def(
            "compute_backward_floyd_warshall_all_pairs_shortest_paths",
            [](const GraphType& self, const ContinuousCostList& w)
            { return bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self, BackwardTag {}), w); },
            "edge_weights"_a)
        .def(
            "compute_forward_breadth_first_search",
            [](const GraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self, ForwardTag {}),
                                                 source_vertex_indices.begin(),
                                                 source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_backward_breadth_first_search",
            [](const GraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self, BackwardTag {}),
                                                 source_vertex_indices.begin(),
                                                 source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_forward_depth_first_search",
            [](const GraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self, ForwardTag {}),
                                               source_vertex_indices.begin(),
                                               source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_backward_depth_first_search",
            [](const GraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self, BackwardTag {}),
                                               source_vertex_indices.begin(),
                                               source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_forward_dijkstra_shortest_paths",
            [](const GraphType& self, const ContinuousCostList& edge_weights, const VertexIndexList& source_vertex_indices)
            {
                return bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self, ForwardTag {}),
                                                    edge_weights,
                                                    source_vertex_indices.begin(),
                                                    source_vertex_indices.end());
            },
            "edge_weights"_a,
            "source_vertex_indices"_a)
        .def(
            "compute_backward_dijkstra_shortest_paths",
            [](const GraphType& self, const ContinuousCostList& edge_weights, const VertexIndexList& source_vertex_indices)
            {
                return bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self, BackwardTag {}),
                                                    edge_weights,
                                                    source_vertex_indices.begin(),
                                                    source_vertex_indices.end());
            },
            "edge_weights"_a,
            "source_vertex_indices"_a)
        .def("compute_strong_components",
             [](const GraphType& self) { return bgl::strong_components(mimir::graphs::DirectionTaggedType(self, ForwardTag {})); });

    if constexpr (std::is_same_v<typename GraphType::VertexType::PropertyType, PropertyValue>)
    {
        m.def("compute_color_refinement_certificate", &color_refinement::compute_certificate<GraphType>, "vertex_colored_graph"_a);
        m.def("compute_2fwl_certificate", &kfwl::compute_certificate<2, GraphType>, "vertex_colored_graph"_a, "isomorphism_type_compression_function"_a);
        m.def("compute_3fwl_certificate", &kfwl::compute_certificate<3, GraphType>, "vertex_colored_graph"_a, "isomorphism_type_compression_function"_a);
        m.def("compute_4fwl_certificate", &kfwl::compute_certificate<4, GraphType>, "vertex_colored_graph"_a, "isomorphism_type_compression_function"_a);
    }

    /* Graph properties. */
    m.def("is_undirected", [](const GraphType& self) { return is_undirected(self); });
    m.def("is_multi", [](const GraphType& self) { return is_multi(self); });
    m.def("is_loopless", [](const GraphType& self) { return is_loopless(self); });
    m.def("is_acyclic", [](const GraphType& self) { return is_acyclic(self); });

    /**
     * Immutable version
     */

    nb::class_<PyImmutable<GraphType>>(m, ("Immutable" + name).c_str())  //
        .def(nb::init<const GraphType&>())
        .def(
            "create_induced_subgraph",
            [](const PyImmutable<GraphType>& self, const graphs::VertexIndexList& vertex_indices) { return self.obj_.create_induced_subgraph(vertex_indices); },
            "vertex_indices")
        .def("create_undirected_graph", [](const PyImmutable<GraphType>& self) { return self.obj_.create_undirected_graph(); })
        .def("__str__", [](const PyImmutable<GraphType>& self) { return to_string(self.obj_); })
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
            [](const PyImmutable<GraphType>& self, VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_vertices",
            [](const PyImmutable<GraphType>& self, VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const PyImmutable<GraphType>& self, VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertex_indices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const PyImmutable<GraphType>& self, VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_vertex_indices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_edges",
            [](const PyImmutable<GraphType>& self, VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edges<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_edges",
            [](const PyImmutable<GraphType>& self, VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edges<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_edge_indices",
            [](const PyImmutable<GraphType>& self, VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edge_indices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_edge_indices",
            [](const PyImmutable<GraphType>& self, VertexIndex vertex)
            {
                auto range = self.obj_.template get_adjacent_edge_indices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def("get_vertices", [](const PyImmutable<GraphType>& self) { return self.obj_.get_vertices(); })
        .def(
            "get_vertex",
            [](const PyImmutable<GraphType>& self, VertexIndex vertex) { return self.obj_.get_vertex(vertex); },
            "vertex_index"_a)
        .def("get_edges", [](const PyImmutable<GraphType>& self) { return self.obj_.get_edges(); })
        .def(
            "get_edge",
            [](const PyImmutable<GraphType>& self, EdgeIndex edge) { return self.obj_.get_edge(edge); },
            "edge_index"_a)
        .def("get_num_vertices", [](const PyImmutable<GraphType>& self) { return self.obj_.get_num_vertices(); })
        .def("get_num_edges", [](const PyImmutable<GraphType>& self) { return self.obj_.get_num_edges(); })
        .def(
            "get_forward_source",
            [](const PyImmutable<GraphType>& self, EdgeIndex edge) { return self.obj_.template get_source<ForwardTag>(edge); },
            "edge_index"_a)
        .def(
            "get_backward_source",
            [](const PyImmutable<GraphType>& self, EdgeIndex edge) { return self.obj_.template get_source<BackwardTag>(edge); },
            "edge_index"_a)
        .def(
            "get_forward_target",
            [](const PyImmutable<GraphType>& self, EdgeIndex edge) { return self.obj_.template get_target<ForwardTag>(edge); },
            "edge_index"_a)
        .def(
            "get_backward_target",
            [](const PyImmutable<GraphType>& self, EdgeIndex edge) { return self.obj_.template get_target<BackwardTag>(edge); },
            "edge_index"_a)
        .def("get_forward_degrees", [](const PyImmutable<GraphType>& self) { return self.obj_.template get_degrees<ForwardTag>(); })
        .def("get_backward_degrees", [](const PyImmutable<GraphType>& self) { return self.obj_.template get_degrees<BackwardTag>(); })
        .def(
            "get_forward_degree",
            [](const PyImmutable<GraphType>& self, VertexIndex vertex) { return self.obj_.template get_degree<ForwardTag>(vertex); },
            "vertex_index"_a)
        .def(
            "get_backward_degree",
            [](const PyImmutable<GraphType>& self, VertexIndex vertex) { return self.obj_.template get_degree<BackwardTag>(vertex); },
            "vertex_index"_a)
        .def("compute_forward_topological_sort",
             [](const PyImmutable<GraphType>& self) { return bgl::topological_sort(mimir::graphs::DirectionTaggedType(self.obj_, ForwardTag {})); })
        .def("compute_backward_topological_sort",
             [](const PyImmutable<GraphType>& self) { return bgl::topological_sort(mimir::graphs::DirectionTaggedType(self.obj_, BackwardTag {})); })
        .def(
            "compute_forward_floyd_warshall_all_pairs_shortest_paths",
            [](const PyImmutable<GraphType>& self, const ContinuousCostList& w)
            { return bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self.obj_, ForwardTag {}), w); },
            "edge_weights"_a)
        .def(
            "compute_backward_floyd_warshall_all_pairs_shortest_paths",
            [](const PyImmutable<GraphType>& self, const ContinuousCostList& w)
            { return bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self.obj_, BackwardTag {}), w); },
            "edge_weights"_a)
        .def(
            "compute_forward_breadth_first_search",
            [](const PyImmutable<GraphType>& self, const VertexIndexList& source_vertex_indices) {
                return bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self.obj_, ForwardTag {}),
                                                 source_vertex_indices.begin(),
                                                 source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_backward_breadth_first_search",
            [](const PyImmutable<GraphType>& self, const VertexIndexList& source_vertex_indices)
            {
                return bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self.obj_, BackwardTag {}),
                                                 source_vertex_indices.begin(),
                                                 source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_forward_depth_first_search",
            [](const PyImmutable<GraphType>& self, const VertexIndexList& source_vertex_indices) {
                return bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self.obj_, ForwardTag {}),
                                               source_vertex_indices.begin(),
                                               source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_backward_depth_first_search",
            [](const PyImmutable<GraphType>& self, const VertexIndexList& source_vertex_indices) {
                return bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self.obj_, BackwardTag {}),
                                               source_vertex_indices.begin(),
                                               source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_forward_dijkstra_shortest_paths",
            [](const PyImmutable<GraphType>& self, const ContinuousCostList& edge_weights, const VertexIndexList& source_vertex_indices)
            {
                return bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self.obj_, ForwardTag {}),
                                                    edge_weights,
                                                    source_vertex_indices.begin(),
                                                    source_vertex_indices.end());
            },
            "edge_weights"_a,
            "source_vertex_indices"_a)
        .def(
            "compute_backward_dijkstra_shortest_paths",
            [](const PyImmutable<GraphType>& self, const ContinuousCostList& edge_weights, const VertexIndexList& source_vertex_indices)
            {
                return bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self.obj_, BackwardTag {}),
                                                    edge_weights,
                                                    source_vertex_indices.begin(),
                                                    source_vertex_indices.end());
            },
            "edge_weights"_a,
            "source_vertex_indices"_a)
        .def("compute_strong_components",
             [](const PyImmutable<GraphType>& self) { return bgl::strong_components(mimir::graphs::DirectionTaggedType(self.obj_, ForwardTag {})); });

    if constexpr (std::is_same_v<typename GraphType::VertexType::PropertyType, PropertyValue>)
    {
        m.def("compute_color_refinement_certificate",
              [](const PyImmutable<GraphType>& self) { return color_refinement::compute_certificate<GraphType>(self.obj_); });
        m.def(
            "compute_2fwl_certificate",
            [](const PyImmutable<GraphType>& self, kfwl::IsomorphismTypeCompressionFunction& f)
            { return kfwl::compute_certificate<2, GraphType>(self.obj_, f); },
            "vertex_colored_graph"_a,
            "isomorphism_type_compression_function"_a);
        m.def(
            "compute_3fwl_certificate",
            [](const PyImmutable<GraphType>& self, kfwl::IsomorphismTypeCompressionFunction& f)
            { return kfwl::compute_certificate<3, GraphType>(self.obj_, f); },
            "vertex_colored_graph"_a,
            "isomorphism_type_compression_function"_a);
        m.def(
            "compute_4fwl_certificate",
            [](const PyImmutable<GraphType>& self, kfwl::IsomorphismTypeCompressionFunction& f)
            { return kfwl::compute_certificate<4, GraphType>(self.obj_, f); },
            "vertex_colored_graph"_a,
            "isomorphism_type_compression_function"_a);
    }

    /* Graph properties. */
    m.def("is_undirected", [](const PyImmutable<GraphType>& self) { return is_undirected(self.obj_); });
    m.def("is_multi", [](const PyImmutable<GraphType>& self) { return is_multi(self.obj_); });
    m.def("is_loopless", [](const PyImmutable<GraphType>& self) { return is_loopless(self.obj_); });
    m.def("is_acyclic", [](const PyImmutable<GraphType>& self) { return is_acyclic(self.obj_); });

    /**
     * Immutable forward version
     */

    using ForwardGraphType = StaticForwardGraph<GraphType>;

    bind_translated_static_graph<ForwardGraphType, GraphType>(m, name, "Forward");

    /**
     * Immutable bidirectional version
     */

    using BidirectionalGraphType = StaticBidirectionalGraph<GraphType>;

    bind_translated_static_graph<BidirectionalGraphType, GraphType>(m, name, "Bidirectional");
}

///////////////////////////////////////////////////////////////////////////////
/// DynamicGraph
///////////////////////////////////////////////////////////////////////////////

template<IsVertex V, IsEdge E>
void bind_dynamic_graph(nb::module_& m, const std::string& name)
{
    /**
     * Mutable version
     */

    using GraphType = DynamicGraph<V, E>;

    nb::class_<GraphType>(m, name.c_str())
        .def(nb::init<>())
        .def("__str__", [](const GraphType& self) { return to_string(self); })
        .def("clear", &GraphType::clear)
        .def("add_vertex", [](GraphType& self, typename V::PropertyType property) { return self.add_vertex(property); })
        .def("add_vertex", [](GraphType& self, const V& vertex) { return self.add_vertex(vertex); })
        .def("add_directed_edge",
             [](GraphType& self, VertexIndex source, VertexIndex target, typename E::PropertyType property)
             { self.add_directed_edge(source, target, property); })
        .def("add_directed_edge",
             [](GraphType& self, VertexIndex source, VertexIndex target, const E& edge) { return self.add_directed_edge(source, target, edge); })
        .def("add_undirected_edge",
             [](GraphType& self, VertexIndex source, VertexIndex target, typename E::PropertyType property)
             { self.add_undirected_edge(source, target, property); })
        .def(
            "remove_vertex",
            [](GraphType& self, VertexIndex vertex) { self.remove_vertex(vertex); },
            "vertex_index"_a)
        .def(
            "remove_edge",
            [](GraphType& self, EdgeIndex edge) { self.remove_edge(edge); },
            "remove_edge"_a)
        .def("create_induced_subgraph", &GraphType::create_induced_subgraph, "vertex_indices")
        .def("create_undirected_graph", &GraphType::create_undirected_graph)
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
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_vertices",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_vertex_indices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent vertex indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_edges",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_edges",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edges<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over backward adjacent edges", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_forward_adjacent_edge_indices",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<ForwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward adjacent edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def(
            "get_backward_adjacent_edge_indices",
            [](const GraphType& self, VertexIndex vertex)
            {
                auto range = self.template get_adjacent_edge_indices<BackwardTag>(vertex);
                return nb::make_iterator(nb::type<GraphType>(), "Iterator over forward backward edge indices", range.begin(), range.end());
            },
            nb::keep_alive<0, 1>(),
            "vertex_index"_a)
        .def("get_vertices", &GraphType::get_vertices)
        .def("get_vertex", &GraphType::get_vertex, "vertex_index"_a)
        .def("get_edges", &GraphType::get_edges)
        .def("get_edge", &GraphType::get_edge, "edge_index"_a)
        .def("get_num_vertices", &GraphType::get_num_vertices)
        .def("get_num_edges", &GraphType::get_num_edges)
        .def("get_forward_source", &GraphType::template get_source<ForwardTag>, "edge_index"_a)
        .def("get_backward_source", &GraphType::template get_source<BackwardTag>, "edge_index"_a)
        .def("get_forward_target", &GraphType::template get_target<ForwardTag>, "edge_index"_a)
        .def("get_backward_target", &GraphType::template get_target<BackwardTag>, "edge_index"_a)
        .def("get_forward_degrees", &GraphType::template get_degrees<ForwardTag>)
        .def("get_backward_degrees", &GraphType::template get_degrees<BackwardTag>)
        .def("get_forward_degree", &GraphType::template get_degree<ForwardTag>, "vertex_index"_a)
        .def("get_backward_degree", &GraphType::template get_degree<BackwardTag>, "vertex_index"_a)
        .def("compute_forward_topological_sort",
             [](const GraphType& self) { return bgl::topological_sort(mimir::graphs::DirectionTaggedType(self, ForwardTag {})); })
        .def("compute_backward_topological_sort",
             [](const GraphType& self) { return bgl::topological_sort(mimir::graphs::DirectionTaggedType(self, BackwardTag {})); })
        .def(
            "compute_forward_floyd_warshall_all_pairs_shortest_paths",
            [](const GraphType& self, const ContinuousCostMap& w)
            { return bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self, ForwardTag {}), w); },
            "edge_weights"_a)
        .def(
            "compute_backward_floyd_warshall_all_pairs_shortest_paths",
            [](const GraphType& self, const ContinuousCostMap& w)
            { return bgl::floyd_warshall_all_pairs_shortest_paths(mimir::graphs::DirectionTaggedType(self, BackwardTag {}), w); },
            "edge_weights"_a)
        .def(
            "compute_forward_breadth_first_search",
            [](const GraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self, ForwardTag {}),
                                                 source_vertex_indices.begin(),
                                                 source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_backward_breadth_first_search",
            [](const GraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::breadth_first_search(mimir::graphs::DirectionTaggedType(self, BackwardTag {}),
                                                 source_vertex_indices.begin(),
                                                 source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_forward_depth_first_search",
            [](const GraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self, ForwardTag {}),
                                               source_vertex_indices.begin(),
                                               source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_backward_depth_first_search",
            [](const GraphType& self, const VertexIndexList& source_vertex_indices) {
                return bgl::depth_first_search(mimir::graphs::DirectionTaggedType(self, BackwardTag {}),
                                               source_vertex_indices.begin(),
                                               source_vertex_indices.end());
            },
            "source_vertex_indices"_a)
        .def(
            "compute_forward_dijkstra_shortest_paths",
            [](const GraphType& self, const ContinuousCostMap& edge_weights, const VertexIndexList& source_vertex_indices)
            {
                return bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self, ForwardTag {}),
                                                    edge_weights,
                                                    source_vertex_indices.begin(),
                                                    source_vertex_indices.end());
            },
            "edge_weights"_a,
            "source_vertex_indices"_a)
        .def(
            "compute_backward_dijkstra_shortest_paths",
            [](const GraphType& self, const ContinuousCostMap& edge_weights, const VertexIndexList& source_vertex_indices)
            {
                return bgl::dijkstra_shortest_paths(mimir::graphs::DirectionTaggedType(self, BackwardTag {}),
                                                    edge_weights,
                                                    source_vertex_indices.begin(),
                                                    source_vertex_indices.end());
            },
            "edge_weights"_a,
            "source_vertex_indices"_a)
        .def("compute_strong_components",
             [](const GraphType& self) { return bgl::strong_components(mimir::graphs::DirectionTaggedType(self, ForwardTag {})); });
    /* Graph properties. */
    m.def("is_undirected", [](const GraphType& self) { return is_undirected(self); });
    m.def("is_multi", [](const GraphType& self) { return is_multi(self); });
    m.def("is_loopless", [](const GraphType& self) { return is_loopless(self); });
    m.def("is_acyclic", [](const GraphType& self) { return is_acyclic(self); });
}

template<size_t k>
void bind_kfwl_certificate(nb::module_& m, const std::string& name)
{
    nb::class_<kfwl::CertificateImpl<k>>(m, name.c_str())  //
        .def("__str__", [](const kfwl::CertificateImpl<k>& self) { return to_string(self); })
        .def("__repr__", [](const kfwl::CertificateImpl<k>& self) { return to_string(self); })
        .def("__eq__", [](const kfwl::CertificateImpl<k>& lhs, const kfwl::CertificateImpl<k>& rhs) { return lhs == rhs; })
        .def("__ne__", [](const kfwl::CertificateImpl<k>& lhs, const kfwl::CertificateImpl<k>& rhs) { return lhs != rhs; })
        .def("__hash__", [](const kfwl::CertificateImpl<k>& self) { return loki::Hash<kfwl::CertificateImpl<k>>()(self); })
        .def("get_canonical_color_compression_function", &kfwl::CertificateImpl<k>::get_canonical_color_compression_function)
        .def("get_canonical_configuration_compression_function",
             [](const kfwl::CertificateImpl<k>& self)
             {
                 auto result = nb::dict {};
                 for (const auto& [configuration, hash] : self.get_canonical_configuration_compression_function())
                 {
                     auto list = nb::list {};
                     for (const auto& array : configuration.second)
                     {
                         auto inner = nb::list {};
                         for (const auto& element : array)
                         {
                             inner.append(element);
                         }
                         list.append(nb::tuple(inner));
                     }
                     auto key = nb::make_tuple(nb::int_(configuration.first), nb::tuple(list));
                     result[key] = hash;
                 }
                 return result;
             })
        .def("get_hash_to_color", &kfwl::CertificateImpl<k>::get_hash_to_color);
}
}

#endif