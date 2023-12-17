#include "../include/mimir/formalism/declarations.hpp"
#include "../include/mimir/generators/complete_state_space.hpp"
#include "../include/mimir/generators/goal_matcher.hpp"
#include "../include/mimir/generators/grounded_successor_generator.hpp"
#include "../include/mimir/generators/lifted_successor_generator.hpp"
#include "../include/mimir/generators/successor_generator.hpp"
#include "../include/mimir/generators/successor_generator_factory.hpp"
#include "../include/mimir/pddl/parsers.hpp"
#include "../include/mimir/search/breadth_first_search.hpp"
#include "../include/mimir/search/eager_astar_search.hpp"
#include "../include/mimir/search/heuristics/h1_heuristic.hpp"
#include "../include/mimir/search/heuristics/h2_heuristic.hpp"
#include "../include/mimir/search/heuristics/heuristic_base.hpp"
#include "../include/mimir/search/openlists/open_list_base.hpp"
#include "../include/mimir/search/openlists/priority_queue_open_list.hpp"
#include "../include/mimir/search/search_base.hpp"

#include <Python.h>
#include <memory>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace py::literals;

/// @brief A class that enables enumerating all ground atoms (and bindings) of a conjunction of literals that are true in the given state.
class LiteralGrounder
{
  private:
    mimir::formalism::ActionSchema action_schema_;
    std::unique_ptr<mimir::planners::LiftedSchemaSuccessorGenerator> generator_;

  public:
    LiteralGrounder(const mimir::formalism::ProblemDescription& problem, const mimir::formalism::AtomList& atom_list) :
        action_schema_(nullptr),
        generator_(nullptr)
    {
        std::map<std::string, mimir::formalism::Parameter> parameter_map;
        mimir::formalism::ParameterList parameters;
        mimir::formalism::LiteralList precondition;

        // Create new parameters

        for (const auto& atom : atom_list)
        {
            for (const auto& term : atom->arguments)
            {
                if (term->is_free_variable() && !parameter_map.count(term->name))
                {
                    const auto id = static_cast<uint32_t>(parameter_map.size());
                    const auto new_parameter = mimir::formalism::create_object(id, term->name, term->type);
                    parameter_map.emplace(term->name, new_parameter);
                    parameters.emplace_back(new_parameter);
                }
            }
        }

        // Create new atoms

        for (const auto& atom : atom_list)
        {
            mimir::formalism::ParameterList new_terms;

            for (const auto& term : atom->arguments)
            {
                if (term->is_free_variable())
                {
                    new_terms.emplace_back(parameter_map.at(term->name));
                }
                else
                {
                    new_terms.emplace_back(term);
                }
            }

            const auto new_atom = mimir::formalism::create_atom(atom->predicate, new_terms);
            const auto new_literal = mimir::formalism::create_literal(new_atom, false);
            precondition.emplace_back(new_literal);
        }

        // Create action schema

        const auto unit_cost = mimir::formalism::create_unit_cost_function(problem->domain);
        action_schema_ = mimir::formalism::create_action_schema("dummy", parameters, precondition, {}, {}, unit_cost);
        generator_ = std::make_unique<mimir::planners::LiftedSchemaSuccessorGenerator>(action_schema_, problem);
    }

    std::vector<std::pair<mimir::formalism::AtomList, std::vector<std::pair<std::string, mimir::formalism::Object>>>>
    ground(const mimir::formalism::State& state)
    {
        const auto matches = generator_->get_applicable_actions(state);

        std::vector<std::pair<mimir::formalism::AtomList, std::vector<std::pair<std::string, mimir::formalism::Object>>>> instantiations_and_bindings;

        for (const auto& match : matches)
        {
            const auto& arguments = match->get_arguments();

            mimir::formalism::AtomList instantiation;
            std::vector<std::pair<std::string, mimir::formalism::Object>> binding;

            for (const auto& literal : match->get_precondition())
            {
                instantiation.emplace_back(literal->atom);
            }

            for (std::size_t index = 0; index < action_schema_->parameters.size(); ++index)
            {
                const auto& parameter = action_schema_->parameters.at(index);
                const auto& object = arguments.at(index);
                binding.emplace_back(parameter->name, object);
            }

            instantiations_and_bindings.emplace_back(std::make_pair(std::move(instantiation), std::move(binding)));
        }

        return instantiations_and_bindings;
    }
};

std::string to_string(const mimir::formalism::ActionImpl& action)
{
    std::string repr = action.schema->name + "(";
    const auto& action_arguments = action.get_arguments();

    for (std::size_t index = 0; index < action_arguments.size(); ++index)
    {
        repr += action_arguments[index]->name;

        if ((index + 1) < action_arguments.size())
        {
            repr += ", ";
        }
    }

    return repr + ")";
}

std::string to_string(const mimir::formalism::AtomImpl& state)
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

std::shared_ptr<mimir::parsers::DomainParser> create_domain_parser(const std::string& path) { return std::make_shared<mimir::parsers::DomainParser>(path); }

std::shared_ptr<mimir::parsers::ProblemParser> create_problem_parser(const std::string& path) { return std::make_shared<mimir::parsers::ProblemParser>(path); }

std::shared_ptr<mimir::planners::LiftedSuccessorGenerator> create_lifted_successor_generator(const mimir::formalism::ProblemDescription& problem)
{
    auto successor_generator = mimir::planners::create_sucessor_generator(problem, mimir::planners::SuccessorGeneratorType::LIFTED);
    return std::dynamic_pointer_cast<mimir::planners::LiftedSuccessorGenerator>(successor_generator);
}

std::shared_ptr<mimir::planners::GroundedSuccessorGenerator> create_grounded_successor_generator(const mimir::formalism::ProblemDescription& problem)
{
    auto successor_generator = mimir::planners::create_sucessor_generator(problem, mimir::planners::SuccessorGeneratorType::GROUNDED);
    return std::dynamic_pointer_cast<mimir::planners::GroundedSuccessorGenerator>(successor_generator);
}

bool state_matches_literals(const mimir::formalism::State& state, const mimir::formalism::LiteralList& literals)
{
    return mimir::formalism::literals_hold(literals, state);
}

PYBIND11_MODULE(pymimir, m)
{
    // clang-format off

    m.doc() = "Mimir: Lifted PDDL parsing and expansion library.";

    // Forward declarations

    py::class_<mimir::formalism::TypeImpl, mimir::formalism::Type> type(m, "Type");
    py::class_<mimir::formalism::ObjectImpl, mimir::formalism::Object> object(m, "Object");
    py::class_<mimir::formalism::PredicateImpl, mimir::formalism::Predicate> predicate(m, "Predicate");
    py::class_<mimir::formalism::AtomImpl, mimir::formalism::Atom> atom(m, "Atom");
    py::class_<mimir::formalism::LiteralImpl, mimir::formalism::Literal> literal(m, "Literal");
    py::class_<mimir::formalism::ActionSchemaImpl, mimir::formalism::ActionSchema> action_schema(m, "ActionSchema");
    py::class_<mimir::formalism::DomainImpl, mimir::formalism::DomainDescription> domain(m, "Domain");
    py::class_<mimir::formalism::StateImpl, mimir::formalism::State> state(m, "State");
    py::class_<mimir::formalism::ActionImpl, mimir::formalism::Action> action(m, "Action");
    py::class_<mimir::formalism::ProblemImpl, mimir::formalism::ProblemDescription> problem(m, "Problem");
    py::class_<mimir::parsers::DomainParser, std::shared_ptr<mimir::parsers::DomainParser>> domain_parser(m, "DomainParser");
    py::class_<mimir::parsers::ProblemParser, std::shared_ptr<mimir::parsers::ProblemParser>> problem_parser(m, "ProblemParser");
    py::class_<mimir::planners::SuccessorGeneratorBase, mimir::planners::SuccessorGenerator> successor_generator_base(m, "SuccessorGenerator");
    py::class_<mimir::planners::LiftedSuccessorGenerator, std::shared_ptr<mimir::planners::LiftedSuccessorGenerator>> lifted_successor_generator(m, "LiftedSuccessorGenerator", successor_generator_base);
    py::class_<mimir::planners::GroundedSuccessorGenerator, std::shared_ptr<mimir::planners::GroundedSuccessorGenerator>> grounded_successor_generator(m, "GroundedSuccessorGenerator", successor_generator_base);
    py::class_<mimir::planners::CompleteStateSpaceImpl, mimir::planners::CompleteStateSpace> state_space(m, "StateSpace");
    py::class_<mimir::planners::SearchBase, mimir::planners::Search> search(m, "Search");
    py::class_<mimir::planners::BreadthFirstSearchImpl, mimir::planners::BreadthFirstSearch> breadth_first_search(m, "BreadthFirstSearch", search);
    py::class_<mimir::planners::EagerAStarSearchImpl, mimir::planners::EagerAStarSearch> eager_astar_search(m, "AStarSearch", search);
    py::class_<mimir::planners::OpenListBase<int32_t>, mimir::planners::OpenList> open_list(m, "OpenList");
    py::class_<mimir::planners::PriorityQueueOpenList<int32_t>, std::shared_ptr<mimir::planners::PriorityQueueOpenList<int32_t>>> priority_queue_open_list(m, "PriorityQueueOpenList", open_list);
    py::class_<mimir::planners::HeuristicBase, mimir::planners::Heuristic> heuristic(m, "Heuristic");
    py::class_<mimir::planners::H1Heuristic, std::shared_ptr<mimir::planners::H1Heuristic>> h1_heuristic(m, "H1Heuristic", heuristic);
    py::class_<mimir::planners::H2Heuristic, std::shared_ptr<mimir::planners::H2Heuristic>> h2_heuristic(m, "H2Heuristic", heuristic);
    py::class_<mimir::formalism::TransitionImpl, mimir::formalism::Transition> transition(m, "Transition");
    py::class_<LiteralGrounder, std::shared_ptr<LiteralGrounder>> literal_grounder(m, "LiteralGrounder");
    py::class_<mimir::planners::GoalMatcher, std::shared_ptr<mimir::planners::GoalMatcher>> goal_matcher(m, "GoalMatcher");
    py::class_<mimir::formalism::Implication, std::shared_ptr<mimir::formalism::Implication>> implication(m, "Implication");

    // Definitions

    type.def_readonly("name", &mimir::formalism::TypeImpl::name, "Gets the name of the type.");
    type.def_readonly("base", &mimir::formalism::TypeImpl::base, "Gets the base type of the type.");
    type.def("__repr__", [](const mimir::formalism::TypeImpl& type) { return type.base ? ("<Type '" + type.name + " : " + type.base->name + "'>") : ("<Type '" + type.name + "'>"); });

    object.def(py::init(&mimir::formalism::create_object), "id"_a, "name"_a, "type"_a, "Creates a new object with the given id, name and type.");
    object.def_readonly("id", &mimir::formalism::ObjectImpl::id, "Gets the identifier of the object.");
    object.def_readonly("name", &mimir::formalism::ObjectImpl::name, "Gets the name of the object.");
    object.def_readonly("type", &mimir::formalism::ObjectImpl::type, "Gets the type of the object.");
    object.def("is_variable", &mimir::formalism::ObjectImpl::is_free_variable, "Returns whether the term is a variable.");
    object.def("is_constant", &mimir::formalism::ObjectImpl::is_constant, "Returns whether the term is a constant.");
    object.def("__repr__", [](const mimir::formalism::ObjectImpl& object) { return "<Object '" + object.name + "'>"; });

    predicate.def_readonly("id", &mimir::formalism::PredicateImpl::id, "Gets the identifier of the predicate.");
    predicate.def_readonly("name", &mimir::formalism::PredicateImpl::name, "Gets the name of the predicate.");
    predicate.def_readonly("arity", &mimir::formalism::PredicateImpl::arity, "Gets the arity of the predicate.");
    predicate.def_readonly("parameters", &mimir::formalism::PredicateImpl::parameters, "Gets the parameters of the predicate.");
    predicate.def("as_atom", [](const mimir::formalism::Predicate& predicate) { return mimir::formalism::create_atom(predicate, predicate->parameters); }, "Creates a new atom where all terms are variables.");
    predicate.def("__repr__", [](const mimir::formalism::PredicateImpl& predicate) { return "<Predicate '" + predicate.name + "/" + std::to_string(predicate.arity) + "'>"; });

    atom.def_readonly("predicate", &mimir::formalism::AtomImpl::predicate, "Gets the predicate of the atom");
    atom.def_readonly("terms", &mimir::formalism::AtomImpl::arguments, "Gets the terms of the atom");
    atom.def("replace_term", &mimir::formalism::replace_term, "index"_a, "object"_a, "Replaces a term in the atom");
    atom.def("matches_state", &mimir::formalism::matches_any_in_state, "state"_a, "Tests if any atom matches an atom in the state");
    atom.def("get_name", [](const mimir::formalism::AtomImpl& atom) { return to_string(atom); }, "Gets the name of the atom.");
    atom.def("__repr__", [](const mimir::formalism::AtomImpl& atom) { return "<Atom '" + to_string(atom) + "'>"; });

    literal.def_readonly("atom", &mimir::formalism::LiteralImpl::atom, "Gets the atom of the literal.");
    literal.def_readonly("negated", &mimir::formalism::LiteralImpl::negated, "Returns whether the literal is negated.");
    literal.def("__repr__",
                [](const mimir::formalism::LiteralImpl& literal)
                {
                    const auto prefix = (literal.negated ? std::string("not ") : std::string(""));
                    const auto name = to_string(*literal.atom);
                    return "<Literal '" + prefix + name + "'>";
                });

    action_schema.def_readonly("name", &mimir::formalism::ActionSchemaImpl::name, "Gets the name of the action schema.");
    action_schema.def_readonly("arity", &mimir::formalism::ActionSchemaImpl::arity, "Gets the arity of the action schema.");
    action_schema.def_readonly("parameters", &mimir::formalism::ActionSchemaImpl::parameters, "Gets the parameters of the action schema.");
    action_schema.def_readonly("precondition", &mimir::formalism::ActionSchemaImpl::precondition, "Gets the precondition of the action schema.");
    action_schema.def_readonly("effect", &mimir::formalism::ActionSchemaImpl::unconditional_effect, "Gets the unconditional effect of the action schema.");
    action_schema.def_readonly("conditional_effect", &mimir::formalism::ActionSchemaImpl::conditional_effect, "Gets the conditional effect of the action schema.");
    action_schema.def("__repr__", [](const mimir::formalism::ActionSchemaImpl& action_schema) { return "<ActionSchema '" + action_schema.name + "/" + std::to_string(action_schema.arity) + "'>"; });

    domain.def_readonly("name", &mimir::formalism::DomainImpl::name, "Gets the name of the domain.");
    domain.def_readonly("requirements", &mimir::formalism::DomainImpl::requirements, "Gets the requirements of the domain.");
    domain.def_readonly("types", &mimir::formalism::DomainImpl::types, "Gets the types of the domain.");
    domain.def_readonly("constants", &mimir::formalism::DomainImpl::constants, "Gets the constants of the domain.");
    domain.def_readonly("predicates", &mimir::formalism::DomainImpl::predicates, "Gets the predicates of the domain.");
    domain.def_readonly("static_predicates", &mimir::formalism::DomainImpl::static_predicates, "Gets the static predicates of the domain.");
    domain.def_readonly("action_schemas", &mimir::formalism::DomainImpl::action_schemas, "Gets the action schemas of the domain.");
    domain.def("get_type_map", &mimir::formalism::DomainImpl::get_type_map, "Gets a dictionary mapping type name to type object.");
    domain.def("get_predicate_name_map", &mimir::formalism::DomainImpl::get_predicate_name_map, "Gets a dictionary mapping predicate name to predicate object.");
    domain.def("get_predicate_id_map", &mimir::formalism::DomainImpl::get_predicate_id_map, "Gets a dictionary mapping predicate identifier to predicate object.");
    domain.def("get_constant_map", &mimir::formalism::DomainImpl::get_constant_map, "Gets a dictionary mapping constant name to constant object.");
    domain.def("__repr__", [](const mimir::formalism::DomainImpl& domain) { return "<Domain '" + domain.name + "'>"; });

    state.def("get_atoms", &mimir::formalism::StateImpl::get_atoms, "Gets the atoms of the state.");
    state.def("get_static_atoms", &mimir::formalism::StateImpl::get_static_atoms, "Gets the static atoms of the state.");
    state.def("get_fluent_atoms", &mimir::formalism::StateImpl::get_dynamic_atoms, "Gets the fluent atoms of the state.");
    state.def("get_problem", &mimir::formalism::StateImpl::get_problem, "Gets the problem associated with the state.");
    state.def("get_atoms_by_predicate", &mimir::formalism::StateImpl::get_atoms_grouped_by_predicate, "Gets a dictionary mapping predicates to ground atoms of the given predicate that are true in the state.");
    state.def("pack_object_ids_by_predicate_id", &mimir::formalism::StateImpl::pack_object_ids_by_predicate_id, "include_types"_a, "include_goal"_a, "Gets a dictionary mapping predicate identifiers to a flattened list of object ids, implicitly denoting the atoms true in the state, and a dictionary mapping identifiers to names.");
    state.def("literals_hold", &state_matches_literals, "literals"_a, "Tests whether all ground literals hold with respect to their polarity in the state.");
    state.def("__eq__", &mimir::formalism::StateImpl::operator==);
    state.def("__hash__", &mimir::formalism::StateImpl::hash);
    state.def("__repr__", [](const mimir::formalism::StateImpl& state) { return "<State '" + std::to_string(state.hash()) + "'>"; });

    state.def("matches_all",
        [](const mimir::formalism::State& state, const mimir::formalism::AtomList& atom_list)
        {
            for (const auto& atom : atom_list)
            {
                if (!mimir::formalism::matches_any_in_state(atom, state))
                {
                    return false;
                }
            }

            return true;
        },
        "atom_list"_a, "Tests whether all atoms matches an atom in the state.");

    state.def("matching_bindings",
        [](const mimir::formalism::State& state, const mimir::formalism::AtomList& atom_list)
        {
            LiteralGrounder grounder(state->get_problem(), atom_list);
            return grounder.ground(state);
        },
        "atom_list"_a, "Gets all instantiations (and bindings) of the atom list that matches the state.");

    action.def_readonly("problem", &mimir::formalism::ActionImpl::problem, "Gets the problem associated with the action.");
    action.def_readonly("schema", &mimir::formalism::ActionImpl::schema, "Gets the action schema associated with the action.");
    action.def_readonly("cost", &mimir::formalism::ActionImpl::cost, "Gets the cost of the action.");
    action.def("get_arguments", &mimir::formalism::ActionImpl::get_arguments, "Gets the arguments of the action.");
    action.def("get_precondition", &mimir::formalism::ActionImpl::get_precondition, "Gets the precondition of the action.");
    action.def("get_effect", &mimir::formalism::ActionImpl::get_unconditional_effect, "Gets the unconditional effect of the action.");
    action.def("get_conditional_effect", &mimir::formalism::ActionImpl::get_conditional_effect, "Gets the conditional effect of the action.");
    action.def("get_name", [](const mimir::formalism::ActionImpl& action) { return to_string(action); }, "Gets the name of the action.");
    action.def("is_applicable", &mimir::formalism::is_applicable, "state"_a, "Tests whether the action is applicable in the state.");
    action.def("apply", &mimir::formalism::apply, "state"_a, "Creates a new state state based on the given state and the effect of the action.");
    action.def("__repr__", [](const mimir::formalism::ActionImpl& action) { return "<Action '" + to_string(action) + "'>"; });

    problem.def_readonly("name", &mimir::formalism::ProblemImpl::name, "Gets the name of the problem.");
    problem.def_readonly("domain", &mimir::formalism::ProblemImpl::domain, "Gets the domain associated with the problem.");
    problem.def_readonly("objects", &mimir::formalism::ProblemImpl::objects, "Gets the objects of the problem.");
    problem.def_readonly("initial", &mimir::formalism::ProblemImpl::initial, "Gets the initial atoms of the problem.");
    problem.def_readonly("goal", &mimir::formalism::ProblemImpl::goal, "Gets the goal of the problem.");
    problem.def("replace_initial", &mimir::formalism::ProblemImpl::replace_initial, "initial"_a, "Gets a new object with the given initial atoms.");
    problem.def("create_state", [](const mimir::formalism::ProblemDescription& problem, const mimir::formalism::AtomList& atom_list) { return mimir::formalism::create_state(atom_list, problem); }, "Creates a new state given a list of atoms.");
    problem.def("get_encountered_atoms", &mimir::formalism::ProblemImpl::get_encountered_atoms, "Gets all atoms seen so far.");
    problem.def("__repr__", [](const mimir::formalism::ProblemImpl& problem) { return "<Problem '" + problem.name + "'>"; });

    domain_parser.def(py::init(&create_domain_parser), "path"_a);
    domain_parser.def("parse", (mimir::formalism::DomainDescription (mimir::parsers::DomainParser::*)()) &mimir::parsers::DomainParser::parse, "Parses the associated file and creates a new domain.");

    problem_parser.def(py::init(&create_problem_parser), "path"_a);
    problem_parser.def("parse", (mimir::formalism::ProblemDescription (mimir::parsers::ProblemParser::*)(const mimir::formalism::DomainDescription&)) &mimir::parsers::ProblemParser::parse, "Parses the associated file and creates a new problem.");

    successor_generator_base.def("get_applicable_actions", &mimir::planners::SuccessorGeneratorBase::get_applicable_actions, "state"_a, "Gets all ground actions applicable in the given state.");
    successor_generator_base.def("__repr__", [](const mimir::planners::SuccessorGeneratorBase& generator) { return "<SuccessorGenerator '" + generator.get_problem()->name + "'>"; });

    lifted_successor_generator.def(py::init(&create_lifted_successor_generator), "problem"_a);
    grounded_successor_generator.def(py::init(&create_grounded_successor_generator), "problem"_a);

    search.def("plan", [](const mimir::planners::Search& search) { mimir::formalism::ActionList plan; const auto result = search->plan(plan); return std::make_pair(result == mimir::planners::SearchResult::SOLVED, plan); });
    search.def("abort", &mimir::planners::SearchBase::abort);
    search.def("set_initial_state", &mimir::planners::SearchBase::set_initial_state, "state"_a, "Sets the initial state of the search.");
    search.def("register_callback", &mimir::planners::SearchBase::register_handler, "callback_function"_a, "The callback function will be invoked as the search algorithm progresses.");
    search.def("get_statistics", &mimir::planners::SearchBase::get_statistics, "Get statistics of the search so far.");

    breadth_first_search.def(py::init(&mimir::planners::create_breadth_first_search), "problem"_a, "successor_generator"_a, "Creates a breadth-first search object.");
    eager_astar_search.def(py::init(&mimir::planners::create_eager_astar), "problem"_a, "successor_generator"_a, "heuristic"_a, "open_list"_a, "Creates an A* search object.");

    priority_queue_open_list.def(py::init(&mimir::planners::create_priority_queue_open_list), "Creates a priority queue open list object.");

    h1_heuristic.def(py::init(&mimir::planners::create_h1_heuristic), "problem"_a, "successor_generator"_a, "Creates a h1 heuristic function object.");
    h2_heuristic.def(py::init(&mimir::planners::create_h2_heuristic), "problem"_a, "successor_generator"_a, "Creates a h2 heuristic function object.");

    transition.def_readonly("source", &mimir::formalism::TransitionImpl::source_state, "Gets the source of the transition.");
    transition.def_readonly("target", &mimir::formalism::TransitionImpl::target_state, "Gets the target of the transition.");
    transition.def_readonly("action", &mimir::formalism::TransitionImpl::action, "Gets the action associated with the transition.");
    transition.def("__repr__", [](const mimir::formalism::TransitionImpl& transition) { return "<Transition '" + to_string(*transition.action) + "'>"; });

    state_space.def_static("new", &mimir::planners::create_complete_state_space, "problem"_a, "successor_generator"_a, "max_expanded"_a = 1'000'000);
    state_space.def_readonly("domain", &mimir::planners::CompleteStateSpaceImpl::domain, "Gets the domain associated with the state space.");
    state_space.def_readonly("problem", &mimir::planners::CompleteStateSpaceImpl::problem, "Gets the problem associated with the state space.");
    state_space.def("get_states", &mimir::planners::CompleteStateSpaceImpl::get_states, "Gets all states in the state space.");
    state_space.def("get_initial_state", &mimir::planners::CompleteStateSpaceImpl::get_initial_state, "Gets the initial state of the state space.");
    state_space.def("get_goal_states", &mimir::planners::CompleteStateSpaceImpl::get_goal_states, "Gets all goal states of the state space.");
    state_space.def("get_distance_from_initial_state", &mimir::planners::CompleteStateSpaceImpl::get_distance_from_initial_state, "state"_a, "Gets the distance from the initial state to the given state.");
    state_space.def("get_distance_to_goal_state", &mimir::planners::CompleteStateSpaceImpl::get_distance_to_goal_state, "state"_a, "Gets the distance from the given state to the closest goal state.");
    state_space.def("get_distance_between_states", &mimir::planners::CompleteStateSpaceImpl::get_distance_between_states, "from_state"_a, "to_state"_a, "Gets the distance between the \"from state\" to the \"to state\".");
    state_space.def("get_longest_distance_to_goal_state", &mimir::planners::CompleteStateSpaceImpl::get_longest_distance_to_goal_state, "Gets the longest distance from a state to its closest goal state.");
    state_space.def("get_forward_transitions", &mimir::planners::CompleteStateSpaceImpl::get_forward_transitions, "state"_a, "Gets the possible forward transitions of the given state.");
    state_space.def("get_backward_transitions", &mimir::planners::CompleteStateSpaceImpl::get_backward_transitions, "state"_a, "Gets the possible backward transitions of the given state.");
    state_space.def("get_unique_id", &mimir::planners::CompleteStateSpaceImpl::get_unique_index_of_state, "state"_a, "Gets an unique identifier of the given from 0 to N - 1, where N is the number of states in the state space.");
    state_space.def("is_dead_end_state", &mimir::planners::CompleteStateSpaceImpl::is_dead_end_state, "state"_a, "Tests whether the given state is a dead end state.");
    state_space.def("is_goal_state", &mimir::planners::CompleteStateSpaceImpl::is_goal_state, "state"_a, "Tests whether the given state is a goal state.");
    state_space.def("sample_state", &mimir::planners::CompleteStateSpaceImpl::sample_state, "Gets a uniformly random state from the state space.");
    state_space.def("sample_state_with_distance_to_goal", &mimir::planners::CompleteStateSpaceImpl::sample_state_with_distance_to_goal, "distance"_a, "Gets a uniformly random state from the state space with the given distance to its closest goal state.");
    state_space.def("sample_dead_end_state", &mimir::planners::CompleteStateSpaceImpl::sample_dead_end_state, "Gets a uniformly random dead end state from the state space.");
    state_space.def("num_states", &mimir::planners::CompleteStateSpaceImpl::num_states, "Gets the number of states in the state space.");
    state_space.def("num_dead_end_states", &mimir::planners::CompleteStateSpaceImpl::num_dead_end_states, "Gets the number of dead end states in the state space.");
    state_space.def("num_goal_states", &mimir::planners::CompleteStateSpaceImpl::num_goal_states, "Gets the number of goal states in the state space.");
    state_space.def("num_transitions", &mimir::planners::CompleteStateSpaceImpl::num_transitions, "Gets the number of transitions in the state space.");
    state_space.def("__repr__", [](const mimir::planners::CompleteStateSpaceImpl& state_space) { return "<StateSpace '" + state_space.problem->name + ": " + std::to_string(state_space.num_states()) + " states'>"; });

    literal_grounder.def(py::init([](const mimir::formalism::ProblemDescription& problem, const mimir::formalism::AtomList& atom_list) { return std::make_shared<LiteralGrounder>(problem, atom_list); }), "problem"_a, "atom_list"_a);
    literal_grounder.def("ground", &LiteralGrounder::ground, "state"_a, "Gets a list of instantiations of the associated atom list that are true in the given state.");
    literal_grounder.def("__repr__", [](const LiteralGrounder& grounder){ return "<LiteralGrounder>"; });

    goal_matcher.def(py::init([](const mimir::planners::CompleteStateSpace& state_space) { return std::make_shared<mimir::planners::GoalMatcher>(state_space); }), "state_space"_a);
    goal_matcher.def("best_match", py::overload_cast<const mimir::formalism::AtomList&>(&mimir::planners::GoalMatcher::best_match), "goal"_a);
    goal_matcher.def("best_match", py::overload_cast<const mimir::formalism::State&, const mimir::formalism::AtomList&>(&mimir::planners::GoalMatcher::best_match), "state"_a, "goal"_a);
    goal_matcher.def("__repr__", [](const mimir::planners::GoalMatcher& goal_matcher) { return "<GoalMatcher>"; });

    implication.def_readonly("antecedent", &mimir::formalism::Implication::antecedent, "Gets the antecedent of the implication.");
    implication.def_readonly("consequence", &mimir::formalism::Implication::consequence, "Gets the consequence of the implication.");
    implication.def("__repr__", [](const mimir::formalism::Implication& implication) { return "<Implication>"; });

    // clang-format on
}
