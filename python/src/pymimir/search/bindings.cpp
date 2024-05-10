#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

#include "mimir/mimir.hpp"

using namespace mimir;

namespace py = pybind11;

void init_search(py::module_& m_search)
{
    py::enum_<SearchNodeStatus>(m_search, "SearchNodeStatus")
        .value("NEW", SearchNodeStatus::NEW)
        .value("OPEN", SearchNodeStatus::OPEN)
        .value("CLOSED", SearchNodeStatus::CLOSED)
        .value("DEAD_END", SearchNodeStatus::DEAD_END)
        .export_values();

    py::enum_<SearchStatus>(m_search, "SearchStatus")
        .value("IN_PROGRESS", SearchStatus::IN_PROGRESS)
        .value("OUT_OF_TIME", SearchStatus::OUT_OF_TIME)
        .value("OUT_OF_MEMORY", SearchStatus::OUT_OF_MEMORY)
        .value("FAILED", SearchStatus::FAILED)
        .value("EXHAUSTED", SearchStatus::EXHAUSTED)
        .value("SOLVED", SearchStatus::SOLVED)
        .export_values();

    /* State */
    py::class_<State>(m_search, "State")  //
        .def("__hash__", &State::hash)
        .def("__eq__", &State::operator==)
        .def(
            "__iter__",
            [](State& state) { return py::make_iterator(state.begin(), state.end()); },
            py::keep_alive<0, 1>())
        .def("to_string",
             [](State self, PDDLFactories& pddl_factories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(self, std::cref(pddl_factories));
                 return ss.str();
             })
        .def("get_id", &State::get_id);

    /* Action */
    py::class_<GroundAction>(m_search, "GroundAction")  //
        .def("__hash__", &GroundAction::hash)
        .def("__eq__", &GroundAction::operator==)
        .def("to_string",
             [](GroundAction self, PDDLFactories& pddl_factories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(self, std::cref(pddl_factories));
                 return ss.str();
             })
        .def("get_id", &GroundAction::get_id);

    /* AAGs */
    py::class_<IDynamicAAG, std::shared_ptr<IDynamicAAG>>(m_search, "IAAG")  //
        .def("generate_applicable_actions", &IDynamicAAG::generate_applicable_actions)
        .def("get_action", &IDynamicAAG::get_action);
    py::class_<LiftedAAG, IDynamicAAG, std::shared_ptr<LiftedAAG>>(m_search, "LiftedAAG")  //
        .def(py::init<Problem, PDDLFactories&>());
    py::class_<GroundedAAG, IDynamicAAG, std::shared_ptr<GroundedAAG>>(m_search, "GroundedAAG")  //
        .def(py::init<Problem, PDDLFactories&>());

    /* SSGs */
    py::class_<IDynamicSSG, std::shared_ptr<IDynamicSSG>>(m_search, "ISSG")  //
        .def("get_or_create_initial_state", &IDynamicSSG::get_or_create_initial_state)
        .def("get_or_create_successor_state", &IDynamicSSG::get_or_create_successor_state)
        .def("get_state_count", &IDynamicSSG::get_state_count);
    py::class_<SuccessorStateGenerator, IDynamicSSG, std::shared_ptr<SuccessorStateGenerator>>(m_search, "SSG")  //
        .def(py::init<Problem, std::shared_ptr<IDynamicAAG>>());

    /* Heuristics */
    py::class_<IDynamicHeuristic, std::shared_ptr<IDynamicHeuristic>>(m_search, "IHeuristic");
    py::class_<BlindHeuristic, IDynamicHeuristic, std::shared_ptr<BlindHeuristic>>(m_search, "BlindHeuristic").def(py::init<>());

    /* Event Handlers */
    py::class_<IEventHandler, std::shared_ptr<IEventHandler>>(m_search, "IEventHandler");
    py::class_<MinimalEventHandler, IEventHandler, std::shared_ptr<MinimalEventHandler>>(m_search, "MinimalEventHandler")  //
        .def(py::init<>());
    py::class_<DebugEventHandler, IEventHandler, std::shared_ptr<DebugEventHandler>>(m_search, "DebugEventHandler")  //
        .def(py::init<>());

    /* Algorithms */
    py::class_<IAlgorithm, std::shared_ptr<IAlgorithm>>(m_search, "IAlgorithm")  //
        .def("find_solution",
             [](IAlgorithm& algorithm)
             {
                 auto out_actions = GroundActionList {};
                 auto search_status = algorithm.find_solution(out_actions);
                 return std::make_tuple(search_status, out_actions);
             });

    py::class_<mimir::BrFsAlgorithm, mimir::IAlgorithm, std::shared_ptr<mimir::BrFsAlgorithm>>(m_search, "BrFsAlgorithm")  //
        .def(py::init<Problem, PDDLFactories&, std::shared_ptr<IDynamicSSG>, std::shared_ptr<IDynamicAAG>, std::shared_ptr<IEventHandler>>());
    py::class_<mimir::AStarAlgorithm, mimir::IAlgorithm, std::shared_ptr<mimir::AStarAlgorithm>>(m_search, "AStarAlgorithm")  //
        .def(py::init<Problem,
                      PDDLFactories&,
                      std::shared_ptr<IDynamicSSG>,
                      std::shared_ptr<IDynamicAAG>,
                      std::shared_ptr<IDynamicHeuristic>,
                      std::shared_ptr<IEventHandler>>());
}
