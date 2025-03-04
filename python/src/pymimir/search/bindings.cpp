
#include "init_declarations.hpp"

using namespace mimir;

namespace py = pybind11;

/**
 * IPyHeuristic
 *
 * Source: https://pybind11.readthedocs.io/en/stable/advanced/classes.html#overriding-virtual-functions-in-python
 */

class IPyHeuristic : public IHeuristic
{
public:
    /* Inherit the constructors */
    using IHeuristic::IHeuristic;

    /* Trampoline (need one for each virtual function) */
    double compute_heuristic(State state, bool is_goal_state) override
    {
        PYBIND11_OVERRIDE_PURE(double,            /* Return type */
                               IHeuristic,        /* Parent class */
                               compute_heuristic, /* Name of function in C++ (must match Python name) */
                               state,             /* Argument(s) */
                               is_goal_state);
    }
};

/**
 * IPyDynamicAStarAlgorithmEventHandlerBase
 *
 * Source: https://pybind11.readthedocs.io/en/stable/advanced/classes.html#overriding-virtual-functions-in-python
 */

class IPyDynamicAStarAlgorithmEventHandlerBase : public DynamicAStarAlgorithmEventHandlerBase
{
public:
    /* Inherit the constructors */
    using DynamicAStarAlgorithmEventHandlerBase::DynamicAStarAlgorithmEventHandlerBase;

    /* Trampoline (need one for each virtual function) */
    void on_expand_state_impl(State state, const ProblemImpl& problem) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_expand_state_impl, state, problem);
    }

    void on_expand_goal_state_impl(State state, const ProblemImpl& problem) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_expand_goal_state_impl, state, problem);
    }

    void on_generate_state_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_generate_state_impl, state, action, action_cost, successor_state, problem);
    }
    void
    on_generate_state_relaxed_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_generate_state_relaxed_impl, state, action, action_cost, successor_state, problem);
    }
    void
    on_generate_state_not_relaxed_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem) override
    {
        PYBIND11_OVERRIDE(void,
                          DynamicAStarAlgorithmEventHandlerBase,
                          on_generate_state_not_relaxed_impl,
                          state,
                          action,
                          action_cost,
                          successor_state,
                          problem);
    }
    void on_close_state_impl(State state, const ProblemImpl& problem) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_close_state_impl, state, problem);
    }
    void on_finish_f_layer_impl(double f_value, uint64_t num_expanded_state, uint64_t num_generated_states) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_finish_f_layer_impl, f_value, num_expanded_state, num_generated_states);
    }
    void on_prune_state_impl(State state, const ProblemImpl& problem) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_prune_state_impl, state, problem);
    }
    void on_start_search_impl(State start_state, const ProblemImpl& problem) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_start_search_impl, start_state, problem);
    }
    /**
     * Note the trailing commas in the PYBIND11_OVERRIDE calls to name() and bark(). These are needed to portably implement a trampoline for a function that
     * does not take any arguments. For functions that take a nonzero number of arguments, the trailing comma must be omitted.
     */
    void on_end_search_impl(uint64_t num_reached_fluent_atoms,
                            uint64_t num_reached_derived_atoms,
                            uint64_t num_bytes_for_unextended_state_portion,
                            uint64_t num_bytes_for_extended_state_portion,
                            uint64_t num_bytes_for_nodes,
                            uint64_t num_bytes_for_actions,
                            uint64_t num_bytes_for_axioms,
                            uint64_t num_states,
                            uint64_t num_nodes,
                            uint64_t num_actions,
                            uint64_t num_axioms) override
    {
        PYBIND11_OVERRIDE(void,
                          DynamicAStarAlgorithmEventHandlerBase,
                          on_end_search_impl,
                          num_reached_fluent_atoms,
                          num_reached_derived_atoms,
                          num_bytes_for_unextended_state_portion,
                          num_bytes_for_extended_state_portion,
                          num_bytes_for_nodes,
                          num_bytes_for_actions,
                          num_bytes_for_axioms,
                          num_states,
                          num_nodes,
                          num_actions,
                          num_axioms);
    }
    void on_solved_impl(const Plan& plan, const ProblemImpl& problem) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_solved_impl, plan, problem);
    }
    void on_unsolvable_impl() override { PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_unsolvable_impl, ); }
    void on_exhausted_impl() override { PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_exhausted_impl, ); }
};

void init_search(py::module_& m)
{
    /* Enums */
    py::enum_<SearchContext::SearchMode>(m, "SearchMode")
        .value("GROUNDED", SearchContext::SearchMode::GROUNDED)
        .value("LIFTED", SearchContext::SearchMode::LIFTED)
        .export_values();

    py::enum_<SearchNodeStatus>(m, "SearchNodeStatus")
        .value("NEW", SearchNodeStatus::NEW)
        .value("OPEN", SearchNodeStatus::OPEN)
        .value("CLOSED", SearchNodeStatus::CLOSED)
        .value("DEAD_END", SearchNodeStatus::DEAD_END)
        .export_values();

    py::enum_<SearchStatus>(m, "SearchStatus")
        .value("IN_PROGRESS", SearchStatus::IN_PROGRESS)
        .value("OUT_OF_TIME", SearchStatus::OUT_OF_TIME)
        .value("OUT_OF_MEMORY", SearchStatus::OUT_OF_MEMORY)
        .value("FAILED", SearchStatus::FAILED)
        .value("EXHAUSTED", SearchStatus::EXHAUSTED)
        .value("SOLVED", SearchStatus::SOLVED)
        .value("UNSOLVABLE", SearchStatus::UNSOLVABLE)
        .export_values();

    /* SearchContext */

    py::class_<SearchContext::Options>(m, "SearchContextOptions")
        .def(py::init<>())
        .def(py::init<SearchContext::SearchMode>(), py::arg("mode"))
        .def_readwrite("mode", &SearchContext::Options::mode);

    py::class_<SearchContext>(m, "SearchContext")
        .def(py::init([](const std::string& domain_filepath, const std::string& problem_filepath, const SearchContext::Options& options)
                      { return SearchContext(fs::path(domain_filepath), fs::path(problem_filepath), options); }),
             py::arg("domain_filepath"),
             py::arg("problem_filepath"),
             py::arg("options") = SearchContext::Options())

        .def(py::init<Problem, const SearchContext::Options&>(), py::arg("problem"), py::arg("options") = SearchContext::Options())

        .def(py::init<Problem, ApplicableActionGenerator, StateRepository>(),
             py::arg("problem"),
             py::arg("applicable_action_generator"),
             py::arg("state_repository"))

        .def("get_problem", &SearchContext::get_problem)
        .def("get_applicable_action_generator", &SearchContext::get_applicable_action_generator)
        .def("get_state_repository", &SearchContext::get_state_repository);

    /* GeneralizedSearchContext */
    py::class_<GeneralizedSearchContext>(m, "GeneralizedSearchContext")
        .def(py::init(
                 [](const std::string& domain_filepath, const std::vector<std::string>& problem_filepaths, const SearchContext::Options& options)
                 {
                     std::vector<fs::path> paths;
                     paths.reserve(problem_filepaths.size());
                     for (const auto& filepath : problem_filepaths)
                     {
                         paths.emplace_back(filepath);
                     }
                     return GeneralizedSearchContext(fs::path(domain_filepath), paths, options);
                 }),
             py::arg("domain_filepath"),
             py::arg("problem_filepaths"),
             py::arg("options") = SearchContext::Options())

        .def(py::init([](const std::string& domain_filepath, const std::string& problems_directory, const SearchContext::Options& options)
                      { return GeneralizedSearchContext(fs::path(domain_filepath), fs::path(problems_directory), options); }),
             py::arg("domain_filepath"),
             py::arg("problems_directory"),
             py::arg("options") = SearchContext::Options())
        .def(py::init<GeneralizedProblem, const SearchContext::Options&>(), py::arg("generalized_problem"), py::arg("options") = SearchContext::Options())

        .def(py::init<GeneralizedProblem, SearchContextList>(), py::arg("generalized_problem"), py::arg("search_contexts"))

        .def("get_generalized_problem", &GeneralizedSearchContext::get_generalized_problem)
        .def("get_search_contexts", &GeneralizedSearchContext::get_search_contexts);

    /* State */
    py::class_<StateImpl>(m, "State")  //
        .def("__hash__", [](const StateImpl& self) { return self.get_index(); })
        .def("__eq__", [](const StateImpl& lhs, const StateImpl& rhs) { return lhs.get_index() == rhs.get_index(); })
        .def("get_fluent_atoms", py::overload_cast<>(&StateImpl::get_atoms<Fluent>, py::const_))
        .def("get_derived_atoms", py::overload_cast<>(&StateImpl::get_atoms<Derived>, py::const_))
        .def("literal_holds", py::overload_cast<GroundLiteral<Fluent>>(&StateImpl::literal_holds<Fluent>, py::const_), py::arg("literal"))
        .def("literal_holds", py::overload_cast<GroundLiteral<Derived>>(&StateImpl::literal_holds<Derived>, py::const_), py::arg("literal"))
        .def("literals_hold", py::overload_cast<const GroundLiteralList<Fluent>&>(&StateImpl::literals_hold<Fluent>, py::const_), py::arg("literals"))
        .def("literals_hold", py::overload_cast<const GroundLiteralList<Derived>&>(&StateImpl::literals_hold<Derived>, py::const_), py::arg("literals"))
        .def(
            "to_string",
            [](const StateImpl& self, const ProblemImpl& problem)
            {
                std::stringstream ss;
                ss << std::make_tuple(State(&self), std::cref(problem));
                return ss.str();
            },
            py::arg("problem"))
        .def("get_index", py::overload_cast<>(&StateImpl::get_index, py::const_));
    py::bind_vector<StateList>(m, "StateList");

    /* Plan */
    py::class_<Plan>(m, "Plan")  //
        .def("__len__", [](const Plan& arg) { return arg.get_actions().size(); })
        .def("get_actions", &Plan::get_actions)
        .def("get_cost", &Plan::get_cost);

    /* ConjunctiveConditionSatisficingBindingGenerator */
    py::class_<ConjunctiveConditionSatisficingBindingGenerator>(m, "ConjunctiveConditionSatisficingBindingGenerator")  //
        .def(py::init<ConjunctiveCondition, Problem>(), py::arg("conjunctive_condition"), py::arg("problem"))
        .def(
            "generate_ground_conjunctions",
            [](ConjunctiveConditionSatisficingBindingGenerator& self, State state, size_t max_num_groundings)
            {
                auto result =
                    std::vector<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>> {};

                auto count = size_t(0);
                for (const auto& ground_conjunction : self.create_ground_conjunction_generator(state))
                {
                    if (count >= max_num_groundings)
                    {
                        break;
                    }
                    result.push_back(ground_conjunction);
                    ++count;
                }
                return result;
            },
            py::arg("state"),
            py::arg("max_num_groundings"));

    py::class_<ActionSatisficingBindingGenerator>(m, "ActionSatisficingBindingGenerator")  //
        .def(py::init<Action, Problem>(), py::arg("action"), py::arg("problem"))
        .def(
            "generate_ground_conjunctions",
            [](ConjunctiveConditionSatisficingBindingGenerator& self, State state, size_t max_num_groundings)
            {
                auto result =
                    std::vector<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>> {};

                auto count = size_t(0);
                for (const auto& ground_conjunction : self.create_ground_conjunction_generator(state))
                {
                    if (count >= max_num_groundings)
                    {
                        break;
                    }
                    result.push_back(ground_conjunction);
                    ++count;
                }
                return result;
            },
            py::arg("state"),
            py::arg("max_num_groundings"));

    py::class_<AxiomSatisficingBindingGenerator>(m, "AxiomSatisficingBindingGenerator")  //
        .def(py::init<Axiom, Problem>(), py::arg("axiom"), py::arg("problem"))
        .def(
            "generate_ground_conjunctions",
            [](ConjunctiveConditionSatisficingBindingGenerator& self, State state, size_t max_num_groundings)
            {
                auto result =
                    std::vector<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>> {};

                auto count = size_t(0);
                for (const auto& ground_conjunction : self.create_ground_conjunction_generator(state))
                {
                    if (count >= max_num_groundings)
                    {
                        break;
                    }
                    result.push_back(ground_conjunction);
                    ++count;
                }
                return result;
            },
            py::arg("state"),
            py::arg("max_num_groundings"));

    /* ApplicableActionGenerators */
    py::class_<IApplicableActionGenerator, ApplicableActionGenerator>(m, "IApplicableActionGenerator")
        .def("get_problem", &IApplicableActionGenerator::get_problem)
        .def(
            "generate_applicable_actions",
            [](IApplicableActionGenerator& self, State state)
            {
                auto actions = GroundActionList();
                for (const auto& action : self.create_applicable_action_generator(state))
                {
                    actions.push_back(action);
                }
                return actions;
            },
            py::keep_alive<0, 1>(),
            py::arg("state"));

    // Lifted
    py::class_<ILiftedApplicableActionGeneratorEventHandler, LiftedApplicableActionGeneratorEventHandler>(m,
                                                                                                          "ILiftedApplicableActionGeneratorEventHandler");  //
    py::class_<DefaultLiftedApplicableActionGeneratorEventHandler,
               ILiftedApplicableActionGeneratorEventHandler,
               std::shared_ptr<DefaultLiftedApplicableActionGeneratorEventHandler>>(m,
                                                                                    "DefaultLiftedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<DebugLiftedApplicableActionGeneratorEventHandler,
               ILiftedApplicableActionGeneratorEventHandler,
               std::shared_ptr<DebugLiftedApplicableActionGeneratorEventHandler>>(m,
                                                                                  "DebugLiftedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<LiftedApplicableActionGenerator, IApplicableActionGenerator, std::shared_ptr<LiftedApplicableActionGenerator>>(
        m,
        "LiftedApplicableActionGenerator")  //
        .def(py::init<Problem>(), py::arg("problem"))
        .def(py::init<Problem, LiftedApplicableActionGeneratorEventHandler>(), py::arg("problem"), py::arg("event_handler"));

    // Grounded
    py::class_<IGroundedApplicableActionGeneratorEventHandler, GroundedApplicableActionGeneratorEventHandler>(
        m,
        "IGroundedApplicableActionGeneratorEventHandler");  //
    py::class_<DefaultGroundedApplicableActionGeneratorEventHandler,
               IGroundedApplicableActionGeneratorEventHandler,
               std::shared_ptr<DefaultGroundedApplicableActionGeneratorEventHandler>>(m,
                                                                                      "DefaultGroundedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<DebugGroundedApplicableActionGeneratorEventHandler,
               IGroundedApplicableActionGeneratorEventHandler,
               std::shared_ptr<DebugGroundedApplicableActionGeneratorEventHandler>>(m,
                                                                                    "DebugGroundedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<GroundedApplicableActionGenerator, IApplicableActionGenerator, std::shared_ptr<GroundedApplicableActionGenerator>>(
        m,
        "GroundedApplicableActionGenerator");

    /* IAxiomEvaluator */
    py::class_<IAxiomEvaluator, AxiomEvaluator>(m, "IAxiomEvaluator")  //
        .def("get_problem", &IAxiomEvaluator::get_problem);

    // Lifted
    py::class_<ILiftedAxiomEvaluatorEventHandler, LiftedAxiomEvaluatorEventHandler>(m, "ILiftedAxiomEvaluatorEventHandler");  //
    py::class_<DefaultLiftedAxiomEvaluatorEventHandler, ILiftedAxiomEvaluatorEventHandler, std::shared_ptr<DefaultLiftedAxiomEvaluatorEventHandler>>(
        m,
        "DefaultLiftedAxiomEvaluatorEventHandler")  //
        .def(py::init<>());
    py::class_<DebugLiftedAxiomEvaluatorEventHandler, ILiftedAxiomEvaluatorEventHandler, std::shared_ptr<DebugLiftedAxiomEvaluatorEventHandler>>(
        m,
        "DebugLiftedAxiomEvaluatorEventHandler")  //
        .def(py::init<>());
    py::class_<LiftedAxiomEvaluator, IAxiomEvaluator, std::shared_ptr<LiftedAxiomEvaluator>>(m, "LiftedAxiomEvaluator")  //
        .def(py::init<Problem>(), py::arg("problem"))
        .def(py::init<Problem, LiftedAxiomEvaluatorEventHandler>(), py::arg("problem"), py::arg("event_handler"));

    // Grounded
    py::class_<IGroundedAxiomEvaluatorEventHandler, GroundedAxiomEvaluatorEventHandler>(m, "IGroundedAxiomEvaluatorEventHandler");  //
    py::class_<DefaultGroundedAxiomEvaluatorEventHandler, IGroundedAxiomEvaluatorEventHandler, std::shared_ptr<DefaultGroundedAxiomEvaluatorEventHandler>>(
        m,
        "DefaultGroundedAxiomEvaluatorEventHandler")  //
        .def(py::init<>());
    py::class_<DebugGroundedAxiomEvaluatorEventHandler, IGroundedAxiomEvaluatorEventHandler, std::shared_ptr<DebugGroundedAxiomEvaluatorEventHandler>>(
        m,
        "DebugGroundedAxiomEvaluatorEventHandler")  //
        .def(py::init<>());
    py::class_<GroundedAxiomEvaluator, IAxiomEvaluator, std::shared_ptr<GroundedAxiomEvaluator>>(m, "GroundedAxiomEvaluator")  //
        ;

    /* DeleteRelaxedProblemExplorator */

    py::class_<match_tree::Options>(m, "MatchTreeOptions")
        .def(py::init<>())
        .def_readwrite("enable_dump_dot_file", &match_tree::Options::enable_dump_dot_file)
        .def_readwrite("output_dot_file", &match_tree::Options::output_dot_file)
        .def_readwrite("max_num_nodes", &match_tree::Options::max_num_nodes)
        .def_readwrite("split_strategy", &match_tree::Options::split_strategy)
        .def_readwrite("split_metric", &match_tree::Options::split_metric)
        .def_readwrite("optimization_direction", &match_tree::Options::optimization_direction);

    py::class_<DeleteRelaxedProblemExplorator>(m, "DeleteRelaxedProblemExplorator")
        .def(py::init<Problem>(), py::arg("problem"))
        .def("create_grounded_axiom_evaluator",
             &DeleteRelaxedProblemExplorator::create_grounded_axiom_evaluator,
             py::arg("match_tree_options") = match_tree::Options(),
             py::arg("axiom_evaluator_event_handler") = std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>())
        .def("create_grounded_applicable_action_generator",
             &DeleteRelaxedProblemExplorator::create_grounded_applicable_action_generator,
             py::arg("match_tree_options") = match_tree::Options(),
             py::arg("axiom_evaluator_event_handler") = std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>());

    /* StateRepositoryImpl */
    py::class_<StateRepositoryImpl, std::shared_ptr<StateRepositoryImpl>>(m, "StateRepository")  //
        .def(py::init<AxiomEvaluator>(), py::arg("axiom_evaluator"))
        .def("get_or_create_initial_state", &StateRepositoryImpl::get_or_create_initial_state, py::return_value_policy::reference_internal)
        .def("get_or_create_state",
             &StateRepositoryImpl::get_or_create_state,
             py::arg("atoms"),
             py::arg("numeric_variables"),
             py::return_value_policy::reference_internal)
        .def("get_or_create_successor_state",
             py::overload_cast<State, GroundAction, ContinuousCost>(&StateRepositoryImpl::get_or_create_successor_state),
             py::arg("state"),
             py::arg("action"),
             py::arg("state_metric_value"),
             py::return_value_policy::copy)
        .def("get_state_count", &StateRepositoryImpl::get_state_count, py::return_value_policy::copy)
        .def("get_reached_fluent_ground_atoms_bitset", &StateRepositoryImpl::get_reached_fluent_ground_atoms_bitset, py::return_value_policy::copy)
        .def("get_reached_derived_ground_atoms_bitset", &StateRepositoryImpl::get_reached_derived_ground_atoms_bitset, py::return_value_policy::copy);

    /* Heuristics */
    py::class_<IHeuristic, IPyHeuristic, Heuristic>(m, "IHeuristic").def(py::init<>());
    py::class_<BlindHeuristic, IHeuristic, std::shared_ptr<BlindHeuristic>>(m, "BlindHeuristic").def(py::init<Problem>());

    /* Algorithms */

    // SearchResult
    py::class_<SearchResult>(m, "SearchResult")
        .def(py::init<>())
        .def_readwrite("status", &SearchResult::status)
        .def_readwrite("plan", &SearchResult::plan)
        .def_readwrite("goal_state", &SearchResult::goal_state);

    // AStar
    py::class_<AStarAlgorithmStatistics>(m, "AStarAlgorithmStatistics")  //
        .def("get_num_generated", &AStarAlgorithmStatistics::get_num_generated)
        .def("get_num_expanded", &AStarAlgorithmStatistics::get_num_expanded)
        .def("get_num_deadends", &AStarAlgorithmStatistics::get_num_deadends)
        .def("get_num_pruned", &AStarAlgorithmStatistics::get_num_pruned)
        .def("get_num_generated_until_f_value", &AStarAlgorithmStatistics::get_num_generated_until_f_value)
        .def("get_num_expanded_until_f_value", &AStarAlgorithmStatistics::get_num_expanded_until_f_value)
        .def("get_num_deadends_until_f_value", &AStarAlgorithmStatistics::get_num_deadends_until_f_value)
        .def("get_num_pruned_until_f_value", &AStarAlgorithmStatistics::get_num_pruned_until_f_value);
    py::class_<IAStarAlgorithmEventHandler, AStarAlgorithmEventHandler>(m,
                                                                        "IAStarAlgorithmEventHandler")  //
        .def("get_statistics", &IAStarAlgorithmEventHandler::get_statistics);
    py::class_<DefaultAStarAlgorithmEventHandler, IAStarAlgorithmEventHandler, std::shared_ptr<DefaultAStarAlgorithmEventHandler>>(
        m,
        "DefaultAStarAlgorithmEventHandler")  //
        .def(py::init<bool>(), py::arg("quiet") = true);
    py::class_<DebugAStarAlgorithmEventHandler, IAStarAlgorithmEventHandler, std::shared_ptr<DebugAStarAlgorithmEventHandler>>(
        m,
        "DebugAStarAlgorithmEventHandler")  //
        .def(py::init<bool>(), py::arg("quiet") = true);
    py::class_<DynamicAStarAlgorithmEventHandlerBase,
               IAStarAlgorithmEventHandler,
               IPyDynamicAStarAlgorithmEventHandlerBase,
               std::shared_ptr<DynamicAStarAlgorithmEventHandlerBase>>(m,
                                                                       "AStarAlgorithmEventHandlerBase")  //
        .def(py::init<bool>(), py::arg("quiet") = true);

    m.def("find_solution_astar",
          &find_solution_astar,
          py::arg("search_context"),
          py::arg("heuristic") = std::nullopt,
          py::arg("start_state") = std::nullopt,
          py::arg("brfs_event_handler") = std::nullopt,
          py::arg("goal_strategy") = std::nullopt,
          py::arg("pruning_strategy") = std::nullopt);

    // BrFS
    py::class_<BrFSAlgorithmStatistics>(m, "BrFSAlgorithmStatistics")  //
        .def("get_num_generated", &BrFSAlgorithmStatistics::get_num_generated)
        .def("get_num_expanded", &BrFSAlgorithmStatistics::get_num_expanded)
        .def("get_num_deadends", &BrFSAlgorithmStatistics::get_num_deadends)
        .def("get_num_pruned", &BrFSAlgorithmStatistics::get_num_pruned)
        .def("get_num_generated_until_g_value", &BrFSAlgorithmStatistics::get_num_generated_until_g_value)
        .def("get_num_expanded_until_g_value", &BrFSAlgorithmStatistics::get_num_expanded_until_g_value)
        .def("get_num_deadends_until_g_value", &BrFSAlgorithmStatistics::get_num_deadends_until_g_value)
        .def("get_num_pruned_until_g_value", &BrFSAlgorithmStatistics::get_num_pruned_until_g_value);
    py::class_<IBrFSAlgorithmEventHandler, BrFSAlgorithmEventHandler>(m, "IBrFSAlgorithmEventHandler")
        .def("get_statistics", &IBrFSAlgorithmEventHandler::get_statistics);
    py::class_<DefaultBrFSAlgorithmEventHandler, IBrFSAlgorithmEventHandler, std::shared_ptr<DefaultBrFSAlgorithmEventHandler>>(
        m,
        "DefaultBrFSAlgorithmEventHandler")  //
        .def(py::init<>());
    py::class_<DebugBrFSAlgorithmEventHandler, IBrFSAlgorithmEventHandler, std::shared_ptr<DebugBrFSAlgorithmEventHandler>>(
        m,
        "DebugBrFSAlgorithmEventHandler")  //
        .def(py::init<>());

    m.def("find_solution_brfs",
          &find_solution_brfs,
          py::arg("search_context"),
          py::arg("start_state") = std::nullopt,
          py::arg("brfs_event_handler") = std::nullopt,
          py::arg("goal_strategy") = std::nullopt,
          py::arg("pruning_strategy") = std::nullopt,
          py::arg("exhaustive") = false);

    // IW
    py::class_<TupleIndexMapper>(m, "TupleIndexMapper")  //
        .def("to_tuple_index", &TupleIndexMapper::to_tuple_index, py::arg("atom_indices"))
        .def(
            "to_atom_indices",
            [](const TupleIndexMapper& self, const TupleIndex tuple_index)
            {
                auto atom_indices = AtomIndexList {};
                self.to_atom_indices(tuple_index, atom_indices);
                return atom_indices;
            },
            py::arg("tuple_index"))
        .def("tuple_index_to_string", &TupleIndexMapper::tuple_index_to_string, py::arg("tuple_index"))
        .def("get_num_atoms", &TupleIndexMapper::get_num_atoms)
        .def("get_arity", &TupleIndexMapper::get_arity)
        .def("get_factors", &TupleIndexMapper::get_factors, py::return_value_policy::reference_internal)
        .def("get_max_tuple_index", &TupleIndexMapper::get_max_tuple_index)
        .def("get_empty_tuple_index", &TupleIndexMapper::get_empty_tuple_index);

    py::class_<IWAlgorithmStatistics>(m, "IWAlgorithmStatistics")  //
        .def("get_effective_width", &IWAlgorithmStatistics::get_effective_width)
        .def("get_brfs_statistics_by_arity", &IWAlgorithmStatistics::get_brfs_statistics_by_arity);
    py::class_<IIWAlgorithmEventHandler, IWAlgorithmEventHandler>(m, "IIWAlgorithmEventHandler")
        .def("get_statistics", &IIWAlgorithmEventHandler::get_statistics);
    py::class_<DefaultIWAlgorithmEventHandler, IIWAlgorithmEventHandler, std::shared_ptr<DefaultIWAlgorithmEventHandler>>(m, "DefaultIWAlgorithmEventHandler")
        .def(py::init<>());

    m.def("find_solution_iw",
          &find_solution_iw,
          py::arg("search_context"),
          py::arg("start_state") = std::nullopt,
          py::arg("max_arity") = std::nullopt,
          py::arg("iw_event_handler") = std::nullopt,
          py::arg("brfs_event_handler") = std::nullopt,
          py::arg("goal_strategy") = std::nullopt);

    // SIW
    py::class_<SIWAlgorithmStatistics>(m, "SIWAlgorithmStatistics")  //
        .def("get_maximum_effective_width", &SIWAlgorithmStatistics::get_maximum_effective_width)
        .def("get_average_effective_width", &SIWAlgorithmStatistics::get_average_effective_width)
        .def("get_iw_statistics_by_subproblem", &SIWAlgorithmStatistics::get_iw_statistics_by_subproblem);
    py::class_<ISIWAlgorithmEventHandler, SIWAlgorithmEventHandler>(m, "ISIWAlgorithmEventHandler")
        .def("get_statistics", &ISIWAlgorithmEventHandler::get_statistics);
    py::class_<DefaultSIWAlgorithmEventHandler, ISIWAlgorithmEventHandler, std::shared_ptr<DefaultSIWAlgorithmEventHandler>>(m,
                                                                                                                             "DefaultSIWAlgorithmEventHandler")
        .def(py::init<>());

    m.def("find_solution_siw",
          &find_solution_siw,
          py::arg("search_context"),
          py::arg("start_state") = std::nullopt,
          py::arg("max_arity") = std::nullopt,
          py::arg("siw_event_handler") = std::nullopt,
          py::arg("iw_event_handler") = std::nullopt,
          py::arg("brfs_event_handler") = std::nullopt,
          py::arg("goal_strategy") = std::nullopt);
}
