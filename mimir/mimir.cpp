#include "formalism/declarations.hpp"
#include "generators/grounded_successor_generator.hpp"
#include "generators/lifted_successor_generator.hpp"
#include "generators/state_space.hpp"
#include "generators/successor_generator.hpp"
#include "pddl/parsers.hpp"

#include <Python.h>
#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace py::literals;

std::string to_string(const formalism::ActionImpl& action)
{
    std::string repr = action.schema->name + "(";

    for (std::size_t index = 0; index < action.arguments.size(); ++index)
    {
        repr += action.arguments[index]->name;

        if ((index + 1) < action.arguments.size())
        {
            repr += ", ";
        }
    }

    return repr + ")";
}

std::string to_string(const formalism::AtomImpl& state)
{
    std::string repr = state.predicate->name + "(";

    for (std::size_t index = 0; index < state.arguments.size(); ++index)
    {
        repr += state.arguments[index]->name;

        if ((index + 1) < state.arguments.size())
        {
            repr += ", ";
        }
    }

    return repr + ")";
}

std::shared_ptr<parsers::DomainParser> create_domain_parser(const std::string& path) { return std::make_shared<parsers::DomainParser>(path); }

std::shared_ptr<parsers::ProblemParser> create_problem_parser(const std::string& path) { return std::make_shared<parsers::ProblemParser>(path); }

std::shared_ptr<planners::LiftedSuccessorGenerator> create_lifted_successor_generator(const formalism::ProblemDescription& problem)
{
    return std::make_shared<planners::LiftedSuccessorGenerator>(problem->domain, problem);
}

std::shared_ptr<planners::GroundedSuccessorGenerator> create_grounded_successor_generator(const formalism::ProblemDescription& problem)
{
    const auto successor_generator = planners::LiftedSuccessorGenerator(problem->domain, problem);
    return std::make_shared<planners::GroundedSuccessorGenerator>(problem, successor_generator.get_actions());
}

PYBIND11_MODULE(mimir, m)
{
    m.doc() = "Mimir: Lifted PDDL parsing and expansion library.";

    py::class_<formalism::TypeImpl, formalism::Type>(m, "Type")
        .def_readonly("name", &formalism::TypeImpl::name)
        .def_readonly("base", &formalism::TypeImpl::base)
        .def("__repr__",
             [](const formalism::TypeImpl& type)
             { return type.base ? ("<Type '" + type.name + " : " + type.base->name + "'>") : ("<Type '" + type.name + "'>"); });

    py::class_<formalism::ObjectImpl, formalism::Object>(m, "Object")
        .def_readonly("id", &formalism::ObjectImpl::id)
        .def_readonly("name", &formalism::ObjectImpl::name)
        .def_readonly("type", &formalism::ObjectImpl::type)
        .def("is_free", &formalism::ObjectImpl::is_free_variable)
        .def("is_constant", &formalism::ObjectImpl::is_constant)
        .def("__repr__", [](const formalism::ObjectImpl& object) { return "<Object '" + object.name + "'>"; });

    py::class_<formalism::PredicateImpl, formalism::Predicate>(m, "Predicate")
        .def_readonly("id", &formalism::PredicateImpl::id)
        .def_readonly("name", &formalism::PredicateImpl::name)
        .def_readonly("arity", &formalism::PredicateImpl::arity)
        .def_readonly("parameters", &formalism::PredicateImpl::parameters)
        .def("__repr__",
             [](const formalism::PredicateImpl& predicate) { return "<Predicate '" + predicate.name + "/" + std::to_string(predicate.arity) + "'>"; });

    py::class_<formalism::AtomImpl, formalism::Atom>(m, "Atom")
        .def_readonly("predicate", &formalism::AtomImpl::predicate)
        .def_readonly("arguments", &formalism::AtomImpl::arguments)
        .def("__repr__", [](const formalism::AtomImpl& atom) { return "<Atom '" + to_string(atom) + "'>"; });

    py::class_<formalism::LiteralImpl, formalism::Literal>(m, "Literal")
        .def_readonly("atom", &formalism::LiteralImpl::atom)
        .def_readonly("negated", &formalism::LiteralImpl::negated)
        .def("__repr__",
             [](const formalism::LiteralImpl& literal)
             {
                 const auto prefix = (literal.negated ? std::string("not ") : std::string(""));
                 const auto name = to_string(*literal.atom);
                 return "<Literal '" + prefix + name + "'>";
             });

    py::class_<formalism::ActionSchemaImpl, formalism::ActionSchema>(m, "ActionSchema")
        .def_readonly("name", &formalism::ActionSchemaImpl::name)
        .def_readonly("arity", &formalism::ActionSchemaImpl::arity)
        .def_readonly("parameters", &formalism::ActionSchemaImpl::parameters)
        .def_readonly("precondition", &formalism::ActionSchemaImpl::precondition)
        .def_readonly("effect", &formalism::ActionSchemaImpl::effect)
        .def("__repr__",
             [](const formalism::ActionSchemaImpl& action_schema)
             { return "<ActionSchema '" + action_schema.name + "/" + std::to_string(action_schema.arity) + "'>"; });

    py::class_<formalism::DomainImpl, formalism::DomainDescription>(m, "Domain")
        .def_readonly("name", &formalism::DomainImpl::name)
        .def_readonly("requirements", &formalism::DomainImpl::requirements)
        .def_readonly("types", &formalism::DomainImpl::types)
        .def_readonly("constants", &formalism::DomainImpl::constants)
        .def_readonly("predicates", &formalism::DomainImpl::predicates)
        .def_readonly("static_predicates", &formalism::DomainImpl::static_predicates)
        .def_readonly("action_schemas", &formalism::DomainImpl::action_schemas)
        .def("get_type_map", &formalism::DomainImpl::get_type_map)
        .def("get_predicate_name_map", &formalism::DomainImpl::get_predicate_name_map)
        .def("get_predicate_id_map", &formalism::DomainImpl::get_predicate_id_map)
        .def("get_constant_map", &formalism::DomainImpl::get_constant_map)
        .def("__repr__", [](const formalism::DomainImpl& domain) { return "<Domain '" + domain.name + "'>"; });

    py::class_<formalism::StateImpl, formalism::State>(m, "State")
        .def("get_atoms", &formalism::StateImpl::get_atoms)
        .def("get_static_atoms", &formalism::StateImpl::get_static_atoms)
        .def("get_dynamic_atoms", &formalism::StateImpl::get_dynamic_atoms)
        // .def("get_problem", &formalism::StateImpl::get_problem)  // Forward declare Problem.
        .def("get_atoms_by_predicate", &formalism::StateImpl::get_atoms_grouped_by_predicate)
        .def("get_atom_argument_ids_grouped_by_predicate_ids", &formalism::StateImpl::get_atom_argument_ids_grouped_by_predicate_ids)
        .def("__repr__", [](const formalism::StateImpl& state) { return "<State '" + std::to_string(state.hash()) + "'>"; });

    py::class_<formalism::ActionImpl, formalism::Action>(m, "Action")
        // .def_readonly("problem", &formalism::ActionImpl::problem)  // Forward declare Problem.
        .def_readonly("schema", &formalism::ActionImpl::schema)
        .def_readonly("arguments", &formalism::ActionImpl::arguments)
        .def_readonly("cost", &formalism::ActionImpl::cost)
        .def("get_precondition", &formalism::ActionImpl::get_precondition)
        .def("get_effect", &formalism::ActionImpl::get_effect)
        .def("__repr__", [](const formalism::ActionImpl& action) { return "<Action '" + to_string(action) + "'>"; });

    py::class_<formalism::ProblemImpl, formalism::ProblemDescription>(m, "Problem")
        .def_readonly("name", &formalism::ProblemImpl::name)
        .def_readonly("domain", &formalism::ProblemImpl::domain)
        .def_readonly("objects", &formalism::ProblemImpl::objects)
        .def_readonly("initial", &formalism::ProblemImpl::initial)
        .def_readonly("goal", &formalism::ProblemImpl::goal)
        .def("__repr__", [](const formalism::ProblemImpl& problem) { return "<Problem '" + problem.name + "'>"; });

    py::class_<parsers::DomainParser, std::shared_ptr<parsers::DomainParser>>(m, "DomainParser")
        .def(py::init(&create_domain_parser))
        .def("parse", &parsers::DomainParser::parse);

    py::class_<parsers::ProblemParser, std::shared_ptr<parsers::ProblemParser>>(m, "ProblemParser")
        .def(py::init(&create_problem_parser))
        .def("parse", &parsers::ProblemParser::parse);

    py::class_<planners::SuccessorGeneratorBase, planners::SuccessorGenerator> successor_generator_base(m, "SuccessorGenerator");
    successor_generator_base.def("get_applicable_actions", &planners::SuccessorGeneratorBase::get_applicable_actions);
    successor_generator_base.def("__repr__",
                                 [](const planners::SuccessorGeneratorBase& generator)
                                 { return "<SuccessorGenerator '" + generator.get_problem()->name + "'>"; });

    py::class_<planners::LiftedSuccessorGenerator, std::shared_ptr<planners::LiftedSuccessorGenerator>>(m, "LiftedSuccessorGenerator", successor_generator_base)
        .def(py::init(&create_lifted_successor_generator));

    py::class_<planners::GroundedSuccessorGenerator, std::shared_ptr<planners::GroundedSuccessorGenerator>>(m,
                                                                                                            "GroundedSuccessorGenerator",
                                                                                                            successor_generator_base)
        .def(py::init(&create_grounded_successor_generator));

    py::class_<formalism::TransitionImpl, formalism::Transition>(m, "Transition")
        .def_readonly("source", &formalism::TransitionImpl::source_state)
        .def_readonly("target", &formalism::TransitionImpl::target_state)
        .def_readonly("action", &formalism::TransitionImpl::action)
        .def("__repr__", [](const formalism::TransitionImpl& transition) { return "<Transition '" + to_string(*transition.action) + "'>"; });

    py::class_<planners::StateSpaceImpl, planners::StateSpace>(m, "StateSpace")
        .def(py::init(&planners::create_state_space), "problem"_a, "successor_generator"_a, "max_expanded"_a = 10'000'000)
        .def_readonly("domain", &planners::StateSpaceImpl::domain)
        .def_readonly("problem", &planners::StateSpaceImpl::problem)
        .def("get_states", &planners::StateSpaceImpl::get_states)
        .def("get_initial_state", &planners::StateSpaceImpl::get_initial_state)
        .def("get_goal_states", &planners::StateSpaceImpl::get_goal_states)
        .def("get_distance_from_initial_state", &planners::StateSpaceImpl::get_distance_from_initial_state, "state"_a)
        .def("get_distance_to_goal_state", &planners::StateSpaceImpl::get_distance_to_goal_state, "state"_a)
        .def("get_longest_distance_to_goal_state", &planners::StateSpaceImpl::get_longest_distance_to_goal_state)
        .def("get_forward_transitions", &planners::StateSpaceImpl::get_forward_transitions, "state"_a)
        .def("get_backward_transitions", &planners::StateSpaceImpl::get_backward_transitions, "state"_a)
        .def("get_unique_id", &planners::StateSpaceImpl::get_unique_index_of_state, "state"_a)
        .def("is_dead_end_state", &planners::StateSpaceImpl::is_dead_end_state, "state"_a)
        .def("is_goal_state", &planners::StateSpaceImpl::is_goal_state, "state"_a)
        .def("sample_state", &planners::StateSpaceImpl::sample_state)
        .def("sample_state_with_distance_to_goal", &planners::StateSpaceImpl::sample_state_with_distance_to_goal, "distance"_a)
        .def("num_states", &planners::StateSpaceImpl::num_states)
        .def("num_dead_end_states", &planners::StateSpaceImpl::num_dead_end_states)
        .def("num_goal_states", &planners::StateSpaceImpl::num_goal_states)
        .def("num_transitions", &planners::StateSpaceImpl::num_transitions)
        .def("__repr__",
             [](const planners::StateSpaceImpl& state_space)
             { return "<StateSpace '" + state_space.problem->name + ": " + std::to_string(state_space.num_states()) + " states'>"; });
}
