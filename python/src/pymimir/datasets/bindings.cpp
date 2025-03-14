#include "bindings.hpp"

#include "../init_declarations.hpp"

using namespace mimir;

void bind_datasets(nb::module_& m)
{
    bind_vertex<graphs::ProblemVertex>(m, PyVertexProperties<graphs::ProblemVertex>::name);
    bind_vertex<graphs::ClassVertex>(m, PyVertexProperties<graphs::ClassVertex>::name);
    bind_edge<graphs::ProblemEdge>(m, PyEdgeProperties<graphs::ProblemEdge>::name);
    bind_edge<graphs::ClassEdge>(m, PyEdgeProperties<graphs::ClassEdge>::name);
    bind_static_graph<graphs::ProblemVertex, graphs::ProblemEdge>(m, "ProblemStaticGraph");
    bind_static_graph<graphs::ClassVertex, graphs::ClassEdge>(m, "ClassStaticGraph");

    nb::class_<datasets::GeneralizedStateSpace::Options::ProblemSpecific>(m, "GeneralizedStateSpaceProblemOptions")
        .def(nb::init<>())
        .def_rw("symmetry_pruning", &datasets::GeneralizedStateSpace::Options::ProblemSpecific::symmetry_pruning)
        .def_rw("mark_true_goal_literals", &datasets::GeneralizedStateSpace::Options::ProblemSpecific::mark_true_goal_literals)
        .def_rw("remove_if_unsolvable", &datasets::GeneralizedStateSpace::Options::ProblemSpecific::remove_if_unsolvable)
        .def_rw("max_num_states", &datasets::GeneralizedStateSpace::Options::ProblemSpecific::max_num_states)
        .def_rw("timeout_ms", &datasets::GeneralizedStateSpace::Options::ProblemSpecific::timeout_ms);

    nb::class_<datasets::GeneralizedStateSpace::Options>(m, "GeneralizedStateSpaceOptions")
        .def(nb::init<>())
        .def_rw("sort_ascending_by_num_states", &datasets::GeneralizedStateSpace::Options::sort_ascending_by_num_states)
        .def_rw("problem_options", &datasets::GeneralizedStateSpace::Options::problem_options);

    nb::class_<datasets::GeneralizedStateSpace>(m, "GeneralizedStateSpace")
        .def(nb::init<GeneralizedSearchContext, const datasets::GeneralizedStateSpace::Options&>(),
             nb::arg("context"),
             nb::arg("options") = datasets::GeneralizedStateSpace::Options())
        .def("get_generalized_search_context", &datasets::GeneralizedStateSpace::get_generalized_search_context, nb::rv_policy::copy)
        .def("get_problem_graphs", &datasets::GeneralizedStateSpace::get_problem_graphs, nb::rv_policy::copy)  // TODO: must return immutable list of graphs
        .def("get_graph", &datasets::GeneralizedStateSpace::get_graph, nb::rv_policy::reference_internal)
        .def("get_initial_vertices", &datasets::GeneralizedStateSpace::get_initial_vertices, nb::rv_policy::copy)
        .def("get_goal_vertices", &datasets::GeneralizedStateSpace::get_goal_vertices, nb::rv_policy::copy)
        .def("get_unsolvable_vertices", &datasets::GeneralizedStateSpace::get_unsolvable_vertices, nb::rv_policy::copy)
        .def("get_problem_graph",
             nb::overload_cast<const graphs::ClassVertex&>(&datasets::GeneralizedStateSpace::get_problem_graph, nb::const_),
             nb::rv_policy::reference_internal)
        .def("get_problem_graph",
             nb::overload_cast<const graphs::ClassEdge&>(&datasets::GeneralizedStateSpace::get_problem_graph, nb::const_),
             nb::rv_policy::reference_internal)
        .def("get_problem_vertex", &datasets::GeneralizedStateSpace::get_problem_vertex, nb::rv_policy::reference_internal)
        .def("get_problem_edge", &datasets::GeneralizedStateSpace::get_problem_edge, nb::rv_policy::reference_internal)
        .def("get_problem",
             static_cast<const Problem& (datasets::GeneralizedStateSpace::*) (const graphs::ClassVertex&) const>(&datasets::GeneralizedStateSpace::get_problem),
             nb::arg("vertex"),
             nb::rv_policy::reference_internal)
        .def("get_problem",
             static_cast<const Problem& (datasets::GeneralizedStateSpace::*) (const graphs::ClassEdge&) const>(&datasets::GeneralizedStateSpace::get_problem),
             nb::arg("edge"),
             nb::rv_policy::reference_internal)
        .def("get_class_vertex", &datasets::GeneralizedStateSpace::get_class_vertex, nb::rv_policy::reference_internal)
        .def("get_class_edge", &datasets::GeneralizedStateSpace::get_class_edge, nb::rv_policy::reference_internal)
        .def("create_induced_subgraph_from_class_vertex_indices",
             &datasets::GeneralizedStateSpace::create_induced_subgraph_from_class_vertex_indices,
             nb::rv_policy::take_ownership)
        .def("create_induced_subgraph_from_problem_indices",
             &datasets::GeneralizedStateSpace::create_induced_subgraph_from_problem_indices,
             nb::rv_policy::take_ownership);

    nb::class_<datasets::TupleGraphCollection::Options>(m, "TupleGraphCollectionOptions")
        .def(nb::init<>())
        .def(nb::init<size_t, bool>(), nb::arg("width"), nb::arg("enable_dominance_pruning"))
        .def_rw("width", &datasets::TupleGraphCollection::Options::width)
        .def_rw("enable_dominance_pruning", &datasets::TupleGraphCollection::Options::enable_dominance_pruning);

    nb::class_<datasets::KnowledgeBase::Options>(m, "KnowledgeBaseOptions")
        .def(nb::init<>())
        .def(nb::init<const datasets::GeneralizedStateSpace::Options&, const std::optional<datasets::TupleGraphCollection::Options>&>(),
             nb::arg("state_space_options"),
             nb::arg("tuple_graph_options") = std::nullopt)
        .def_rw("state_space_options", &datasets::KnowledgeBase::Options::state_space_options)
        .def_rw("tuple_graph_options", &datasets::KnowledgeBase::Options::tuple_graph_options);

    nb::class_<datasets::KnowledgeBase>(m, "KnowledgeBase")
        .def(nb::init<GeneralizedSearchContext, const datasets::KnowledgeBase::Options&>(),
             nb::arg("contexts"),
             nb::arg("options") = datasets::KnowledgeBase::Options())
        .def_static("create", &datasets::KnowledgeBase::create, nb::arg("contexts"), nb::arg("options") = datasets::KnowledgeBase::Options())
        .def("get_generalized_state_space", &datasets::KnowledgeBase::get_generalized_state_space, nb::rv_policy::reference_internal);
}