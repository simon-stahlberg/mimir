#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

#include "mimir/mimir.hpp"

using namespace mimir;

namespace py = pybind11;

void init_search(py::module_& m_search)
{
    py::class_<IDynamicSSG>(m_search, "ISSG");

    /* SSGs */
    py::class_<IDynamicSSG>(m_search, "ISSG")  //
        .def("get_or_create_initial_state", &IDynamicSSG::get_or_create_initial_state)
        .def("get_or_create_successor_state", &IDynamicSSG::get_or_create_successor_state)
        .def("state_count", &IDynamicSSG::state_count);
    py::class_<SSG<SSGDispatcher<StateReprTag>>, IDynamicSSG>(m_search, "SSG");

    /* AAGs */
    py::class_<IDynamicAAG>(m_search, "IAAG")  //
        .def("generate_applicable_actions", &IDynamicAAG::generate_applicable_actions)
        .def("get_action", &IDynamicAAG::get_action);
    py::class_<AAG<LiftedAAGDispatcher<StateReprTag>>, IDynamicAAG>(m_search, "LiftedAAG");
    py::class_<AAG<GroundedAAGDispatcher<StateReprTag>>, IDynamicAAG>(m_search, "LiftedAAG");

    /* Event Handlers */
    py::class_<IEventHandler>(m_search, "IEventHandler");
    py::class_<MinimalEventHandler, IEventHandler>(m_search, "MinimalEventHandler")  //
        .def(py::init<>());
    py::class_<DebugEventHandler, IEventHandler>(m_search, "DebugEventHandler")  //
        .def(py::init<>());

    /* Algorithms */
    py::class_<IAlgorithm>(m_search, "IAlgorithm")  //
        .def("find_solution", &mimir::IAlgorithm::find_solution);

    py::class_<mimir::BrFsAlgorithm, mimir::IAlgorithm>(m_search, "BrFsAlgorithm")  //
        .def(py::init<Problem, PDDLFactories&, std::shared_ptr<IDynamicSSG>, std::shared_ptr<IDynamicAAG>, std::shared_ptr<IEventHandler>>());
}
