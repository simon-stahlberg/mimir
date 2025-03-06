#include "../init_declarations.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/trampoline.h>

using namespace mimir;

/**
 * IPyHeuristic
 *
 * Source: https://pybind11.readthedocs.io/en/stable/advanced/classes.html#overriding-virtual-functions-in-python
 */

class IPyHeuristic : public IHeuristic
{
public:
    NB_TRAMPOLINE(IHeuristic, 1);

    /* Trampoline (need one for each virtual function) */
    double compute_heuristic(State state, bool is_goal_state) override
    {
        NB_OVERRIDE(compute_heuristic, /* Name of function in C++ (must match Python name) */
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
    NB_TRAMPOLINE(DynamicAStarAlgorithmEventHandlerBase, 13);

    /* Trampoline (need one for each virtual function) */
    void on_expand_state_impl(State state, const ProblemImpl& problem) override { NB_OVERRIDE(on_expand_state_impl, state, problem); }

    void on_expand_goal_state_impl(State state, const ProblemImpl& problem) override { NB_OVERRIDE(on_expand_goal_state_impl, state, problem); }

    void on_generate_state_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem) override
    {
        NB_OVERRIDE(on_generate_state_impl, state, action, action_cost, successor_state, problem);
    }
    void
    on_generate_state_relaxed_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem) override
    {
        NB_OVERRIDE(on_generate_state_relaxed_impl, state, action, action_cost, successor_state, problem);
    }
    void
    on_generate_state_not_relaxed_impl(State state, GroundAction action, ContinuousCost action_cost, State successor_state, const ProblemImpl& problem) override
    {
        NB_OVERRIDE(on_generate_state_not_relaxed_impl, state, action, action_cost, successor_state, problem);
    }
    void on_close_state_impl(State state, const ProblemImpl& problem) override { NB_OVERRIDE(on_close_state_impl, state, problem); }
    void on_finish_f_layer_impl(double f_value, uint64_t num_expanded_state, uint64_t num_generated_states) override
    {
        NB_OVERRIDE(on_finish_f_layer_impl, f_value, num_expanded_state, num_generated_states);
    }
    void on_prune_state_impl(State state, const ProblemImpl& problem) override { NB_OVERRIDE(on_prune_state_impl, state, problem); }
    void on_start_search_impl(State start_state, const ProblemImpl& problem) override { NB_OVERRIDE(on_start_search_impl, start_state, problem); }
    /**
     * Note the trailing commas in the NB_OVERRIDE calls to name() and bark(). These are needed to portably implement a trampoline for a function that
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
        NB_OVERRIDE(on_end_search_impl,
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
    void on_solved_impl(const Plan& plan, const ProblemImpl& problem) override { NB_OVERRIDE(on_solved_impl, plan, problem); }
    void on_unsolvable_impl() override { NB_OVERRIDE(on_unsolvable_impl, ); }
    void on_exhausted_impl() override { NB_OVERRIDE(on_exhausted_impl, ); }
};

void bind_search(nb::module_& m)
{ /* Enums */
    nb::enum_<SearchContext::SearchMode>(m, "SearchMode")
        .value("GROUNDED", SearchContext::SearchMode::GROUNDED)
        .value("LIFTED", SearchContext::SearchMode::LIFTED)
        .export_values();

    nb::enum_<SearchNodeStatus>(m, "SearchNodeStatus")
        .value("NEW", SearchNodeStatus::NEW)
        .value("OPEN", SearchNodeStatus::OPEN)
        .value("CLOSED", SearchNodeStatus::CLOSED)
        .value("DEAD_END", SearchNodeStatus::DEAD_END)
        .export_values();

    nb::enum_<SearchStatus>(m, "SearchStatus")
        .value("IN_PROGRESS", SearchStatus::IN_PROGRESS)
        .value("OUT_OF_TIME", SearchStatus::OUT_OF_TIME)
        .value("OUT_OF_MEMORY", SearchStatus::OUT_OF_MEMORY)
        .value("FAILED", SearchStatus::FAILED)
        .value("EXHAUSTED", SearchStatus::EXHAUSTED)
        .value("SOLVED", SearchStatus::SOLVED)
        .value("UNSOLVABLE", SearchStatus::UNSOLVABLE)
        .export_values();

    /* SearchContext */

    nb::class_<SearchContext::Options>(m, "SearchContextOptions")
        .def(nb::init<>())
        .def(nb::init<SearchContext::SearchMode>(), nb::arg("mode"))
        .def_rw("mode", &SearchContext::Options::mode);

    nb::class_<SearchContext>(m, "SearchContext")
        .def(
            "__init__",
            [](SearchContext* self, const std::string& domain_filepath, const std::string& problem_filepath, const SearchContext::Options& options)
            { new (self) SearchContext(fs::path(domain_filepath), fs::path(problem_filepath), options); },
            nb::arg("domain_filepath"),
            nb::arg("problem_filepath"),
            nb::arg("options") = SearchContext::Options())

        .def(nb::init<Problem, const SearchContext::Options&>(), nb::arg("problem"), nb::arg("options") = SearchContext::Options())

        .def(nb::init<Problem, ApplicableActionGenerator, StateRepository>(),
             nb::arg("problem"),
             nb::arg("applicable_action_generator"),
             nb::arg("state_repository"))

        .def("get_problem", &SearchContext::get_problem)
        .def("get_applicable_action_generator", &SearchContext::get_applicable_action_generator)
        .def("get_state_repository", &SearchContext::get_state_repository);

    /* GeneralizedSearchContext */
    nb::class_<GeneralizedSearchContext>(m, "GeneralizedSearchContext")
        .def(
            "__init__",
            [](GeneralizedSearchContext* self, std::string domain_filepath, std::vector<std::string> problem_filepaths, SearchContext::Options options)
            {
                std::vector<fs::path> paths;
                paths.reserve(problem_filepaths.size());
                for (const auto& filepath : problem_filepaths)
                {
                    paths.emplace_back(filepath);
                }
                new (self) GeneralizedSearchContext(fs::path(std::move(domain_filepath)), std::move(paths), std::move(options));
            },
            nb::arg("domain_filepath"),
            nb::arg("problem_filepaths"),
            nb::arg("options") = SearchContext::Options())

        .def(
            "__init__",
            [](GeneralizedSearchContext* self, std::string domain_filepath, std::string problems_directory, SearchContext::Options options)
            { new (self) GeneralizedSearchContext(fs::path(std::move(domain_filepath)), fs::path(std::move(problems_directory)), std::move(options)); },
            nb::arg("domain_filepath"),
            nb::arg("problems_directory"),
            nb::arg("options") = SearchContext::Options())
        .def(nb::init<GeneralizedProblem, const SearchContext::Options&>(), nb::arg("generalized_problem"), nb::arg("options") = SearchContext::Options())

        .def(nb::init<GeneralizedProblem, SearchContextList>(), nb::arg("generalized_problem"), nb::arg("search_contexts"))

        .def("get_generalized_problem", &GeneralizedSearchContext::get_generalized_problem)
        .def("get_search_contexts", &GeneralizedSearchContext::get_search_contexts);

    /* State */
    nb::class_<StateImpl>(m, "State")  //
        .def("__hash__", [](const StateImpl& self) { return self.get_index(); })
        .def("__eq__", [](const StateImpl& lhs, const StateImpl& rhs) { return lhs.get_index() == rhs.get_index(); })
        .def("get_fluent_atoms", nb::overload_cast<>(&StateImpl::get_atoms<Fluent>, nb::const_))
        .def("get_derived_atoms", nb::overload_cast<>(&StateImpl::get_atoms<Derived>, nb::const_))
        .def("literal_holds", nb::overload_cast<GroundLiteral<Fluent>>(&StateImpl::literal_holds<Fluent>, nb::const_), nb::arg("literal"))
        .def("literal_holds", nb::overload_cast<GroundLiteral<Derived>>(&StateImpl::literal_holds<Derived>, nb::const_), nb::arg("literal"))
        .def("literals_hold", nb::overload_cast<const GroundLiteralList<Fluent>&>(&StateImpl::literals_hold<Fluent>, nb::const_), nb::arg("literals"))
        .def("literals_hold", nb::overload_cast<const GroundLiteralList<Derived>&>(&StateImpl::literals_hold<Derived>, nb::const_), nb::arg("literals"))
        .def(
            "to_string",
            [](const StateImpl& self, const ProblemImpl& problem)
            {
                std::stringstream ss;
                ss << std::make_tuple(State(&self), std::cref(problem));
                return ss.str();
            },
            nb::arg("problem"))
        .def("get_index", nb::overload_cast<>(&StateImpl::get_index, nb::const_));
    nb::bind_vector<StateList>(m, "StateList");

    /* Plan */
    nb::class_<Plan>(m, "Plan")  //
        .def("__len__", [](const Plan& arg) { return arg.get_actions().size(); })
        .def("get_actions", &Plan::get_actions)
        .def("get_cost", &Plan::get_cost);

    /* ConjunctiveConditionSatisficingBindingGenerator */
    nb::class_<ConjunctiveConditionSatisficingBindingGenerator>(m, "ConjunctiveConditionSatisficingBindingGenerator")  //
        .def(nb::init<ConjunctiveCondition, Problem>(), nb::arg("conjunctive_condition"), nb::arg("problem"))
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
            nb::arg("state"),
            nb::arg("max_num_groundings"));

    nb::class_<ActionSatisficingBindingGenerator>(m, "ActionSatisficingBindingGenerator")  //
        .def(nb::init<Action, Problem>(), nb::arg("action"), nb::arg("problem"))
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
            nb::arg("state"),
            nb::arg("max_num_groundings"));

    nb::class_<AxiomSatisficingBindingGenerator>(m, "AxiomSatisficingBindingGenerator")  //
        .def(nb::init<Axiom, Problem>(), nb::arg("axiom"), nb::arg("problem"))
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
            nb::arg("state"),
            nb::arg("max_num_groundings"));

    /* ApplicableActionGenerators */
    nb::class_<IApplicableActionGenerator>(m, "IApplicableActionGenerator")
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
            nb::keep_alive<0, 1>(),
            nb::arg("state"));

    // Lifted
    nb::class_<ILiftedApplicableActionGeneratorEventHandler>(m,
                                                             "ILiftedApplicableActionGeneratorEventHandler");  //
    nb::class_<DefaultLiftedApplicableActionGeneratorEventHandler, ILiftedApplicableActionGeneratorEventHandler>(
        m,
        "DefaultLiftedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<DebugLiftedApplicableActionGeneratorEventHandler, ILiftedApplicableActionGeneratorEventHandler>(
        m,
        "DebugLiftedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<LiftedApplicableActionGenerator, IApplicableActionGenerator>(m,
                                                                            "LiftedApplicableActionGenerator")  //
        .def(nb::init<Problem>(), nb::arg("problem"))
        .def(nb::init<Problem, LiftedApplicableActionGeneratorEventHandler>(), nb::arg("problem"), nb::arg("event_handler"));

    // Grounded
    nb::class_<IGroundedApplicableActionGeneratorEventHandler>(m,
                                                               "IGroundedApplicableActionGeneratorEventHandler");  //
    nb::class_<DefaultGroundedApplicableActionGeneratorEventHandler, IGroundedApplicableActionGeneratorEventHandler>(
        m,
        "DefaultGroundedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<DebugGroundedApplicableActionGeneratorEventHandler, IGroundedApplicableActionGeneratorEventHandler>(
        m,
        "DebugGroundedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<GroundedApplicableActionGenerator, IApplicableActionGenerator>(m, "GroundedApplicableActionGenerator");

    /* IAxiomEvaluator */
    nb::class_<IAxiomEvaluator>(m, "IAxiomEvaluator")  //
        .def("get_problem", &IAxiomEvaluator::get_problem);

    // Lifted
    nb::class_<ILiftedAxiomEvaluatorEventHandler>(m, "ILiftedAxiomEvaluatorEventHandler");  //
    nb::class_<DefaultLiftedAxiomEvaluatorEventHandler, ILiftedAxiomEvaluatorEventHandler>(m,
                                                                                           "DefaultLiftedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<DebugLiftedAxiomEvaluatorEventHandler, ILiftedAxiomEvaluatorEventHandler>(m,
                                                                                         "DebugLiftedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<LiftedAxiomEvaluator, IAxiomEvaluator>(m, "LiftedAxiomEvaluator")  //
        .def(nb::init<Problem>(), nb::arg("problem"))
        .def(nb::init<Problem, LiftedAxiomEvaluatorEventHandler>(), nb::arg("problem"), nb::arg("event_handler"));

    // Grounded
    nb::class_<IGroundedAxiomEvaluatorEventHandler>(m, "IGroundedAxiomEvaluatorEventHandler");  //
    nb::class_<DefaultGroundedAxiomEvaluatorEventHandler, IGroundedAxiomEvaluatorEventHandler>(m,
                                                                                               "DefaultGroundedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<DebugGroundedAxiomEvaluatorEventHandler, IGroundedAxiomEvaluatorEventHandler>(m,
                                                                                             "DebugGroundedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<GroundedAxiomEvaluator, IAxiomEvaluator>(m, "GroundedAxiomEvaluator")  //
        ;

    /* DeleteRelaxedProblemExplorator */

    nb::class_<match_tree::Options>(m, "MatchTreeOptions")
        .def(nb::init<>())
        .def_rw("enable_dump_dot_file", &match_tree::Options::enable_dump_dot_file)
        .def_rw("output_dot_file", &match_tree::Options::output_dot_file)
        .def_rw("max_num_nodes", &match_tree::Options::max_num_nodes)
        .def_rw("split_strategy", &match_tree::Options::split_strategy)
        .def_rw("split_metric", &match_tree::Options::split_metric)
        .def_rw("optimization_direction", &match_tree::Options::optimization_direction);

    nb::class_<DeleteRelaxedProblemExplorator>(m, "DeleteRelaxedProblemExplorator")
        .def(nb::init<Problem>(), nb::arg("problem"))
        .def("create_grounded_axiom_evaluator",
             &DeleteRelaxedProblemExplorator::create_grounded_axiom_evaluator,
             nb::arg("match_tree_options") = match_tree::Options(),
             nb::arg("axiom_evaluator_event_handler") = std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>())
        .def("create_grounded_applicable_action_generator",
             &DeleteRelaxedProblemExplorator::create_grounded_applicable_action_generator,
             nb::arg("match_tree_options") = match_tree::Options(),
             nb::arg("axiom_evaluator_event_handler") = std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>());

    /* StateRepositoryImpl */
    nb::class_<StateRepositoryImpl>(m, "StateRepository")  //
        .def(nb::init<AxiomEvaluator>(), nb::arg("axiom_evaluator"))
        .def("get_or_create_initial_state", &StateRepositoryImpl::get_or_create_initial_state, nb::rv_policy::reference_internal)
        .def("get_or_create_state",
             &StateRepositoryImpl::get_or_create_state,
             nb::arg("atoms"),
             nb::arg("numeric_variables"),
             nb::rv_policy::reference_internal)
        .def("get_or_create_successor_state",
             nb::overload_cast<State, GroundAction, ContinuousCost>(&StateRepositoryImpl::get_or_create_successor_state),
             nb::arg("state"),
             nb::arg("action"),
             nb::arg("state_metric_value"),
             nb::rv_policy::copy)
        .def("get_state_count", &StateRepositoryImpl::get_state_count, nb::rv_policy::copy)
        .def("get_reached_fluent_ground_atoms_bitset", &StateRepositoryImpl::get_reached_fluent_ground_atoms_bitset, nb::rv_policy::copy)
        .def("get_reached_derived_ground_atoms_bitset", &StateRepositoryImpl::get_reached_derived_ground_atoms_bitset, nb::rv_policy::copy);

    /* Heuristics */
    nb::class_<IHeuristic, IPyHeuristic>(m, "IHeuristic").def(nb::init<>());
    nb::class_<BlindHeuristic, IHeuristic>(m, "BlindHeuristic").def(nb::init<Problem>());

    /* Algorithms */

    // SearchResult
    nb::class_<SearchResult>(m, "SearchResult")
        .def(nb::init<>())
        .def_rw("status", &SearchResult::status)
        .def_rw("plan", &SearchResult::plan)
        .def_rw("goal_state", &SearchResult::goal_state);

    // AStar
    nb::class_<AStarAlgorithmStatistics>(m, "AStarAlgorithmStatistics")  //
        .def("get_num_generated", &AStarAlgorithmStatistics::get_num_generated)
        .def("get_num_expanded", &AStarAlgorithmStatistics::get_num_expanded)
        .def("get_num_deadends", &AStarAlgorithmStatistics::get_num_deadends)
        .def("get_num_pruned", &AStarAlgorithmStatistics::get_num_pruned)
        .def("get_num_generated_until_f_value", &AStarAlgorithmStatistics::get_num_generated_until_f_value)
        .def("get_num_expanded_until_f_value", &AStarAlgorithmStatistics::get_num_expanded_until_f_value)
        .def("get_num_deadends_until_f_value", &AStarAlgorithmStatistics::get_num_deadends_until_f_value)
        .def("get_num_pruned_until_f_value", &AStarAlgorithmStatistics::get_num_pruned_until_f_value);
    nb::class_<IAStarAlgorithmEventHandler>(m,
                                            "IAStarAlgorithmEventHandler")  //
        .def("get_statistics", &IAStarAlgorithmEventHandler::get_statistics);
    nb::class_<DefaultAStarAlgorithmEventHandler, IAStarAlgorithmEventHandler>(m,
                                                                               "DefaultAStarAlgorithmEventHandler")  //
        .def(nb::init<bool>(), nb::arg("quiet") = true);
    nb::class_<DebugAStarAlgorithmEventHandler, IAStarAlgorithmEventHandler>(m,
                                                                             "DebugAStarAlgorithmEventHandler")  //
        .def(nb::init<bool>(), nb::arg("quiet") = true);
    nb::class_<DynamicAStarAlgorithmEventHandlerBase, IAStarAlgorithmEventHandler, IPyDynamicAStarAlgorithmEventHandlerBase>(
        m,
        "AStarAlgorithmEventHandlerBase")  //
        .def(nb::init<bool>(), nb::arg("quiet") = true);

    m.def("find_solution_astar",
          &find_solution_astar,
          nb::arg("search_context"),
          nb::arg("heuristic") = std::nullopt,
          nb::arg("start_state") = std::nullopt,
          nb::arg("brfs_event_handler") = std::nullopt,
          nb::arg("goal_strategy") = std::nullopt,
          nb::arg("pruning_strategy") = std::nullopt);

    // BrFS
    nb::class_<BrFSAlgorithmStatistics>(m, "BrFSAlgorithmStatistics")  //
        .def("get_num_generated", &BrFSAlgorithmStatistics::get_num_generated)
        .def("get_num_expanded", &BrFSAlgorithmStatistics::get_num_expanded)
        .def("get_num_deadends", &BrFSAlgorithmStatistics::get_num_deadends)
        .def("get_num_pruned", &BrFSAlgorithmStatistics::get_num_pruned)
        .def("get_num_generated_until_g_value", &BrFSAlgorithmStatistics::get_num_generated_until_g_value)
        .def("get_num_expanded_until_g_value", &BrFSAlgorithmStatistics::get_num_expanded_until_g_value)
        .def("get_num_deadends_until_g_value", &BrFSAlgorithmStatistics::get_num_deadends_until_g_value)
        .def("get_num_pruned_until_g_value", &BrFSAlgorithmStatistics::get_num_pruned_until_g_value);
    nb::class_<IBrFSAlgorithmEventHandler>(m, "IBrFSAlgorithmEventHandler").def("get_statistics", &IBrFSAlgorithmEventHandler::get_statistics);
    nb::class_<DefaultBrFSAlgorithmEventHandler, IBrFSAlgorithmEventHandler>(m,
                                                                             "DefaultBrFSAlgorithmEventHandler")  //
        .def(nb::init<>());
    nb::class_<DebugBrFSAlgorithmEventHandler, IBrFSAlgorithmEventHandler>(m,
                                                                           "DebugBrFSAlgorithmEventHandler")  //
        .def(nb::init<>());

    m.def("find_solution_brfs",
          &find_solution_brfs,
          nb::arg("search_context"),
          nb::arg("start_state") = std::nullopt,
          nb::arg("brfs_event_handler") = std::nullopt,
          nb::arg("goal_strategy") = std::nullopt,
          nb::arg("pruning_strategy") = std::nullopt,
          nb::arg("exhaustive") = false);

    // IW
    nb::class_<TupleIndexMapper>(m, "TupleIndexMapper")  //
        .def("to_tuple_index", &TupleIndexMapper::to_tuple_index, nb::arg("atom_indices"))
        .def(
            "to_atom_indices",
            [](const TupleIndexMapper& self, const TupleIndex tuple_index)
            {
                auto atom_indices = AtomIndexList {};
                self.to_atom_indices(tuple_index, atom_indices);
                return atom_indices;
            },
            nb::arg("tuple_index"))
        .def("tuple_index_to_string", &TupleIndexMapper::tuple_index_to_string, nb::arg("tuple_index"))
        .def("get_num_atoms", &TupleIndexMapper::get_num_atoms)
        .def("get_arity", &TupleIndexMapper::get_arity)
        .def("get_factors", &TupleIndexMapper::get_factors, nb::rv_policy::reference_internal)
        .def("get_max_tuple_index", &TupleIndexMapper::get_max_tuple_index)
        .def("get_empty_tuple_index", &TupleIndexMapper::get_empty_tuple_index);

    nb::class_<IWAlgorithmStatistics>(m, "IWAlgorithmStatistics")  //
        .def("get_effective_width", &IWAlgorithmStatistics::get_effective_width)
        .def("get_brfs_statistics_by_arity", &IWAlgorithmStatistics::get_brfs_statistics_by_arity);
    nb::class_<IIWAlgorithmEventHandler>(m, "IIWAlgorithmEventHandler").def("get_statistics", &IIWAlgorithmEventHandler::get_statistics);
    nb::class_<DefaultIWAlgorithmEventHandler, IIWAlgorithmEventHandler>(m, "DefaultIWAlgorithmEventHandler").def(nb::init<>());

    m.def("find_solution_iw",
          &find_solution_iw,
          nb::arg("search_context"),
          nb::arg("start_state") = std::nullopt,
          nb::arg("max_arity") = std::nullopt,
          nb::arg("iw_event_handler") = std::nullopt,
          nb::arg("brfs_event_handler") = std::nullopt,
          nb::arg("goal_strategy") = std::nullopt);

    // SIW
    nb::class_<SIWAlgorithmStatistics>(m, "SIWAlgorithmStatistics")  //
        .def("get_maximum_effective_width", &SIWAlgorithmStatistics::get_maximum_effective_width)
        .def("get_average_effective_width", &SIWAlgorithmStatistics::get_average_effective_width)
        .def("get_iw_statistics_by_subproblem", &SIWAlgorithmStatistics::get_iw_statistics_by_subproblem);
    nb::class_<ISIWAlgorithmEventHandler>(m, "ISIWAlgorithmEventHandler")  //
        .def("get_statistics", &ISIWAlgorithmEventHandler::get_statistics);
    nb::class_<DefaultSIWAlgorithmEventHandler, ISIWAlgorithmEventHandler>(m, "DefaultSIWAlgorithmEventHandler").def(nb::init<>());

    m.def("find_solution_siw",
          &find_solution_siw,
          nb::arg("search_context"),
          nb::arg("start_state") = std::nullopt,
          nb::arg("max_arity") = std::nullopt,
          nb::arg("siw_event_handler") = std::nullopt,
          nb::arg("iw_event_handler") = std::nullopt,
          nb::arg("brfs_event_handler") = std::nullopt,
          nb::arg("goal_strategy") = std::nullopt);
}