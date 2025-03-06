
#include "bindings.hpp"

using namespace mimir;

namespace py = pybind11;

void init_datasets(py::module_& m)
{
    bind_vertex<ProblemVertex>(m, PyVertexProperties<ProblemVertex>::name);
    bind_vertex<ClassVertex>(m, PyVertexProperties<ClassVertex>::name);
    bind_edge<ProblemEdge>(m, PyEdgeProperties<ProblemEdge>::name);
    bind_edge<ClassEdge>(m, PyEdgeProperties<ClassEdge>::name);
    bind_static_graph<ProblemVertex, ProblemEdge>(m, "ProblemStaticGraph");
    bind_static_graph<ClassVertex, ClassEdge>(m, "ClassStaticGraph");

    py::class_<ClassStateSpace>(m, "ClassStateSpace")
        .def(py::init<>())
        .def(py::init<ClassGraph>(), py::arg("graph"))
        .def("get_graph", &ClassStateSpace::get_graph, py::return_value_policy::reference_internal)
        .def("get_initial_vertices", &ClassStateSpace::get_initial_vertices, py::return_value_policy::copy)
        .def("get_goal_vertices", &ClassStateSpace::get_goal_vertices, py::return_value_policy::copy)
        .def("get_unsolvable_vertices", &ClassStateSpace::get_unsolvable_vertices, py::return_value_policy::copy)
        .def("get_alive_vertices", &ClassStateSpace::get_alive_vertices, py::return_value_policy::copy);

    py::class_<GeneralizedStateSpace::Options::ProblemSpecific>(m, "GeneralizedStateSpaceProblemOptions")
        .def(py::init<>())
        .def_readwrite("symmetry_pruning", &GeneralizedStateSpace::Options::ProblemSpecific::symmetry_pruning)
        .def_readwrite("mark_true_goal_literals", &GeneralizedStateSpace::Options::ProblemSpecific::mark_true_goal_literals)
        .def_readwrite("remove_if_unsolvable", &GeneralizedStateSpace::Options::ProblemSpecific::remove_if_unsolvable)
        .def_readwrite("max_num_states", &GeneralizedStateSpace::Options::ProblemSpecific::max_num_states)
        .def_readwrite("timeout_ms", &GeneralizedStateSpace::Options::ProblemSpecific::timeout_ms);

    py::class_<GeneralizedStateSpace::Options>(m, "GeneralizedStateSpaceOptions")
        .def(py::init<>())
        .def_readwrite("sort_ascending_by_num_states", &GeneralizedStateSpace::Options::sort_ascending_by_num_states)
        .def_readwrite("problem_options", &GeneralizedStateSpace::Options::problem_options);

    py::class_<GeneralizedStateSpace>(m, "GeneralizedStateSpace")
        .def(py::init<GeneralizedSearchContext, const GeneralizedStateSpace::Options&>(),
             py::arg("context"),
             py::arg("options") = GeneralizedStateSpace::Options())
        .def("get_generalized_search_context", &GeneralizedStateSpace::get_generalized_search_context, py::return_value_policy::copy)
        .def("get_problem_graphs", &GeneralizedStateSpace::get_problem_graphs, py::return_value_policy::copy)  // TODO: must return immutable list of graphs
        .def("get_class_state_space", &GeneralizedStateSpace::get_class_state_space, py::return_value_policy::reference_internal)
        .def("get_problem_graph",
             py::overload_cast<const ClassVertex&>(&GeneralizedStateSpace::get_problem_graph, py::const_),
             py::return_value_policy::reference_internal)
        .def("get_problem_graph",
             py::overload_cast<const ClassEdge&>(&GeneralizedStateSpace::get_problem_graph, py::const_),
             py::return_value_policy::reference_internal)
        .def("get_problem_vertex", &GeneralizedStateSpace::get_problem_vertex, py::return_value_policy::reference_internal)
        .def("get_problem_edge", &GeneralizedStateSpace::get_problem_edge, py::return_value_policy::reference_internal)
        .def("get_class_vertex", &GeneralizedStateSpace::get_class_vertex, py::return_value_policy::reference_internal)
        .def("get_class_edge", &GeneralizedStateSpace::get_class_edge, py::return_value_policy::reference_internal)
        .def("create_induced_subspace_from_class_vertex_indices",
             &GeneralizedStateSpace::create_induced_subspace_from_class_vertex_indices,
             py::return_value_policy::take_ownership)
        .def("create_induced_subspace_from_problem_indices",
             &GeneralizedStateSpace::create_induced_subspace_from_problem_indices,
             py::return_value_policy::take_ownership);
    ;

    py::class_<KnowledgeBase>(m, "KnowledgeBase");
}
