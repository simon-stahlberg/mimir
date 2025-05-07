#include "../init_declarations.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/trampoline.h>

using namespace mimir::formalism;

namespace mimir::search
{

class IPyGoalStrategy : public IGoalStrategy
{
public:
    NB_TRAMPOLINE(IGoalStrategy, 2);

    /* Trampoline (need one for each virtual function) */
    bool test_static_goal() override { NB_OVERRIDE_PURE(test_static_goal); }

    bool test_dynamic_goal(State state) override { NB_OVERRIDE_PURE(test_dynamic_goal, state); }
};

class IPyPruningStrategy : public IPruningStrategy
{
public:
    NB_TRAMPOLINE(IPruningStrategy, 2);

    /* Trampoline (need one for each virtual function) */
    bool test_prune_initial_state(State state) override { NB_OVERRIDE_PURE(test_prune_initial_state, state); }

    bool test_prune_successor_state(State state, State succ_state, bool is_new_succ) override
    {
        NB_OVERRIDE_PURE(test_prune_successor_state, state, succ_state, is_new_succ);
    }
};

class IPyHeuristic : public IHeuristic
{
public:
    NB_TRAMPOLINE(IHeuristic, 1);

    /* Trampoline (need one for each virtual function) */
    double compute_heuristic(State state, bool is_goal_state) override
    {
        NB_OVERRIDE_PURE(compute_heuristic, /* Name of function in C++ (must match Python name) */
                         state,             /* Argument(s) */
                         is_goal_state);
    }
};

class IPyAStarEventHandler : public astar::IEventHandler
{
public:
    NB_TRAMPOLINE(astar::IEventHandler, 14);

    /* Trampoline (need one for each virtual function) */
    void on_expand_state(State state) override { NB_OVERRIDE_PURE(on_expand_state, state); }

    void on_expand_goal_state(State state) override { NB_OVERRIDE_PURE(on_expand_goal_state, state); }

    void on_generate_state(State state, GroundAction action, ContinuousCost action_cost, State successor_state) override
    {
        NB_OVERRIDE_PURE(on_generate_state, state, action, action_cost, successor_state);
    }
    void on_generate_state_relaxed(State state, GroundAction action, ContinuousCost action_cost, State successor_state) override
    {
        NB_OVERRIDE_PURE(on_generate_state_relaxed, state, action, action_cost, successor_state);
    }
    void on_generate_state_not_relaxed(State state, GroundAction action, ContinuousCost action_cost, State successor_state) override
    {
        NB_OVERRIDE_PURE(on_generate_state_not_relaxed, state, action, action_cost, successor_state);
    }
    void on_close_state(State state) override { NB_OVERRIDE_PURE(on_close_state, state); }
    void on_finish_f_layer(double f_value) override { NB_OVERRIDE_PURE(on_finish_f_layer, f_value); }
    void on_prune_state(State state) override { NB_OVERRIDE_PURE(on_prune_state, state); }
    void on_start_search(State start_state) override { NB_OVERRIDE_PURE(on_start_search, start_state); }
    void on_end_search(uint64_t num_reached_fluent_atoms,
                       uint64_t num_reached_derived_atoms,
                       uint64_t num_bytes_for_problem,
                       uint64_t num_bytes_for_nodes,
                       uint64_t num_states,
                       uint64_t num_nodes,
                       uint64_t num_actions,
                       uint64_t num_axioms) override
    {
        NB_OVERRIDE_PURE(on_end_search,
                         num_reached_fluent_atoms,
                         num_reached_derived_atoms,
                         num_bytes_for_problem,
                         num_bytes_for_nodes,
                         num_states,
                         num_nodes,
                         num_actions,
                         num_axioms);
    }
    void on_solved(const Plan& plan) override { NB_OVERRIDE_PURE(on_solved, plan); }
    void on_unsolvable() override { NB_OVERRIDE_PURE(on_unsolvable); }
    void on_exhausted() override { NB_OVERRIDE_PURE(on_exhausted); }
    const astar::Statistics& get_statistics() const override { NB_OVERRIDE_PURE(get_statistics); }
};

class IPyBrFSEventHandler : public brfs::IEventHandler
{
public:
    NB_TRAMPOLINE(brfs::IEventHandler, 12);

    /* Trampoline (need one for each virtual function) */
    void on_expand_state(State state) override { NB_OVERRIDE_PURE(on_expand_state, state); }

    void on_expand_goal_state(State state) override { NB_OVERRIDE_PURE(on_expand_goal_state, state); }

    void on_generate_state(State state, GroundAction action, ContinuousCost action_cost, State successor_state) override
    {
        NB_OVERRIDE_PURE(on_generate_state, state, action, action_cost, successor_state);
    }
    void on_generate_state_in_search_tree(State state, GroundAction action, ContinuousCost action_cost, State successor_state) override
    {
        NB_OVERRIDE_PURE(on_generate_state_in_search_tree, state, action, action_cost, successor_state);
    }
    void on_generate_state_not_in_search_tree(State state, GroundAction action, ContinuousCost action_cost, State successor_state) override
    {
        NB_OVERRIDE_PURE(on_generate_state_not_in_search_tree, state, action, action_cost, successor_state);
    }

    void on_finish_g_layer(DiscreteCost g_value) override { NB_OVERRIDE_PURE(on_finish_g_layer, g_value); }
    void on_start_search(State start_state) override { NB_OVERRIDE_PURE(on_start_search, start_state); }
    void on_end_search(uint64_t num_reached_fluent_atoms,
                       uint64_t num_reached_derived_atoms,
                       uint64_t num_bytes_for_problem,
                       uint64_t num_bytes_for_nodes,
                       uint64_t num_states,
                       uint64_t num_nodes,
                       uint64_t num_actions,
                       uint64_t num_axioms) override
    {
        NB_OVERRIDE_PURE(on_end_search,
                         num_reached_fluent_atoms,
                         num_reached_derived_atoms,
                         num_bytes_for_problem,
                         num_bytes_for_nodes,
                         num_states,
                         num_nodes,
                         num_actions,
                         num_axioms);
    }
    void on_solved(const Plan& plan) override { NB_OVERRIDE_PURE(on_solved, plan); }
    void on_unsolvable() override { NB_OVERRIDE_PURE(on_unsolvable); }
    void on_exhausted() override { NB_OVERRIDE_PURE(on_exhausted); }
    const brfs::Statistics& get_statistics() const override { NB_OVERRIDE_PURE(get_statistics); }
};

class IPyGBFSEventHandler : public gbfs::IEventHandler
{
public:
    NB_TRAMPOLINE(gbfs::IEventHandler, 10);

    /* Trampoline (need one for each virtual function) */
    void on_expand_state(State state) override { NB_OVERRIDE_PURE(on_expand_state, state); }

    void on_expand_goal_state(State state) override { NB_OVERRIDE_PURE(on_expand_goal_state, state); }

    void on_generate_state(State state, GroundAction action, ContinuousCost action_cost, State successor_state) override
    {
        NB_OVERRIDE_PURE(on_generate_state, state, action, action_cost, successor_state);
    }
    void on_prune_state(State state) override { NB_OVERRIDE_PURE(on_prune_state, state); }
    void on_start_search(State start_state) override { NB_OVERRIDE_PURE(on_start_search, start_state); }
    void on_end_search(uint64_t num_reached_fluent_atoms,
                       uint64_t num_reached_derived_atoms,
                       uint64_t num_bytes_for_problem,
                       uint64_t num_bytes_for_nodes,
                       uint64_t num_states,
                       uint64_t num_nodes,
                       uint64_t num_actions,
                       uint64_t num_axioms) override
    {
        NB_OVERRIDE_PURE(on_end_search,
                         num_reached_fluent_atoms,
                         num_reached_derived_atoms,
                         num_bytes_for_problem,
                         num_bytes_for_nodes,
                         num_states,
                         num_nodes,
                         num_actions,
                         num_axioms);
    }
    void on_solved(const Plan& plan) override { NB_OVERRIDE_PURE(on_solved, plan); }
    void on_unsolvable() override { NB_OVERRIDE_PURE(on_unsolvable); }
    void on_exhausted() override { NB_OVERRIDE_PURE(on_exhausted); }
    const gbfs::Statistics& get_statistics() const override { NB_OVERRIDE_PURE(get_statistics); }
};

void bind_module_definitions(nb::module_& m)
{
    /* Enums */
    nb::enum_<SearchContextImpl::SearchMode>(m, "SearchMode")
        .value("GROUNDED", SearchContextImpl::SearchMode::GROUNDED)
        .value("LIFTED", SearchContextImpl::SearchMode::LIFTED)
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

    nb::enum_<match_tree::SplitMetricEnum>(m, "MatchTreeSplitMetric")
        .value("FREQUENCY", match_tree::SplitMetricEnum::FREQUENCY)
        .value("GINI", match_tree::SplitMetricEnum::GINI);

    nb::enum_<match_tree::SplitStrategyEnum>(m, "MatchTreeSplitStrategy")  //
        .value("DYNAMIC", match_tree::SplitStrategyEnum::DYNAMIC);

    nb::enum_<match_tree::OptimizationDirectionEnum>(m, "MatchTreeOptimizationDirection")
        .value("MINIMIZE", match_tree::OptimizationDirectionEnum::MINIMIZE)
        .value("MAXIMIZE", match_tree::OptimizationDirectionEnum::MAXIMIZE);

    /* SearchContext */

    nb::class_<SearchContextImpl::Options>(m, "SearchContextOptions")
        .def(nb::init<>())
        .def(nb::init<SearchContextImpl::SearchMode>(), "mode"_a)
        .def_rw("mode", &SearchContextImpl::Options::mode);

    nb::class_<SearchContextImpl>(m, "SearchContext")
        .def_static(
            "create",
            [](const fs::path& domain_filepath, const fs::path& problem_filepath, const SearchContextImpl::Options& options) -> SearchContext
            { return SearchContextImpl::create(domain_filepath, problem_filepath, options); },
            "domain_filepath"_a,
            "problem_filepath"_a,
            "options"_a = SearchContextImpl::Options())
        .def_static("create",
                    nb::overload_cast<formalism::Problem, const SearchContextImpl::Options&>(&SearchContextImpl::create),
                    "problem"_a,
                    "options"_a = SearchContextImpl::Options())
        .def_static("create",
                    nb::overload_cast<formalism::Problem, ApplicableActionGenerator, StateRepository>(&SearchContextImpl::create),
                    "problem"_a,
                    "applicable_action_generator"_a,
                    "state_repository"_a)
        .def("get_problem", &SearchContextImpl::get_problem)
        .def("get_applicable_action_generator", &SearchContextImpl::get_applicable_action_generator)
        .def("get_state_repository", &SearchContextImpl::get_state_repository);

    /* GeneralizedSearchContext */
    nb::class_<GeneralizedSearchContextImpl>(m, "GeneralizedSearchContext")
        .def_static(
            "create",
            [](const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const SearchContextImpl::Options& options)
                -> GeneralizedSearchContext { return GeneralizedSearchContextImpl::create(domain_filepath, problem_filepaths, options); },
            "domain_filepath"_a,
            "problem_filepaths"_a,
            "options"_a = SearchContextImpl::Options())
        .def_static(
            "create",
            [](const fs::path& domain_filepath, const fs::path& problems_directory, const SearchContextImpl::Options& options) -> GeneralizedSearchContext
            { return GeneralizedSearchContextImpl::create(domain_filepath, problems_directory, options); },
            "domain_filepath"_a,
            "problems_directory"_a,
            "options"_a = SearchContextImpl::Options())
        .def_static("create",
                    nb::overload_cast<formalism::GeneralizedProblem, const SearchContextImpl::Options&>(&GeneralizedSearchContextImpl::create),
                    "generalized_problem"_a,
                    "options"_a = SearchContextImpl::Options())
        .def_static("create",
                    nb::overload_cast<formalism::GeneralizedProblem, SearchContextList>(&GeneralizedSearchContextImpl::create),
                    "generalized_problem"_a,
                    "search_contexts"_a)
        .def("get_generalized_problem", &GeneralizedSearchContextImpl::get_generalized_problem)
        .def("get_search_contexts", &GeneralizedSearchContextImpl::get_search_contexts);

    /* State */
    nb::class_<StateImpl>(m, "State")  //
        .def("__hash__", [](const StateImpl& self) { return std::hash<State> {}(&self); })
        .def("__eq__", [](const StateImpl& lhs, const StateImpl& rhs) { return &lhs == &rhs; })
        .def(
            "get_fluent_atoms",
            [](const StateImpl& self)
            {
                return nb::make_iterator(nb::type<nb::iterator>(),
                                         "Iterator over fluent state atom indices.",
                                         self.get_atoms<FluentTag>().begin(),
                                         self.get_atoms<FluentTag>().end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_derived_atoms",
            [](const StateImpl& self)
            {
                return nb::make_iterator(nb::type<nb::iterator>(),
                                         "Iterator over derived state atom indices.",
                                         self.get_atoms<DerivedTag>().begin(),
                                         self.get_atoms<DerivedTag>().end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "to_string",
            [](const StateImpl& self, const ProblemImpl& problem)
            {
                std::stringstream ss;
                mimir::operator<<(ss, std::make_tuple(State(&self), std::cref(problem)));
                return ss.str();
            },
            "problem"_a)
        .def("literal_holds", &StateImpl::literal_holds<FluentTag>, nb::rv_policy::copy, "literal"_a)
        .def("literal_holds", &StateImpl::literal_holds<DerivedTag>, nb::rv_policy::copy, "literal"_a)
        .def("literal_holds", &StateImpl::literals_hold<FluentTag>, nb::rv_policy::copy, "literals"_a)
        .def("literal_holds", &StateImpl::literals_hold<DerivedTag>, nb::rv_policy::copy, "literals"_a)
        .def("get_index", &StateImpl::get_index);
    nb::bind_vector<StateList>(m, "StateList");

    /* Plan */
    nb::class_<Plan>(m, "Plan")  //
        .def("__len__", [](const Plan& arg) { return arg.get_actions().size(); })
        .def("get_actions", &Plan::get_actions)
        .def("get_cost", &Plan::get_cost);

    /* ConjunctiveConditionSatisficingBindingGenerator */
    nb::class_<ConjunctiveConditionSatisficingBindingGenerator>(m, "ConjunctiveConditionSatisficingBindingGenerator")  //
        .def(nb::init<ConjunctiveCondition, Problem>(), "conjunctive_condition"_a, "problem"_a)
        .def(
            "generate_ground_conjunctions",
            [](ConjunctiveConditionSatisficingBindingGenerator& self, State state, size_t max_num_groundings)
            {
                auto result = std::vector<
                    std::pair<ObjectList, std::tuple<GroundLiteralList<StaticTag>, GroundLiteralList<FluentTag>, GroundLiteralList<DerivedTag>>>> {};

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
            "state"_a,
            "max_num_groundings"_a);

    nb::class_<ActionSatisficingBindingGenerator>(m, "ActionSatisficingBindingGenerator")  //
        .def(nb::init<Action, Problem>(), "action"_a, "problem"_a)
        .def(
            "generate_ground_conjunctions",
            [](ConjunctiveConditionSatisficingBindingGenerator& self, State state, size_t max_num_groundings)
            {
                auto result = std::vector<
                    std::pair<ObjectList, std::tuple<GroundLiteralList<StaticTag>, GroundLiteralList<FluentTag>, GroundLiteralList<DerivedTag>>>> {};

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
            "state"_a,
            "max_num_groundings"_a);

    nb::class_<AxiomSatisficingBindingGenerator>(m, "AxiomSatisficingBindingGenerator")  //
        .def(nb::init<Axiom, Problem>(), "axiom"_a, "problem"_a)
        .def(
            "generate_ground_conjunctions",
            [](ConjunctiveConditionSatisficingBindingGenerator& self, State state, size_t max_num_groundings)
            {
                auto result = std::vector<
                    std::pair<ObjectList, std::tuple<GroundLiteralList<StaticTag>, GroundLiteralList<FluentTag>, GroundLiteralList<DerivedTag>>>> {};

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
            "state"_a,
            "max_num_groundings"_a);

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
            "state"_a);

    // Lifted
    nb::class_<LiftedApplicableActionGeneratorImpl::IEventHandler>(m,
                                                                   "ILiftedApplicableActionGeneratorEventHandler");  //
    nb::class_<LiftedApplicableActionGeneratorImpl::DefaultEventHandlerImpl, LiftedApplicableActionGeneratorImpl::IEventHandler>(
        m,
        "DefaultLiftedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<LiftedApplicableActionGeneratorImpl::DebugEventHandlerImpl, LiftedApplicableActionGeneratorImpl::IEventHandler>(
        m,
        "DebugLiftedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<LiftedApplicableActionGeneratorImpl, IApplicableActionGenerator>(m,
                                                                                "LiftedApplicableActionGenerator")  //
        .def(nb::init<Problem>(), "problem"_a)
        .def(nb::init<Problem, LiftedApplicableActionGeneratorImpl::EventHandler>(), "problem"_a, "event_handler"_a);

    // Grounded
    nb::class_<GroundedApplicableActionGeneratorImpl::IEventHandler>(m,
                                                                     "IGroundedApplicableActionGeneratorEventHandler");  //
    nb::class_<GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl, GroundedApplicableActionGeneratorImpl::IEventHandler>(
        m,
        "DefaultGroundedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<GroundedApplicableActionGeneratorImpl::DebugEventHandlerImpl, GroundedApplicableActionGeneratorImpl::IEventHandler>(
        m,
        "DebugGroundedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<GroundedApplicableActionGeneratorImpl, IApplicableActionGenerator>(m, "GroundedApplicableActionGenerator");

    /* IAxiomEvaluator */
    nb::class_<IAxiomEvaluator>(m, "IAxiomEvaluator")  //
        .def("get_problem", &IAxiomEvaluator::get_problem);

    // Lifted
    nb::class_<LiftedAxiomEvaluatorImpl::IEventHandler>(m, "ILiftedAxiomEvaluatorEventHandler");  //
    nb::class_<LiftedAxiomEvaluatorImpl::DefaultEventHandlerImpl, LiftedAxiomEvaluatorImpl::IEventHandler>(m,
                                                                                                           "DefaultLiftedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<LiftedAxiomEvaluatorImpl::DebugEventHandlerImpl, LiftedAxiomEvaluatorImpl::IEventHandler>(m,
                                                                                                         "DebugLiftedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<LiftedAxiomEvaluatorImpl, IAxiomEvaluator>(m, "LiftedAxiomEvaluator")  //
        .def(nb::init<Problem>(), "problem"_a)
        .def(nb::init<Problem, LiftedAxiomEvaluatorImpl::EventHandler>(), "problem"_a, "event_handler"_a);

    // Grounded
    nb::class_<GroundedAxiomEvaluatorImpl::IEventHandler>(m, "IGroundedAxiomEvaluatorEventHandler");  //
    nb::class_<GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl, GroundedAxiomEvaluatorImpl::IEventHandler>(m,
                                                                                                               "DefaultGroundedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<GroundedAxiomEvaluatorImpl::DebugEventHandlerImpl, GroundedAxiomEvaluatorImpl::IEventHandler>(m,
                                                                                                             "DebugGroundedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<GroundedAxiomEvaluatorImpl, IAxiomEvaluator>(m, "GroundedAxiomEvaluator")  //
        ;

    /* StateRepositoryImpl */
    m.def("compute_state_metric_value", &compute_state_metric_value, "state"_a, "problem"_a);

    nb::class_<StateRepositoryImpl>(m, "StateRepository")  //
        .def(nb::init<AxiomEvaluator>(), "axiom_evaluator"_a)
        .def("get_or_create_initial_state", &StateRepositoryImpl::get_or_create_initial_state, nb::rv_policy::reference_internal)
        .def("get_or_create_state", &StateRepositoryImpl::get_or_create_state, "atoms"_a, "numeric_variables"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_successor_state",
             nb::overload_cast<State, GroundAction, ContinuousCost>(&StateRepositoryImpl::get_or_create_successor_state),
             "state"_a,
             "action"_a,
             "state_metric_value"_a,
             nb::rv_policy::copy)
        .def("get_state_count", &StateRepositoryImpl::get_state_count, nb::rv_policy::copy)
        .def("get_reached_fluent_ground_atoms_bitset", &StateRepositoryImpl::get_reached_fluent_ground_atoms_bitset, nb::rv_policy::copy)
        .def("get_reached_derived_ground_atoms_bitset", &StateRepositoryImpl::get_reached_derived_ground_atoms_bitset, nb::rv_policy::copy);

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
        .def(nb::init<Problem>(), "problem"_a)
        .def("create_grounded_axiom_evaluator",
             &DeleteRelaxedProblemExplorator::create_grounded_axiom_evaluator,
             "match_tree_options"_a = match_tree::Options(),
             "axiom_evaluator_event_handler"_a = nullptr)
        .def("create_grounded_applicable_action_generator",
             &DeleteRelaxedProblemExplorator::create_grounded_applicable_action_generator,
             "match_tree_options"_a = match_tree::Options(),
             "axiom_evaluator_event_handler"_a = nullptr);

    /* Heuristics */
    nb::class_<IHeuristic, IPyHeuristic>(m, "IHeuristic")  //
        .def(nb::init<>())
        .def("compute_heuristic", &IHeuristic::compute_heuristic, "state"_a, "is_goal_state"_a);

    nb::class_<BlindHeuristicImpl, IHeuristic>(m, "BlindHeuristic").def(nb::init<Problem>());

    /* Algorithms */

    // SearchResult
    nb::class_<SearchResult>(m, "SearchResult")
        .def(nb::init<>())
        .def_rw("status", &SearchResult::status)
        .def_rw("plan", &SearchResult::plan)
        .def_rw("goal_state", &SearchResult::goal_state);

    // GoalStrategy
    nb::class_<IGoalStrategy, IPyGoalStrategy>(m, "IGoalStrategy")
        .def(nb::init<>())
        .def("test_static_goal", &IGoalStrategy::test_static_goal)
        .def("test_dynamic_goal", &IGoalStrategy::test_dynamic_goal, "state"_a);

    nb::class_<ProblemGoalStrategyImpl, IGoalStrategy>(m, "ProblemGoalStrategy")  //
        .def(nb::init<Problem>(), "problem"_a)
        .def_static("create", &ProblemGoalStrategyImpl::create, "problem"_a);

    // PruningStrategy
    nb::class_<IPruningStrategy, IPyPruningStrategy>(m, "IPruningStrategy")
        .def(nb::init<>())
        .def("test_prune_initial_state", &IPruningStrategy::test_prune_initial_state, "initial_state"_a)
        .def("test_prune_successor_state", &IPruningStrategy::test_prune_successor_state, "state"_a, "successor_state"_a, "is_new_successor"_a);

    nb::class_<NoPruningStrategyImpl, IPruningStrategy>(m, "NoPruningStrategy")  //
        .def(nb::init<>())
        .def_static("create", &NoPruningStrategyImpl::create);

    nb::class_<DuplicatePruningStrategyImpl, IPruningStrategy>(m, "DuplicatePruningStrategy")  //
        .def(nb::init<>())
        .def_static("create", &DuplicatePruningStrategyImpl::create);

    nb::class_<iw::ArityZeroNoveltyPruningStrategyImpl, IPruningStrategy>(m, "ArityZeroNoveltyPruningStrategy")  //
        .def(nb::init<State>(), "initial_state"_a)
        .def_static("create", &iw::ArityZeroNoveltyPruningStrategyImpl::create, "initial_state"_a);

    nb::class_<iw::ArityKNoveltyPruningStrategyImpl, IPruningStrategy>(m, "ArityKNoveltyPruningStrategy")  //
        .def(nb::init<size_t, size_t>(), "arity"_a, "num_atoms"_a)
        .def_static("create", &iw::ArityKNoveltyPruningStrategyImpl::create, "arity"_a, "num_atoms"_a);

    // AStar
    nb::class_<astar::Statistics>(m, "AStarStatistics")  //
        .def(nb::init<>())
        .def("get_num_generated", &astar::Statistics::get_num_generated)
        .def("get_num_expanded", &astar::Statistics::get_num_expanded)
        .def("get_num_deadends", &astar::Statistics::get_num_deadends)
        .def("get_num_pruned", &astar::Statistics::get_num_pruned)
        .def("get_num_generated_until_f_value", &astar::Statistics::get_num_generated_until_f_value)
        .def("get_num_expanded_until_f_value", &astar::Statistics::get_num_expanded_until_f_value)
        .def("get_num_deadends_until_f_value", &astar::Statistics::get_num_deadends_until_f_value)
        .def("get_num_pruned_until_f_value", &astar::Statistics::get_num_pruned_until_f_value);

    nb::class_<astar::IEventHandler, IPyAStarEventHandler>(m,
                                                           "IAStarEventHandler")  //
        .def(nb::init<>())
        .def("on_expand_state", &astar::IEventHandler::on_expand_state)
        .def("on_expand_goal_state", &astar::IEventHandler::on_expand_goal_state)
        .def("on_generate_state", &astar::IEventHandler::on_generate_state)
        .def("on_generate_state_relaxed", &astar::IEventHandler::on_generate_state_relaxed)
        .def("on_generate_state_not_relaxed", &astar::IEventHandler::on_generate_state_not_relaxed)
        .def("on_close_state", &astar::IEventHandler::on_close_state)
        .def("on_finish_f_layer", &astar::IEventHandler::on_finish_f_layer)
        .def("on_prune_state", &astar::IEventHandler::on_prune_state)
        .def("on_start_search", &astar::IEventHandler::on_start_search)
        .def("on_end_search", &astar::IEventHandler::on_end_search)
        .def("on_solved", &astar::IEventHandler::on_solved)
        .def("on_unsolvable", &astar::IEventHandler::on_unsolvable)
        .def("on_exhausted", &astar::IEventHandler::on_exhausted)
        .def("get_statistics", &astar::IEventHandler::get_statistics);

    nb::class_<astar::DefaultEventHandlerImpl, astar::IEventHandler>(m,
                                                                     "DefaultAStarEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);
    nb::class_<astar::DebugEventHandlerImpl, astar::IEventHandler>(m,
                                                                   "DebugAStarEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    m.def("find_solution_astar",
          &astar::find_solution,
          "search_context"_a,
          "heuristic"_a,
          "start_state"_a = nullptr,
          "astar_event_handler"_a = nullptr,
          "goal_strategy"_a = nullptr,
          "pruning_strategy"_a = nullptr);

    // BrFS
    nb::class_<brfs::Statistics>(m, "BrFSStatistics")  //
        .def(nb::init<>())
        .def("get_num_generated", &brfs::Statistics::get_num_generated)
        .def("get_num_expanded", &brfs::Statistics::get_num_expanded)
        .def("get_num_deadends", &brfs::Statistics::get_num_deadends)
        .def("get_num_pruned", &brfs::Statistics::get_num_pruned)
        .def("get_num_generated_until_g_value", &brfs::Statistics::get_num_generated_until_g_value)
        .def("get_num_expanded_until_g_value", &brfs::Statistics::get_num_expanded_until_g_value)
        .def("get_num_deadends_until_g_value", &brfs::Statistics::get_num_deadends_until_g_value)
        .def("get_num_pruned_until_g_value", &brfs::Statistics::get_num_pruned_until_g_value);

    nb::class_<brfs::IEventHandler, IPyBrFSEventHandler>(m, "IBrFSEventHandler")  //
        .def(nb::init<>())
        .def("on_expand_state", &brfs::IEventHandler::on_expand_state)
        .def("on_expand_goal_state", &brfs::IEventHandler::on_expand_goal_state)
        .def("on_generate_state", &brfs::IEventHandler::on_generate_state)
        .def("on_generate_state_in_search_tree", &brfs::IEventHandler::on_generate_state_in_search_tree)
        .def("on_generate_state_not_in_search_tree", &brfs::IEventHandler::on_generate_state_not_in_search_tree)
        .def("on_finish_g_layer", &brfs::IEventHandler::on_finish_g_layer)
        .def("on_start_search", &brfs::IEventHandler::on_start_search)
        .def("on_end_search", &brfs::IEventHandler::on_end_search)
        .def("on_solved", &brfs::IEventHandler::on_solved)
        .def("on_unsolvable", &brfs::IEventHandler::on_unsolvable)
        .def("on_exhausted", &brfs::IEventHandler::on_exhausted)
        .def("get_statistics", &brfs::IEventHandler::get_statistics);

    nb::class_<brfs::DefaultEventHandlerImpl, brfs::IEventHandler>(m,
                                                                   "DefaultBrFSEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);
    nb::class_<brfs::DebugEventHandlerImpl, brfs::IEventHandler>(m,
                                                                 "DebugBrFSEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    m.def("find_solution_brfs",
          &brfs::find_solution,
          "search_context"_a,
          "start_state"_a = nullptr,
          "brfs_event_handler"_a = nullptr,
          "goal_strategy"_a = nullptr,
          "pruning_strategy"_a = nullptr,
          "stop_if_goal"_a = true,
          "max_num_states"_a = std::numeric_limits<uint32_t>::max(),
          "max_time_in_ms"_a = std::numeric_limits<uint32_t>::max());

    // GBFS
    nb::class_<gbfs::Statistics>(m, "GBFSStatistics")  //
        .def(nb::init<>())
        .def("get_num_generated", &gbfs::Statistics::get_num_generated)
        .def("get_num_expanded", &gbfs::Statistics::get_num_expanded)
        .def("get_num_deadends", &gbfs::Statistics::get_num_deadends)
        .def("get_num_pruned", &gbfs::Statistics::get_num_pruned);

    nb::class_<gbfs::IEventHandler, IPyGBFSEventHandler>(m, "IGBFSEventHandler")  //
        .def(nb::init<>())
        .def("on_expand_state", &gbfs::IEventHandler::on_expand_state)
        .def("on_expand_goal_state", &gbfs::IEventHandler::on_expand_goal_state)
        .def("on_generate_state", &gbfs::IEventHandler::on_generate_state)
        .def("on_prune_state", &gbfs::IEventHandler::on_prune_state)
        .def("on_start_search", &gbfs::IEventHandler::on_start_search)
        .def("on_end_search", &gbfs::IEventHandler::on_end_search)
        .def("on_solved", &gbfs::IEventHandler::on_solved)
        .def("on_unsolvable", &gbfs::IEventHandler::on_unsolvable)
        .def("on_exhausted", &gbfs::IEventHandler::on_exhausted)
        .def("get_statistics", &gbfs::IEventHandler::get_statistics);

    nb::class_<gbfs::DefaultEventHandlerImpl, gbfs::IEventHandler>(m,
                                                                   "DefaultGBFSEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);
    nb::class_<gbfs::DebugEventHandlerImpl, gbfs::IEventHandler>(m,
                                                                 "DebugGBFSEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    m.def("find_solution_gbfs",
          &gbfs::find_solution,
          "search_context"_a,
          "heuristic"_a,
          "start_state"_a = nullptr,
          "gbfs_event_handler"_a = nullptr,
          "goal_strategy"_a = nullptr,
          "pruning_strategy"_a = nullptr);

    // IW
    nb::class_<iw::TupleIndexMapper>(m, "TupleIndexMapper")  //
        .def(nb::init<size_t, size_t>(), "arity"_a, "num_atoms"_a)
        .def("to_tuple_index", &iw::TupleIndexMapper::to_tuple_index, "atom_indices"_a)
        .def(
            "to_atom_indices",
            [](const iw::TupleIndexMapper& self, const iw::TupleIndex tuple_index)
            {
                auto atom_indices = iw::AtomIndexList {};
                self.to_atom_indices(tuple_index, atom_indices);
                return atom_indices;
            },
            "tuple_index"_a)
        .def("initialize", &iw::TupleIndexMapper::initialize, "arity"_a, "num_atoms"_a)
        .def("tuple_index_to_string", &iw::TupleIndexMapper::tuple_index_to_string, "tuple_index"_a)
        .def("get_num_atoms", &iw::TupleIndexMapper::get_num_atoms)
        .def("get_arity", &iw::TupleIndexMapper::get_arity)
        .def("get_factors", &iw::TupleIndexMapper::get_factors, nb::rv_policy::reference_internal)
        .def("get_max_tuple_index", &iw::TupleIndexMapper::get_max_tuple_index)
        .def("get_empty_tuple_index", &iw::TupleIndexMapper::get_empty_tuple_index);

    nb::class_<iw::StateTupleIndexGenerator>(m, "StateTupleIndexGenerator")  //
        .def(nb::init<const iw::TupleIndexMapper*>(), "tuple_index_mapper"_a)
        .def(
            "__iter__",
            [](iw::StateTupleIndexGenerator& self, State state)
            {
                return nb::make_iterator(nb::type<iw::StateTupleIndexGenerator>(),
                                         "Iterator over atom tuples of size at most the arity as specified in the tuple index mapper.",
                                         self.begin(state),
                                         self.end());
            },
            nb::keep_alive<0, 1>());

    nb::class_<iw::StatePairTupleIndexGenerator>(m, "StatePairTupleIndexGenerator")  //
        .def(nb::init<const iw::TupleIndexMapper*>(), "tuple_index_mapper"_a)
        .def(
            "__iter__",
            [](iw::StatePairTupleIndexGenerator& self, State state, State succ_state)
            {
                return nb::make_iterator(nb::type<iw::StatePairTupleIndexGenerator>(),
                                         "Iterator over atom tuples of size at most the arity as specified in the tuple index mapper.",
                                         self.begin(state, succ_state),
                                         self.end());
            },
            nb::keep_alive<0, 1>());

    nb::class_<iw::Statistics>(m, "IWStatistics")  //
        .def(nb::init<>())
        .def("get_effective_width", &iw::Statistics::get_effective_width)
        .def("get_brfs_statistics_by_arity", &iw::Statistics::get_brfs_statistics_by_arity);

    nb::class_<iw::IEventHandler>(m, "IIWEventHandler")  //
        .def("get_statistics", &iw::IEventHandler::get_statistics);

    nb::class_<iw::DefaultEventHandlerImpl, iw::IEventHandler>(m, "DefaultIWEventHandler").def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    m.def("find_solution_iw",
          &iw::find_solution,
          "search_context"_a,
          "start_state"_a = nullptr,
          "max_arity"_a = nullptr,
          "iw_event_handler"_a = nullptr,
          "brfs_event_handler"_a = nullptr,
          "goal_strategy"_a = nullptr);

    // SIW
    nb::class_<siw::Statistics>(m, "SIWStatistics")  //
        .def(nb::init<>())
        .def("get_maximum_effective_width", &siw::Statistics::get_maximum_effective_width)
        .def("get_average_effective_width", &siw::Statistics::get_average_effective_width)
        .def("get_iw_statistics_by_subproblem", &siw::Statistics::get_iw_statistics_by_subproblem);
    nb::class_<siw::IEventHandler>(m, "ISIWEventHandler")  //
        .def("get_statistics", &siw::IEventHandler::get_statistics);
    nb::class_<siw::DefaultEventHandlerImpl, siw::IEventHandler>(m, "DefaultSIWEventHandler").def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    m.def("find_solution_siw",
          &siw::find_solution,
          "search_context"_a,
          "start_state"_a = nullptr,
          "max_arity"_a = nullptr,
          "siw_event_handler"_a = nullptr,
          "iw_event_handler"_a = nullptr,
          "brfs_event_handler"_a = nullptr,
          "goal_strategy"_a = nullptr);
}

}
