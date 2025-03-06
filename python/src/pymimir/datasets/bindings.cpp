#include "bindings.hpp"

#include "../init_declarations.hpp"

using namespace mimir;

void bind_datasets(nb::module_& m)
{
    bind_vertex<ProblemVertex>(m, PyVertexProperties<ProblemVertex>::name);
    bind_vertex<ClassVertex>(m, PyVertexProperties<ClassVertex>::name);
    bind_edge<ProblemEdge>(m, PyEdgeProperties<ProblemEdge>::name);
    bind_edge<ClassEdge>(m, PyEdgeProperties<ClassEdge>::name);
    bind_static_graph<ProblemVertex, ProblemEdge>(m, "ProblemStaticGraph");
    bind_static_graph<ClassVertex, ClassEdge>(m, "ClassStaticGraph");

    nb::class_<ClassStateSpace>(m, "ClassStateSpace")
        .def(nb::init<>())
        .def(nb::init<ClassGraph>(), nb::arg("graph"))
        .def("get_graph", &ClassStateSpace::get_graph, nb::rv_policy::reference_internal)
        .def("get_initial_vertices", &ClassStateSpace::get_initial_vertices, nb::rv_policy::copy)
        .def("get_goal_vertices", &ClassStateSpace::get_goal_vertices, nb::rv_policy::copy)
        .def("get_unsolvable_vertices", &ClassStateSpace::get_unsolvable_vertices, nb::rv_policy::copy)
        .def("get_alive_vertices", &ClassStateSpace::get_alive_vertices, nb::rv_policy::copy);

    nb::class_<GeneralizedStateSpace::Options::ProblemSpecific>(m, "GeneralizedStateSpaceProblemOptions")
        .def(nb::init<>())
        .def_rw("symmetry_pruning", &GeneralizedStateSpace::Options::ProblemSpecific::symmetry_pruning)
        .def_rw("mark_true_goal_literals", &GeneralizedStateSpace::Options::ProblemSpecific::mark_true_goal_literals)
        .def_rw("remove_if_unsolvable", &GeneralizedStateSpace::Options::ProblemSpecific::remove_if_unsolvable)
        .def_rw("max_num_states", &GeneralizedStateSpace::Options::ProblemSpecific::max_num_states)
        .def_rw("timeout_ms", &GeneralizedStateSpace::Options::ProblemSpecific::timeout_ms);

    nb::class_<GeneralizedStateSpace::Options>(m, "GeneralizedStateSpaceOptions")
        .def(nb::init<>())
        .def_rw("sort_ascending_by_num_states", &GeneralizedStateSpace::Options::sort_ascending_by_num_states)
        .def_rw("problem_options", &GeneralizedStateSpace::Options::problem_options);

    nb::class_<GeneralizedStateSpace>(m, "GeneralizedStateSpace")
        .def(nb::init<GeneralizedSearchContext, const GeneralizedStateSpace::Options&>(),
             nb::arg("context"),
             nb::arg("options") = GeneralizedStateSpace::Options())
        .def("get_generalized_search_context", &GeneralizedStateSpace::get_generalized_search_context, nb::rv_policy::copy)
        .def("get_problem_graphs", &GeneralizedStateSpace::get_problem_graphs, nb::rv_policy::copy)  // TODO: must return immutable list of graphs
        .def("get_class_state_space", &GeneralizedStateSpace::get_class_state_space, nb::rv_policy::reference_internal)
        .def("get_problem_graph",
             nb::overload_cast<const ClassVertex&>(&GeneralizedStateSpace::get_problem_graph, nb::const_),
             nb::rv_policy::reference_internal)
        .def("get_problem_graph", nb::overload_cast<const ClassEdge&>(&GeneralizedStateSpace::get_problem_graph, nb::const_), nb::rv_policy::reference_internal)
        .def("get_problem_vertex", &GeneralizedStateSpace::get_problem_vertex, nb::rv_policy::reference_internal)
        .def("get_problem_edge", &GeneralizedStateSpace::get_problem_edge, nb::rv_policy::reference_internal)
        .def("get_class_vertex", &GeneralizedStateSpace::get_class_vertex, nb::rv_policy::reference_internal)
        .def("get_class_edge", &GeneralizedStateSpace::get_class_edge, nb::rv_policy::reference_internal)
        .def("create_induced_subspace_from_class_vertex_indices",
             &GeneralizedStateSpace::create_induced_subspace_from_class_vertex_indices,
             nb::rv_policy::take_ownership)
        .def("create_induced_subspace_from_problem_indices",
             &GeneralizedStateSpace::create_induced_subspace_from_problem_indices,
             nb::rv_policy::take_ownership);

    nb::class_<KnowledgeBase>(m, "KnowledgeBase");
}