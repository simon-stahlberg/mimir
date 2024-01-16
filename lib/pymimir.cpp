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
    mimir::ActionSchema action_schema_;
    std::unique_ptr<mimir::planners::LiftedSchemaSuccessorGenerator> generator_;

  public:
    LiteralGrounder(const mimir::ProblemDescription& problem, const mimir::AtomList& atom_list) :
        action_schema_(nullptr),
        generator_(nullptr)
    {
        std::map<std::string, mimir::Parameter> parameter_map;
        mimir::ParameterList parameters;
        mimir::LiteralList precondition;

        // Create new parameters

        for (const auto& atom : atom_list)
        {
            for (const auto& term : atom->arguments)
            {
                if (term->is_free_variable() && !parameter_map.count(term->name))
                {
                    const auto id = static_cast<uint32_t>(parameter_map.size());
                    const auto new_parameter = mimir::create_object(id, term->name, term->type);
                    parameter_map.emplace(term->name, new_parameter);
                    parameters.emplace_back(new_parameter);
                }
            }
        }

        // Create new atoms

        for (const auto& atom : atom_list)
        {
            mimir::ParameterList new_terms;

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

            const auto new_atom = mimir::create_atom(atom->predicate, new_terms);
            const auto new_literal = mimir::create_literal(new_atom, false);
            precondition.emplace_back(new_literal);
        }

        // Create action schema

        const auto unit_cost = mimir::create_unit_cost_function(problem->domain);
        action_schema_ = mimir::create_action_schema("dummy", parameters, precondition, {}, {}, unit_cost);
        generator_ = std::make_unique<mimir::planners::LiftedSchemaSuccessorGenerator>(action_schema_, problem);
    }

    std::vector<std::pair<mimir::AtomList, std::vector<std::pair<std::string, mimir::Object>>>>
    ground(const mimir::State& state)
    {
        const auto matches = generator_->get_applicable_actions(state);

        std::vector<std::pair<mimir::AtomList, std::vector<std::pair<std::string, mimir::Object>>>> instantiations_and_bindings;

        for (const auto& match : matches)
        {
            const auto& arguments = match->get_arguments();

            mimir::AtomList instantiation;
            std::vector<std::pair<std::string, mimir::Object>> binding;

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

std::string to_string(const mimir::ActionImpl& action)
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

std::string to_string(const mimir::AtomImpl& state)
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

std::shared_ptr<mimir::planners::LiftedSuccessorGenerator> create_lifted_successor_generator(const mimir::ProblemDescription& problem)
{
    auto successor_generator = mimir::planners::create_sucessor_generator(problem, mimir::planners::SuccessorGeneratorType::LIFTED);
    return std::dynamic_pointer_cast<mimir::planners::LiftedSuccessorGenerator>(successor_generator);
}

std::shared_ptr<mimir::planners::GroundedSuccessorGenerator> create_grounded_successor_generator(const mimir::ProblemDescription& problem)
{
    auto successor_generator = mimir::planners::create_sucessor_generator(problem, mimir::planners::SuccessorGeneratorType::GROUNDED);
    return std::dynamic_pointer_cast<mimir::planners::GroundedSuccessorGenerator>(successor_generator);
}

bool state_matches_literals(const mimir::State& state, const mimir::LiteralList& literals)
{
    return mimir::literals_hold(literals, state);
}

PYBIND11_MODULE(pymimir, m)
{
    // clang-format off

    m.doc() = "Mimir: Lifted PDDL parsing and expansion library.";

    // Forward declarations

    py::class_<mimir::TypeImpl, mimir::Type> type(m, "Type");
    py::class_<mimir::ObjectImpl, mimir::Object> object(m, "Object");
    py::class_<mimir::PredicateImpl, mimir::Predicate> predicate(m, "Predicate");
    py::class_<mimir::AtomImpl, mimir::Atom> atom(m, "Atom");
    py::class_<mimir::LiteralImpl, mimir::Literal> literal(m, "Literal");
    py::class_<mimir::ActionSchemaImpl, mimir::ActionSchema> action_schema(m, "ActionSchema");
    py::class_<mimir::DomainImpl, mimir::DomainDescription> domain(m, "Domain");
    py::class_<mimir::StateImpl, mimir::State> state(m, "State");
    py::class_<mimir::ActionImpl, mimir::Action> action(m, "Action");
    py::class_<mimir::ProblemImpl, mimir::ProblemDescription> problem(m, "Problem");
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
    py::class_<mimir::TransitionImpl, mimir::Transition> transition(m, "Transition");
    py::class_<LiteralGrounder, std::shared_ptr<LiteralGrounder>> literal_grounder(m, "LiteralGrounder");
    py::class_<mimir::planners::GoalMatcher, std::shared_ptr<mimir::planners::GoalMatcher>> goal_matcher(m, "GoalMatcher");
    py::class_<mimir::Implication, std::shared_ptr<mimir::Implication>> implication(m, "Implication");

    // Definitions

    type.def_readonly("name", &mimir::TypeImpl::name, "Gets the name of the type.");
    type.def_readonly("base", &mimir::TypeImpl::base, "Gets the base type of the type.");
    type.def("__repr__", [](const mimir::TypeImpl& type) { return type.base ? ("<Type '" + type.name + " : " + type.base->name + "'>") : ("<Type '" + type.name + "'>"); });

    object.def(py::init(&mimir::create_object), "id"_a, "name"_a, "type"_a, "Creates a new object with the given id, name and type.");
    object.def_readonly("id", &mimir::ObjectImpl::id, "Gets the identifier of the object.");
    object.def_readonly("name", &mimir::ObjectImpl::name, "Gets the name of the object.");
    object.def_readonly("type", &mimir::ObjectImpl::type, "Gets the type of the object.");
    object.def("is_variable", &mimir::ObjectImpl::is_free_variable, "Returns whether the term is a variable.");
    object.def("is_constant", &mimir::ObjectImpl::is_constant, "Returns whether the term is a constant.");
    object.def("__repr__", [](const mimir::ObjectImpl& object) { return "<Object '" + object.name + "'>"; });

    predicate.def_readonly("id", &mimir::PredicateImpl::id, "Gets the identifier of the predicate.");
    predicate.def_readonly("name", &mimir::PredicateImpl::name, "Gets the name of the predicate.");
    predicate.def_readonly("arity", &mimir::PredicateImpl::arity, "Gets the arity of the predicate.");
    predicate.def_readonly("parameters", &mimir::PredicateImpl::parameters, "Gets the parameters of the predicate.");
    predicate.def("as_atom", [](const mimir::Predicate& predicate) { return mimir::create_atom(predicate, predicate->parameters); }, "Creates a new atom where all terms are variables.");
    predicate.def("__repr__", [](const mimir::PredicateImpl& predicate) { return "<Predicate '" + predicate.name + "/" + std::to_string(predicate.arity) + "'>"; });

    atom.def_readonly("predicate", &mimir::AtomImpl::predicate, "Gets the predicate of the atom");
    atom.def_readonly("terms", &mimir::AtomImpl::arguments, "Gets the terms of the atom");
    atom.def("replace_term", &mimir::replace_term, "index"_a, "object"_a, "Replaces a term in the atom");
    atom.def("matches_state", &mimir::matches_any_in_state, "state"_a, "Tests if any atom matches an atom in the state");
    atom.def("get_name", [](const mimir::AtomImpl& atom) { return to_string(atom); }, "Gets the name of the atom.");
    atom.def("__repr__", [](const mimir::AtomImpl& atom) { return "<Atom '" + to_string(atom) + "'>"; });

    literal.def_readonly("atom", &mimir::LiteralImpl::atom, "Gets the atom of the literal.");
    literal.def_readonly("negated", &mimir::LiteralImpl::negated, "Returns whether the literal is negated.");
    literal.def("__repr__",
                [](const mimir::LiteralImpl& literal)
                {
                    const auto prefix = (literal.negated ? std::string("not ") : std::string(""));
                    const auto name = to_string(*literal.atom);
                    return "<Literal '" + prefix + name + "'>";
                });

    action_schema.def_readonly("name", &mimir::ActionSchemaImpl::name, "Gets the name of the action schema.");
    action_schema.def_readonly("arity", &mimir::ActionSchemaImpl::arity, "Gets the arity of the action schema.");
    action_schema.def_readonly("parameters", &mimir::ActionSchemaImpl::parameters, "Gets the parameters of the action schema.");
    action_schema.def_readonly("precondition", &mimir::ActionSchemaImpl::precondition, "Gets the precondition of the action schema.");
    action_schema.def_readonly("effect", &mimir::ActionSchemaImpl::unconditional_effect, "Gets the unconditional effect of the action schema.");
    action_schema.def_readonly("conditional_effect", &mimir::ActionSchemaImpl::conditional_effect, "Gets the conditional effect of the action schema.");
    action_schema.def("__repr__", [](const mimir::ActionSchemaImpl& action_schema) { return "<ActionSchema '" + action_schema.name + "/" + std::to_string(action_schema.arity) + "'>"; });

    domain.def_readonly("name", &mimir::DomainImpl::name, "Gets the name of the domain.");
    domain.def_readonly("requirements", &mimir::DomainImpl::requirements, "Gets the requirements of the domain.");
    domain.def_readonly("types", &mimir::DomainImpl::types, "Gets the types of the domain.");
    domain.def_readonly("constants", &mimir::DomainImpl::constants, "Gets the constants of the domain.");
    domain.def_readonly("predicates", &mimir::DomainImpl::predicates, "Gets the predicates of the domain.");
    domain.def_readonly("static_predicates", &mimir::DomainImpl::static_predicates, "Gets the static predicates of the domain.");
    domain.def_readonly("action_schemas", &mimir::DomainImpl::action_schemas, "Gets the action schemas of the domain.");
    domain.def("get_type_map", &mimir::DomainImpl::get_type_map, "Gets a dictionary mapping type name to type object.");
    domain.def("get_predicate_name_map", &mimir::DomainImpl::get_predicate_name_map, "Gets a dictionary mapping predicate name to predicate object.");
    domain.def("get_predicate_id_map", &mimir::DomainImpl::get_predicate_id_map, "Gets a dictionary mapping predicate identifier to predicate object.");
    domain.def("get_constant_map", &mimir::DomainImpl::get_constant_map, "Gets a dictionary mapping constant name to constant object.");
    domain.def("__repr__", [](const mimir::DomainImpl& domain) { return "<Domain '" + domain.name + "'>"; });

    state.def("get_atoms", &mimir::StateImpl::get_atoms, "Gets the atoms of the state.");
    state.def("get_static_atoms", &mimir::StateImpl::get_static_atoms, "Gets the static atoms of the state.");
    state.def("get_fluent_atoms", &mimir::StateImpl::get_dynamic_atoms, "Gets the fluent atoms of the state.");
    state.def("get_problem", &mimir::StateImpl::get_problem, "Gets the problem associated with the state.");
    state.def("get_atoms_by_predicate", &mimir::StateImpl::get_atoms_grouped_by_predicate, "Gets a dictionary mapping predicates to ground atoms of the given predicate that are true in the state.");
    state.def("pack_object_ids_by_predicate_id", &mimir::StateImpl::pack_object_ids_by_predicate_id, "include_types"_a, "include_goal"_a, "Gets a dictionary mapping predicate identifiers to a flattened list of object ids, implicitly denoting the atoms true in the state, and a dictionary mapping identifiers to names.");
    state.def("literals_hold", &state_matches_literals, "literals"_a, "Tests whether all ground literals hold with respect to their polarity in the state.");
    state.def("__eq__", &mimir::StateImpl::operator==);
    state.def("__hash__", &mimir::StateImpl::hash);
    state.def("__repr__", [](const mimir::StateImpl& state) { return "<State '" + std::to_string(state.hash()) + "'>"; });

    state.def("matches_all",
        [](const mimir::State& state, const mimir::AtomList& atom_list)
        {
            for (const auto& atom : atom_list)
            {
                if (!mimir::matches_any_in_state(atom, state))
                {
                    return false;
                }
            }

            return true;
        },
        "atom_list"_a, "Tests whether all atoms matches an atom in the state.");

    state.def("matching_bindings",
        [](const mimir::State& state, const mimir::AtomList& atom_list)
        {
            LiteralGrounder grounder(state->get_problem(), atom_list);
            return grounder.ground(state);
        },
        "atom_list"_a, "Gets all instantiations (and bindings) of the atom list that matches the state.");

    action.def_readonly("problem", &mimir::ActionImpl::problem, "Gets the problem associated with the action.");
    action.def_readonly("schema", &mimir::ActionImpl::schema, "Gets the action schema associated with the action.");
    action.def_readonly("cost", &mimir::ActionImpl::cost, "Gets the cost of the action.");
    action.def("get_arguments", &mimir::ActionImpl::get_arguments, "Gets the arguments of the action.");
    action.def("get_precondition", &mimir::ActionImpl::get_precondition, "Gets the precondition of the action.");
    action.def("get_effect", &mimir::ActionImpl::get_unconditional_effect, "Gets the unconditional effect of the action.");
    action.def("get_conditional_effect", &mimir::ActionImpl::get_conditional_effect, "Gets the conditional effect of the action.");
    action.def("get_name", [](const mimir::ActionImpl& action) { return to_string(action); }, "Gets the name of the action.");
    action.def("is_applicable", &mimir::is_applicable, "state"_a, "Tests whether the action is applicable in the state.");
    action.def("apply", &mimir::apply, "state"_a, "Creates a new state state based on the given state and the effect of the action.");
    action.def("__repr__", [](const mimir::ActionImpl& action) { return "<Action '" + to_string(action) + "'>"; });

    problem.def_readonly("name", &mimir::ProblemImpl::name, "Gets the name of the problem.");
    problem.def_readonly("domain", &mimir::ProblemImpl::domain, "Gets the domain associated with the problem.");
    problem.def_readonly("objects", &mimir::ProblemImpl::objects, "Gets the objects of the problem.");
    problem.def_readonly("initial", &mimir::ProblemImpl::initial, "Gets the initial atoms of the problem.");
    problem.def_readonly("goal", &mimir::ProblemImpl::goal, "Gets the goal of the problem.");
    problem.def("replace_initial", &mimir::ProblemImpl::replace_initial, "initial"_a, "Gets a new object with the given initial atoms.");
    problem.def("create_state", [](const mimir::ProblemDescription& problem, const mimir::AtomList& atom_list) { return mimir::create_state(atom_list, problem); }, "Creates a new state given a list of atoms.");
    problem.def("get_encountered_atoms", &mimir::ProblemImpl::get_encountered_atoms, "Gets all atoms seen so far.");
    problem.def("__repr__", [](const mimir::ProblemImpl& problem) { return "<Problem '" + problem.name + "'>"; });

    domain_parser.def(py::init(&create_domain_parser), "path"_a);
    domain_parser.def("parse", (mimir::DomainDescription (mimir::parsers::DomainParser::*)()) &mimir::parsers::DomainParser::parse, "Parses the associated file and creates a new domain.");

    problem_parser.def(py::init(&create_problem_parser), "path"_a);
    problem_parser.def("parse", (mimir::ProblemDescription (mimir::parsers::ProblemParser::*)(const mimir::DomainDescription&)) &mimir::parsers::ProblemParser::parse, "Parses the associated file and creates a new problem.");

    successor_generator_base.def("get_applicable_actions", &mimir::planners::SuccessorGeneratorBase::get_applicable_actions, "state"_a, "Gets all ground actions applicable in the given state.");
    successor_generator_base.def("__repr__", [](const mimir::planners::SuccessorGeneratorBase& generator) { return "<SuccessorGenerator '" + generator.get_problem()->name + "'>"; });

    lifted_successor_generator.def(py::init(&create_lifted_successor_generator), "problem"_a);
    grounded_successor_generator.def(py::init(&create_grounded_successor_generator), "problem"_a);

    search.def("plan", [](const mimir::planners::Search& search) { mimir::ActionList plan; const auto result = search->plan(plan); return std::make_pair(result == mimir::planners::SearchResult::SOLVED, plan); });
    search.def("abort", &mimir::planners::SearchBase::abort);
    search.def("set_initial_state", &mimir::planners::SearchBase::set_initial_state, "state"_a, "Sets the initial state of the search.");
    search.def("register_callback", &mimir::planners::SearchBase::register_handler, "callback_function"_a, "The callback function will be invoked as the search algorithm progresses.");
    search.def("get_statistics", &mimir::planners::SearchBase::get_statistics, "Get statistics of the search so far.");

    breadth_first_search.def(py::init(&mimir::planners::create_breadth_first_search), "problem"_a, "successor_generator"_a, "Creates a breadth-first search object.");
    eager_astar_search.def(py::init(&mimir::planners::create_eager_astar), "problem"_a, "successor_generator"_a, "heuristic"_a, "open_list"_a, "Creates an A* search object.");

    priority_queue_open_list.def(py::init(&mimir::planners::create_priority_queue_open_list), "Creates a priority queue open list object.");

    h1_heuristic.def(py::init(&mimir::planners::create_h1_heuristic), "problem"_a, "successor_generator"_a, "Creates a h1 heuristic function object.");
    h2_heuristic.def(py::init(&mimir::planners::create_h2_heuristic), "problem"_a, "successor_generator"_a, "Creates a h2 heuristic function object.");

    transition.def_readonly("source", &mimir::TransitionImpl::source_state, "Gets the source of the transition.");
    transition.def_readonly("target", &mimir::TransitionImpl::target_state, "Gets the target of the transition.");
    transition.def_readonly("action", &mimir::TransitionImpl::action, "Gets the action associated with the transition.");
    transition.def("__repr__", [](const mimir::TransitionImpl& transition) { return "<Transition '" + to_string(*transition.action) + "'>"; });

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

    literal_grounder.def(py::init([](const mimir::ProblemDescription& problem, const mimir::AtomList& atom_list) { return std::make_shared<LiteralGrounder>(problem, atom_list); }), "problem"_a, "atom_list"_a);
    literal_grounder.def("ground", &LiteralGrounder::ground, "state"_a, "Gets a list of instantiations of the associated atom list that are true in the given state.");
    literal_grounder.def("__repr__", [](const LiteralGrounder& grounder){ return "<LiteralGrounder>"; });

    goal_matcher.def(py::init([](const mimir::planners::CompleteStateSpace& state_space) { return std::make_shared<mimir::planners::GoalMatcher>(state_space); }), "state_space"_a);
    goal_matcher.def("best_match", py::overload_cast<const mimir::AtomList&>(&mimir::planners::GoalMatcher::best_match), "goal"_a);
    goal_matcher.def("best_match", py::overload_cast<const mimir::State&, const mimir::AtomList&>(&mimir::planners::GoalMatcher::best_match), "state"_a, "goal"_a);
    goal_matcher.def("__repr__", [](const mimir::planners::GoalMatcher& goal_matcher) { return "<GoalMatcher>"; });

    implication.def_readonly("antecedent", &mimir::Implication::antecedent, "Gets the antecedent of the implication.");
    implication.def_readonly("consequence", &mimir::Implication::consequence, "Gets the consequence of the implication.");
    implication.def("__repr__", [](const mimir::Implication& implication) { return "<Implication>"; });

    // clang-format on
}
