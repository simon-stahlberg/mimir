#include "bindings.hpp"

#include "../init_declarations.hpp"

using namespace mimir;
using namespace mimir::graphs;
using namespace mimir::formalism;

namespace mimir::datasets
{

void bind_module_definitions(nb::module_& m)
{
    /* ProblemGraph */
    bind_vertex<graphs::ProblemVertex>(m, PyVertexProperties<graphs::ProblemVertex>::name);
    m.def("get_state", [](const graphs::ProblemVertex& self) -> search::State { return graphs::get_state(self); }, "vertex"_a, nb::rv_policy::reference);
    m.def("get_problem", [](const graphs::ProblemVertex& self) -> const Problem& { return graphs::get_problem(self); }, "vertex"_a, nb::rv_policy::reference);
    m.def("get_unit_goal_distance", [](const graphs::ProblemVertex& self) -> DiscreteCost { return graphs::get_unit_goal_distance(self); }, "vertex"_a);
    m.def("get_action_goal_distance", [](const graphs::ProblemVertex& self) -> ContinuousCost { return graphs::get_action_goal_distance(self); }, "vertex"_a);
    m.def("is_initial", [](const graphs::ProblemVertex& self) -> bool { return graphs::is_initial(self); }, "vertex"_a);
    m.def("is_goal", [](const graphs::ProblemVertex& self) -> bool { return graphs::is_goal(self); }, "vertex"_a);
    m.def("is_unsolvable", [](const graphs::ProblemVertex& self) -> bool { return graphs::is_unsolvable(self); }, "vertex"_a);
    m.def("is_alive", [](const graphs::ProblemVertex& self) -> bool { return graphs::is_alive(self); }, "vertex"_a);
    bind_edge<graphs::ProblemEdge>(m, PyEdgeProperties<graphs::ProblemEdge>::name);
    m.def("get_action", [](const graphs::ProblemEdge& self) -> GroundAction { return graphs::get_action(self); }, "edge"_a, nb::rv_policy::reference);
    m.def("get_problem", [](const graphs::ProblemEdge& self) -> const Problem& { return graphs::get_problem(self); }, "edge"_a, nb::rv_policy::reference);
    m.def("get_action_cost", [](const graphs::ProblemEdge& self) -> ContinuousCost { return graphs::get_action_cost(self); }, "edge"_a);
    bind_static_graph<graphs::ProblemVertex, graphs::ProblemEdge>(m, "StaticProblemGraph");

    /* ClassGraph */
    bind_vertex<graphs::ClassVertex>(m, PyVertexProperties<graphs::ClassVertex>::name);
    m.def("get_problem_vertex_index", [](const graphs::ClassVertex& self) -> Index { return graphs::get_problem_vertex_index(self); }, "vertex"_a);
    m.def("get_problem_index", [](const graphs::ClassVertex& self) -> Index { return graphs::get_problem_index(self); }, "vertex"_a);
    bind_edge<graphs::ClassEdge>(m, PyEdgeProperties<graphs::ClassEdge>::name);
    m.def("get_problem_edge_index", [](const graphs::ClassEdge& self) -> Index { return graphs::get_problem_edge_index(self); }, "vertex"_a);
    m.def("get_problem_index", [](const graphs::ClassEdge& self) -> Index { return graphs::get_problem_index(self); }, "vertex"_a);
    bind_static_graph<graphs::ClassVertex, graphs::ClassEdge>(m, "StaticClassGraph");

    bind_vertex<graphs::TupleGraphVertex>(m, PyVertexProperties<graphs::TupleGraphVertex>::name);
    m.def(
        "get_atom_tuple",
        [](const graphs::TupleGraphVertex& self) -> search::iw::AtomIndexList { return graphs::get_atom_tuple(self); },
        "vertex"_a,
        nb::rv_policy::copy);
    m.def(
        "get_problem_vertices",
        [](const graphs::TupleGraphVertex& self) -> IndexList { return graphs::get_problem_vertices(self); },
        "vertex"_a,
        nb::rv_policy::copy);
    bind_static_graph<graphs::TupleGraphVertex, graphs::EmptyEdge>(m, "StaticTupleGraph");

    nb::class_<StateSpaceImpl::Options>(m, "StateSpaceOptions")
        .def(nb::init<>())
        .def_rw("sort_ascending_by_num_states", &StateSpaceImpl::Options::sort_ascending_by_num_states)
        .def_rw("symmetry_pruning", &StateSpaceImpl::Options::symmetry_pruning)
        .def_rw("remove_if_unsolvable", &StateSpaceImpl::Options::remove_if_unsolvable)
        .def_rw("max_num_states", &StateSpaceImpl::Options::max_num_states)
        .def_rw("timeout_ms", &StateSpaceImpl::Options::timeout_ms);

    nb::class_<TupleGraphImpl::Options>(m, "TupleGraphOptions")
        .def(nb::init<>())
        .def(nb::init<size_t, bool>(), "width"_a, "enable_dominance_pruning"_a)
        .def_rw("width", &TupleGraphImpl::Options::width)
        .def_rw("enable_dominance_pruning", &TupleGraphImpl::Options::enable_dominance_pruning);

    nb::class_<GeneralizedStateSpaceImpl::Options>(m, "GeneralizedStateSpaceOptions")  //
        .def(nb::init<>());

    nb::class_<KnowledgeBaseImpl::Options>(m, "KnowledgeBaseOptions")
        .def(nb::init<>())
        .def(nb::init<const state_space::Options&, const GeneralizedStateSpaceImpl::Options&, const std::optional<TupleGraphImpl::Options>&>(),
             "state_space_options"_a,
             "generalized_state_space_options"_a,
             "tuple_graph_options"_a = std::nullopt)
        .def_rw("state_space_options", &KnowledgeBaseImpl::Options::state_space_options)
        .def_rw("generalized_state_space_options", &KnowledgeBaseImpl::Options::generalized_state_space_options)
        .def_rw("tuple_graph_options", &KnowledgeBaseImpl::Options::tuple_graph_options);

    nb::class_<StateSpaceImpl>(m, "StateSpace")
        .def("__str__", [](const StateSpaceImpl& self) { return to_string(self.get_graph()); })
        .def_static(
            "create",
            [](search::SearchContext context, const StateSpaceImpl::Options& options) -> std::optional<std::pair<StateSpace, std::optional<CertificateMaps>>>
            { return StateSpaceImpl::create(context, options); },
            "context"_a,
            "options"_a)
        .def_static(
            "create",
            [](search::GeneralizedSearchContext contexts,
               const StateSpaceImpl::Options& options) -> std::vector<std::pair<StateSpace, std::optional<CertificateMaps>>>
            { return StateSpaceImpl::create(contexts, options); },
            "contexts"_a,
            "options"_a)
        .def("get_search_context", &StateSpaceImpl::get_search_context, nb::rv_policy::copy)
        .def("get_graph", &StateSpaceImpl::get_graph, nb::rv_policy::reference_internal)
        .def("get_initial_vertex", &StateSpaceImpl::get_initial_vertex, nb::rv_policy::copy)
        .def("get_goal_vertices", &StateSpaceImpl::get_goal_vertices, nb::rv_policy::copy)
        .def("get_unsolvable_vertices", &StateSpaceImpl::get_unsolvable_vertices, nb::rv_policy::copy);

    nb::class_<GeneralizedStateSpaceImpl>(m, "GeneralizedStateSpace")
        .def("__str__", [](const GeneralizedStateSpaceImpl& self) { return to_string(self.get_graph()); })
        .def_static("create", &GeneralizedStateSpaceImpl::create, "state_spaces"_a, "options"_a)
        .def("get_state_spaces", &GeneralizedStateSpaceImpl::get_state_spaces, nb::rv_policy::copy)
        .def("get_graph", &GeneralizedStateSpaceImpl::get_graph, nb::rv_policy::reference_internal)
        .def("get_initial_vertices", &GeneralizedStateSpaceImpl::get_initial_vertices, nb::rv_policy::copy)
        .def("get_goal_vertices", &GeneralizedStateSpaceImpl::get_goal_vertices, nb::rv_policy::copy)
        .def("get_unsolvable_vertices", &GeneralizedStateSpaceImpl::get_unsolvable_vertices, nb::rv_policy::copy)
        .def("get_state_space",
             nb::overload_cast<const graphs::ClassVertex&>(&GeneralizedStateSpaceImpl::get_state_space, nb::const_),
             "class_vertex"_a,
             nb::rv_policy::reference_internal)
        .def("get_state_space",
             nb::overload_cast<const graphs::ClassEdge&>(&GeneralizedStateSpaceImpl::get_state_space, nb::const_),
             "class_edge"_a,
             nb::rv_policy::reference_internal)
        .def("get_problem_vertex", &GeneralizedStateSpaceImpl::get_problem_vertex, "class_vertex"_a, nb::rv_policy::reference_internal)
        .def("get_problem_edge", &GeneralizedStateSpaceImpl::get_problem_edge, "class_edge"_a, nb::rv_policy::reference_internal)
        .def("get_class_vertex", &GeneralizedStateSpaceImpl::get_class_vertex, "problem_vertex"_a, nb::rv_policy::reference_internal)
        .def("get_class_edge", &GeneralizedStateSpaceImpl::get_class_edge, "problem_edge"_a, nb::rv_policy::reference_internal)
        .def("create_induced_subgraph_from_class_vertex_indices",
             &GeneralizedStateSpaceImpl::create_induced_subgraph_from_class_vertex_indices,
             "class_vertex_indices"_a)
        .def("create_induced_subgraph_from_problem_indices",
             &GeneralizedStateSpaceImpl::create_induced_subgraph_from_problem_indices,
             "problem_vertex_indices"_a);

    nb::class_<TupleGraphImpl>(m, "TupleGraph")
        .def("__str__", [](const TupleGraphImpl& self) { return to_string(self); })
        .def("get_state_space", &TupleGraphImpl::get_state_space, nb::rv_policy::copy)
        .def("get_graph", &TupleGraphImpl::get_graph, nb::rv_policy::reference_internal)
        .def("get_tuple_vertex_indices_grouped_by_distance",
             [](const TupleGraphImpl& self)
             {
                 auto result = std::vector<graphs::VertexIndexList> {};
                 for (const auto& group : self.get_tuple_vertex_indices_grouped_by_distance())
                 {
                     result.push_back(graphs::VertexIndexList(group.begin(), group.end()));
                 }
                 return result;
             })
        .def("get_problem_vertex_indices_grouped_by_distance",
             [](const TupleGraphImpl& self)
             {
                 auto result = std::vector<graphs::VertexIndexList> {};
                 for (const auto& group : self.get_problem_vertex_indices_grouped_by_distance())
                 {
                     result.push_back(graphs::VertexIndexList(group.begin(), group.end()));
                 }
                 return result;
             });

    nb::class_<KnowledgeBaseImpl>(m, "KnowledgeBase")
        .def_static("create", &KnowledgeBaseImpl::create, "contexts"_a, "options"_a)
        .def("get_domain", &KnowledgeBaseImpl::get_domain, nb::rv_policy::reference_internal)
        .def("get_state_spaces", &KnowledgeBaseImpl::get_state_spaces, nb::rv_policy::copy)
        .def("get_generalized_state_space", &KnowledgeBaseImpl::get_generalized_state_space, nb::rv_policy::reference_internal)
        .def("get_tuple_graphs", &KnowledgeBaseImpl::get_tuple_graphs, nb::rv_policy::copy);

    m.def("create_object_graph", create_object_graph, "state"_a, "problem"_a);
}
}
