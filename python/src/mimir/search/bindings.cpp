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
    py::class_<ConstView<StateDispatcher<StateReprTag>>>(m_search, "State")  //
        .def("get_id", &ConstView<StateDispatcher<StateReprTag>>::get_id);

    /* Action */
    py::class_<ConstView<ActionDispatcher<StateReprTag>>>(m_search, "GroundAction")  //
        .def("get_id", &ConstView<ActionDispatcher<StateReprTag>>::get_id);

    /* SSGs */
    py::class_<IDynamicSSG, std::shared_ptr<IDynamicSSG>>(m_search, "ISSG")  //
        .def("get_or_create_initial_state", &IDynamicSSG::get_or_create_initial_state)
        .def("get_or_create_successor_state", &IDynamicSSG::get_or_create_successor_state)
        .def("get_state_count", &IDynamicSSG::get_state_count);
    py::class_<SSG<SSGDispatcher<StateReprTag>>, IDynamicSSG, std::shared_ptr<SSG<SSGDispatcher<StateReprTag>>>>(m_search, "SSG")  //
        .def(py::init<Problem>());

    /* AAGs */
    py::class_<IDynamicAAG, std::shared_ptr<IDynamicAAG>>(m_search, "IAAG")  //
        .def("generate_applicable_actions", &IDynamicAAG::generate_applicable_actions)
        .def("get_action", &IDynamicAAG::get_action);
    py::class_<AAG<LiftedAAGDispatcher<StateReprTag>>, IDynamicAAG, std::shared_ptr<AAG<LiftedAAGDispatcher<StateReprTag>>>>(m_search, "LiftedAAG")  //
        .def(py::init<Problem, PDDLFactories&>());
    py::class_<AAG<GroundedAAGDispatcher<StateReprTag>>, IDynamicAAG, std::shared_ptr<AAG<GroundedAAGDispatcher<StateReprTag>>>>(m_search, "GroundedAAG")  //
        .def(py::init<Problem, PDDLFactories&>());

    /* Heuristics */
    py::class_<IDynamicHeuristic, std::shared_ptr<IDynamicHeuristic>>(m_search, "IHeuristic");
    py::class_<Heuristic<HeuristicDispatcher<BlindTag, StateReprTag>>,
               IDynamicHeuristic,
               std::shared_ptr<Heuristic<HeuristicDispatcher<BlindTag, StateReprTag>>>>(m_search, "BlindHeuristic")
        .def(py::init<>());

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
                 auto out_actions = std::vector<ConstView<ActionDispatcher<StateReprTag>>>();
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
