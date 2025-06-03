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

class IPyExplorationStrategy : public IExplorationStrategy
{
public:
    NB_TRAMPOLINE(IExplorationStrategy, 1);

    /* Trampoline (need one for each virtual function) */
    bool on_generate_state(State state, GroundAction action, State succ_state) override { NB_OVERRIDE_PURE(on_generate_state, state, action, succ_state); }
};

class IPyHeuristic : public IHeuristic
{
public:
    NB_TRAMPOLINE(IHeuristic, 2);

    /* Trampoline (need one for each virtual function) */
    ContinuousCost compute_heuristic(State state, bool is_goal_state) override { NB_OVERRIDE_PURE(compute_heuristic, state, is_goal_state); }

    const PreferredActions& get_preferred_actions() const override { NB_OVERRIDE(get_preferred_actions); }
};

class IPyAStarEagerEventHandler : public astar_eager::IEventHandler
{
public:
    NB_TRAMPOLINE(astar_eager::IEventHandler, 14);

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
    void on_finish_f_layer(ContinuousCost f_value) override { NB_OVERRIDE_PURE(on_finish_f_layer, f_value); }
    void on_prune_state(State state) override { NB_OVERRIDE_PURE(on_prune_state, state); }
    void on_start_search(State start_state, ContinuousCost g_value, ContinuousCost h_value) override
    {
        NB_OVERRIDE_PURE(on_start_search, start_state, g_value, h_value);
    }
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
    const astar_eager::Statistics& get_statistics() const override { NB_OVERRIDE_PURE(get_statistics); }
};

class IPyAStarLazyEventHandler : public astar_lazy::IEventHandler
{
public:
    NB_TRAMPOLINE(astar_lazy::IEventHandler, 14);

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
    void on_finish_f_layer(ContinuousCost f_value) override { NB_OVERRIDE_PURE(on_finish_f_layer, f_value); }
    void on_prune_state(State state) override { NB_OVERRIDE_PURE(on_prune_state, state); }
    void on_start_search(State start_state, ContinuousCost g_value, ContinuousCost h_value) override
    {
        NB_OVERRIDE_PURE(on_start_search, start_state, g_value, h_value);
    }
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
    const astar_lazy::Statistics& get_statistics() const override { NB_OVERRIDE_PURE(get_statistics); }
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

class IPyGBFSEagerEventHandler : public gbfs_eager::IEventHandler
{
public:
    NB_TRAMPOLINE(gbfs_eager::IEventHandler, 11);

    /* Trampoline (need one for each virtual function) */
    void on_expand_state(State state) override { NB_OVERRIDE_PURE(on_expand_state, state); }

    void on_expand_goal_state(State state) override { NB_OVERRIDE_PURE(on_expand_goal_state, state); }

    void on_generate_state(State state, GroundAction action, ContinuousCost action_cost, State successor_state) override
    {
        NB_OVERRIDE_PURE(on_generate_state, state, action, action_cost, successor_state);
    }
    void on_prune_state(State state) override { NB_OVERRIDE_PURE(on_prune_state, state); }
    void on_start_search(State start_state, ContinuousCost g_value, ContinuousCost h_value) override
    {
        NB_OVERRIDE_PURE(on_start_search, start_state, g_value, h_value);
    }
    void on_new_best_h_value(ContinuousCost h_value) override { NB_OVERRIDE_PURE(on_new_best_h_value, h_value); }
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
    const gbfs_eager::Statistics& get_statistics() const override { NB_OVERRIDE_PURE(get_statistics); }
};

class IPyGBFSLazyEventHandler : public gbfs_lazy::IEventHandler
{
public:
    NB_TRAMPOLINE(gbfs_lazy::IEventHandler, 11);

    /* Trampoline (need one for each virtual function) */
    void on_expand_state(State state) override { NB_OVERRIDE_PURE(on_expand_state, state); }

    void on_expand_goal_state(State state) override { NB_OVERRIDE_PURE(on_expand_goal_state, state); }

    void on_generate_state(State state, GroundAction action, ContinuousCost action_cost, State successor_state) override
    {
        NB_OVERRIDE_PURE(on_generate_state, state, action, action_cost, successor_state);
    }
    void on_prune_state(State state) override { NB_OVERRIDE_PURE(on_prune_state, state); }
    void on_start_search(State start_state, ContinuousCost g_value, ContinuousCost h_value) override
    {
        NB_OVERRIDE_PURE(on_start_search, start_state, g_value, h_value);
    }
    void on_new_best_h_value(ContinuousCost h_value) override { NB_OVERRIDE_PURE(on_new_best_h_value, h_value); }
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
    const gbfs_lazy::Statistics& get_statistics() const override { NB_OVERRIDE_PURE(get_statistics); }
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
        .value("GOAL", SearchNodeStatus::GOAL)
        .export_values();

    nb::enum_<SearchStatus>(m, "SearchStatus")
        .value("IN_PROGRESS", SearchStatus::IN_PROGRESS)
        .value("OUT_OF_TIME", SearchStatus::OUT_OF_TIME)
        .value("OUT_OF_MEMORY", SearchStatus::OUT_OF_MEMORY)
        .value("OUT_OF_STATES", SearchStatus::OUT_OF_STATES)
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
            "options"_a)
        .def_static("create", nb::overload_cast<formalism::Problem, const SearchContextImpl::Options&>(&SearchContextImpl::create), "problem"_a, "options"_a)
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
            "options"_a)
        .def_static(
            "create",
            [](const fs::path& domain_filepath, const fs::path& problems_directory, const SearchContextImpl::Options& options) -> GeneralizedSearchContext
            { return GeneralizedSearchContextImpl::create(domain_filepath, problems_directory, options); },
            "domain_filepath"_a,
            "problems_directory"_a,
            "options"_a)
        .def_static("create",
                    nb::overload_cast<formalism::GeneralizedProblem, const SearchContextImpl::Options&>(&GeneralizedSearchContextImpl::create),
                    "generalized_problem"_a,
                    "options"_a)
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
        .def("get_numeric_variables",
             [](const StateImpl& self) { return std::vector<double>(self.get_numeric_variables().begin(), self.get_numeric_variables().end()); })
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
        .def(nb::init<SearchContext, StateList, GroundActionList, ContinuousCost>(), "search_context"_a, "states"_a, "actions"_a, "cost"_a)
        .def("__str__", [](const Plan& self) { return to_string(self); })
        .def("__len__", &Plan::get_length)
        .def("get_search_context", &Plan::get_search_context)
        .def("get_states", &Plan::get_states, nb::rv_policy::copy)
        .def("get_actions", &Plan::get_actions, nb::rv_policy::copy)
        .def("get_cost", &Plan::get_cost);

    /* PartiallyOrderedPlan*/
    nb::class_<PartiallyOrderedPlan>(m, "PartiallyOrderedPlan")  //
        .def(nb::init<Plan>(), "total_ordered_plan"_a)
        .def("__str__", [](const PartiallyOrderedPlan& self) { return to_string(self); })
        .def("compute_totally_ordered_plan_with_maximal_makespan", &PartiallyOrderedPlan::compute_t_o_plan_with_maximal_makespan)
        .def("get_totally_ordered_plan", &PartiallyOrderedPlan::get_t_o_plan, nb::rv_policy::reference_internal)  // Plan is immutable
        .def("get_graph", [](const PartiallyOrderedPlan& self) { return PyImmutable<graphs::DynamicDigraph>(self.get_graph()); });

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
    m.def("is_applicable",
          nb::overload_cast<formalism::GroundAction, const formalism::ProblemImpl&, State>(&search::is_applicable),
          "action"_a,
          "problem"_a,
          "state"_a);

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
        .def(
            "get_or_create_state",
            [](StateRepositoryImpl& self, const GroundAtomList<FluentTag>& fluent_atoms, const ContinuousCostList& numeric_variables)
            { return self.get_or_create_state(fluent_atoms, FlatDoubleList(numeric_variables.begin(), numeric_variables.end())); },
            "fluent_atoms"_a,
            "numeric_variables"_a,
            nb::rv_policy::reference_internal)
        .def("get_or_create_successor_state",
             nb::overload_cast<State, GroundAction, ContinuousCost>(&StateRepositoryImpl::get_or_create_successor_state),
             "state"_a,
             "action"_a,
             "state_metric_value"_a,
             nb::rv_policy::reference_internal)
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
        .def("create_ground_actions", &DeleteRelaxedProblemExplorator::create_ground_actions)
        .def("create_ground_axioms", &DeleteRelaxedProblemExplorator::create_ground_axioms)
        .def("create_grounded_axiom_evaluator",
             &DeleteRelaxedProblemExplorator::create_grounded_axiom_evaluator,
             "match_tree_options"_a,
             "axiom_evaluator_event_handler"_a = nullptr)
        .def("create_grounded_applicable_action_generator",
             &DeleteRelaxedProblemExplorator::create_grounded_applicable_action_generator,
             "match_tree_options"_a,
             "axiom_evaluator_event_handler"_a = nullptr);

    /* Heuristics */
    nb::class_<PreferredActions>(m, "PreferredActions")  //
        .def_rw("data", &PreferredActions::data);

    nb::class_<IHeuristic, IPyHeuristic>(m, "IHeuristic")  //
        .def(nb::init<>())
        .def("compute_heuristic", &IHeuristic::compute_heuristic, "state"_a, "is_goal_state"_a)
        .def("get_preferred_actions", &IHeuristic::get_preferred_actions, nb::rv_policy::reference_internal);

    nb::class_<BlindHeuristicImpl, IHeuristic>(m, "BlindHeuristic")  //
        .def(nb::init<Problem>(), "problem"_a);

    nb::class_<PerfectHeuristicImpl, IHeuristic>(m, "PerfectHeuristic")  //
        .def(nb::init<SearchContext>(), "search_context"_a);

    nb::class_<MaxHeuristicImpl, IHeuristic>(m, "MaxHeuristic")  //
        .def(nb::init<const DeleteRelaxedProblemExplorator&>(), "delete_relaxed_problem_explorator"_a);

    nb::class_<AddHeuristicImpl, IHeuristic>(m, "AddHeuristic")  //
        .def(nb::init<const DeleteRelaxedProblemExplorator&>(), "delete_relaxed_problem_explorator"_a);

    nb::class_<SetAddHeuristicImpl, IHeuristic>(m, "SetAddHeuristic")  //
        .def(nb::init<const DeleteRelaxedProblemExplorator&>(), "delete_relaxed_problem_explorator"_a);

    nb::class_<FFHeuristicImpl, IHeuristic>(m, "FFHeuristic")  //
        .def(nb::init<const DeleteRelaxedProblemExplorator&>(), "delete_relaxed_problem_explorator"_a);

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

    // ExplorationStrategy
    nb::class_<IExplorationStrategy, IPyExplorationStrategy>(m, "IExplorationStrategy")
        .def(nb::init<>())
        .def("on_generate_state", &IExplorationStrategy::on_generate_state, "state"_a, "action"_a, "succ_state"_a);

    // AStar_EAGER
    nb::class_<astar_eager::Statistics>(m, "AStarEagerStatistics")  //
        .def(nb::init<>())
        .def("__str__", [](const astar_eager::Statistics& self) { return to_string(self); })
        .def("get_num_generated", &astar_eager::Statistics::get_num_generated)
        .def("get_num_expanded", &astar_eager::Statistics::get_num_expanded)
        .def("get_num_deadends", &astar_eager::Statistics::get_num_deadends)
        .def("get_num_pruned", &astar_eager::Statistics::get_num_pruned)
        .def("get_num_generated_until_f_value", &astar_eager::Statistics::get_num_generated_until_f_value)
        .def("get_num_expanded_until_f_value", &astar_eager::Statistics::get_num_expanded_until_f_value)
        .def("get_num_deadends_until_f_value", &astar_eager::Statistics::get_num_deadends_until_f_value)
        .def("get_num_pruned_until_f_value", &astar_eager::Statistics::get_num_pruned_until_f_value)
        .def("get_search_time_ms", &astar_eager::Statistics::get_search_time_ms);

    nb::class_<astar_eager::IEventHandler, IPyAStarEagerEventHandler>(m,
                                                                      "IAStarEagerEventHandler")  //
        .def(nb::init<>())
        .def("on_expand_state", &astar_eager::IEventHandler::on_expand_state)
        .def("on_expand_goal_state", &astar_eager::IEventHandler::on_expand_goal_state)
        .def("on_generate_state", &astar_eager::IEventHandler::on_generate_state)
        .def("on_generate_state_relaxed", &astar_eager::IEventHandler::on_generate_state_relaxed)
        .def("on_generate_state_not_relaxed", &astar_eager::IEventHandler::on_generate_state_not_relaxed)
        .def("on_close_state", &astar_eager::IEventHandler::on_close_state)
        .def("on_finish_f_layer", &astar_eager::IEventHandler::on_finish_f_layer)
        .def("on_prune_state", &astar_eager::IEventHandler::on_prune_state)
        .def("on_start_search", &astar_eager::IEventHandler::on_start_search)
        .def("on_end_search", &astar_eager::IEventHandler::on_end_search)
        .def("on_solved", &astar_eager::IEventHandler::on_solved)
        .def("on_unsolvable", &astar_eager::IEventHandler::on_unsolvable)
        .def("on_exhausted", &astar_eager::IEventHandler::on_exhausted)
        .def("get_statistics", &astar_eager::IEventHandler::get_statistics);

    nb::class_<astar_eager::DefaultEventHandlerImpl, astar_eager::IEventHandler>(m,
                                                                                 "DefaultAStarEagerEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);
    nb::class_<astar_eager::DebugEventHandlerImpl, astar_eager::IEventHandler>(m,
                                                                               "DebugAStarEagerEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    nb::class_<astar_eager::Options>(m, "AStarEagerOptions")  //
        .def(nb::init<>())
        .def_rw("start_state", &astar_eager::Options::start_state)
        .def_rw("event_handler", &astar_eager::Options::event_handler)
        .def_rw("goal_strategy", &astar_eager::Options::goal_strategy)
        .def_rw("pruning_strategy", &astar_eager::Options::pruning_strategy)
        .def_rw("max_num_states", &astar_eager::Options::max_num_states)
        .def_rw("max_time_in_ms", &astar_eager::Options::max_time_in_ms);

    m.def("find_solution_astar_eager", &astar_eager::find_solution, "search_context"_a, "heuristic"_a, "options"_a);

    // AStar_LAZY
    nb::class_<astar_lazy::Statistics>(m, "AStarLazyStatistics")  //
        .def(nb::init<>())
        .def("__str__", [](const astar_lazy::Statistics& self) { return to_string(self); })
        .def("get_num_generated", &astar_lazy::Statistics::get_num_generated)
        .def("get_num_expanded", &astar_lazy::Statistics::get_num_expanded)
        .def("get_num_deadends", &astar_lazy::Statistics::get_num_deadends)
        .def("get_num_pruned", &astar_lazy::Statistics::get_num_pruned)
        .def("get_num_generated_until_f_value", &astar_lazy::Statistics::get_num_generated_until_f_value)
        .def("get_num_expanded_until_f_value", &astar_lazy::Statistics::get_num_expanded_until_f_value)
        .def("get_num_deadends_until_f_value", &astar_lazy::Statistics::get_num_deadends_until_f_value)
        .def("get_num_pruned_until_f_value", &astar_lazy::Statistics::get_num_pruned_until_f_value)
        .def("get_search_time_ms", &astar_lazy::Statistics::get_search_time_ms);

    nb::class_<astar_lazy::IEventHandler, IPyAStarLazyEventHandler>(m,
                                                                    "IAStarLazyEventHandler")  //
        .def(nb::init<>())
        .def("on_expand_state", &astar_lazy::IEventHandler::on_expand_state)
        .def("on_expand_goal_state", &astar_lazy::IEventHandler::on_expand_goal_state)
        .def("on_generate_state", &astar_lazy::IEventHandler::on_generate_state)
        .def("on_generate_state_relaxed", &astar_lazy::IEventHandler::on_generate_state_relaxed)
        .def("on_generate_state_not_relaxed", &astar_lazy::IEventHandler::on_generate_state_not_relaxed)
        .def("on_close_state", &astar_lazy::IEventHandler::on_close_state)
        .def("on_finish_f_layer", &astar_lazy::IEventHandler::on_finish_f_layer)
        .def("on_prune_state", &astar_lazy::IEventHandler::on_prune_state)
        .def("on_start_search", &astar_lazy::IEventHandler::on_start_search)
        .def("on_end_search", &astar_lazy::IEventHandler::on_end_search)
        .def("on_solved", &astar_lazy::IEventHandler::on_solved)
        .def("on_unsolvable", &astar_lazy::IEventHandler::on_unsolvable)
        .def("on_exhausted", &astar_lazy::IEventHandler::on_exhausted)
        .def("get_statistics", &astar_lazy::IEventHandler::get_statistics);

    nb::class_<astar_lazy::DefaultEventHandlerImpl, astar_lazy::IEventHandler>(m,
                                                                               "DefaultAStarLazyEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);
    nb::class_<astar_lazy::DebugEventHandlerImpl, astar_lazy::IEventHandler>(m,
                                                                             "DebugAStarLazyEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    nb::class_<astar_lazy::Options>(m, "AStarLazyOptions")  //
        .def(nb::init<>())
        .def_rw("start_state", &astar_lazy::Options::start_state)
        .def_rw("event_handler", &astar_lazy::Options::event_handler)
        .def_rw("goal_strategy", &astar_lazy::Options::goal_strategy)
        .def_rw("pruning_strategy", &astar_lazy::Options::pruning_strategy)
        .def_rw("max_num_states", &astar_lazy::Options::max_num_states)
        .def_rw("max_time_in_ms", &astar_lazy::Options::max_time_in_ms)
        .def_rw("openlist_weights", &astar_lazy::Options::openlist_weights);

    m.def("find_solution_astar_lazy", &astar_lazy::find_solution, "search_context"_a, "heuristic"_a, "options"_a);

    // BrFS
    nb::class_<brfs::Statistics>(m, "BrFSStatistics")  //
        .def(nb::init<>())
        .def("__str__", [](const brfs::Statistics& self) { return to_string(self); })
        .def("get_num_generated", &brfs::Statistics::get_num_generated)
        .def("get_num_expanded", &brfs::Statistics::get_num_expanded)
        .def("get_num_deadends", &brfs::Statistics::get_num_deadends)
        .def("get_num_pruned", &brfs::Statistics::get_num_pruned)
        .def("get_num_generated_until_g_value", &brfs::Statistics::get_num_generated_until_g_value)
        .def("get_num_expanded_until_g_value", &brfs::Statistics::get_num_expanded_until_g_value)
        .def("get_num_deadends_until_g_value", &brfs::Statistics::get_num_deadends_until_g_value)
        .def("get_num_pruned_until_g_value", &brfs::Statistics::get_num_pruned_until_g_value)
        .def("get_search_time_ms", &brfs::Statistics::get_search_time_ms);

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

    nb::class_<brfs::Options>(m, "BrFSOptions")  //
        .def(nb::init<>())
        .def_rw("start_state", &brfs::Options::start_state)
        .def_rw("event_handler", &brfs::Options::event_handler)
        .def_rw("goal_strategy", &brfs::Options::goal_strategy)
        .def_rw("pruning_strategy", &brfs::Options::pruning_strategy)
        .def_rw("stop_if_goal", &brfs::Options::stop_if_goal)
        .def_rw("max_num_states", &brfs::Options::max_num_states)
        .def_rw("max_time_in_ms", &brfs::Options::max_time_in_ms);

    m.def("find_solution_brfs", &brfs::find_solution, "search_context"_a, "options"_a);

    // GBFS_EAGER
    nb::class_<gbfs_eager::Statistics>(m, "GBFSEagerStatistics")  //
        .def(nb::init<>())
        .def("__str__", [](const gbfs_eager::Statistics& self) { return to_string(self); })
        .def("get_num_generated", &gbfs_eager::Statistics::get_num_generated)
        .def("get_num_expanded", &gbfs_eager::Statistics::get_num_expanded)
        .def("get_num_deadends", &gbfs_eager::Statistics::get_num_deadends)
        .def("get_num_pruned", &gbfs_eager::Statistics::get_num_pruned)
        .def("get_search_time_ms", &gbfs_eager::Statistics::get_search_time_ms);

    nb::class_<gbfs_eager::IEventHandler, IPyGBFSEagerEventHandler>(m, "IGBFSEagerEventHandler")  //
        .def(nb::init<>())
        .def("on_expand_state", &gbfs_eager::IEventHandler::on_expand_state)
        .def("on_expand_goal_state", &gbfs_eager::IEventHandler::on_expand_goal_state)
        .def("on_generate_state", &gbfs_eager::IEventHandler::on_generate_state)
        .def("on_prune_state", &gbfs_eager::IEventHandler::on_prune_state)
        .def("on_start_search", &gbfs_eager::IEventHandler::on_start_search)
        .def("on_new_best_h_value", &gbfs_eager::IEventHandler::on_new_best_h_value)
        .def("on_end_search", &gbfs_eager::IEventHandler::on_end_search)
        .def("on_solved", &gbfs_eager::IEventHandler::on_solved)
        .def("on_unsolvable", &gbfs_eager::IEventHandler::on_unsolvable)
        .def("on_exhausted", &gbfs_eager::IEventHandler::on_exhausted)
        .def("get_statistics", &gbfs_eager::IEventHandler::get_statistics);

    nb::class_<gbfs_eager::DefaultEventHandlerImpl, gbfs_eager::IEventHandler>(m,
                                                                               "DefaultGBFSEagerEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);
    nb::class_<gbfs_eager::DebugEventHandlerImpl, gbfs_eager::IEventHandler>(m,
                                                                             "DebugGBFSEagerEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    nb::class_<gbfs_eager::Options>(m, "GBFSEagerOptions")  //
        .def(nb::init<>())
        .def_rw("start_state", &gbfs_eager::Options::start_state)
        .def_rw("event_handler", &gbfs_eager::Options::event_handler)
        .def_rw("goal_strategy", &gbfs_eager::Options::goal_strategy)
        .def_rw("pruning_strategy", &gbfs_eager::Options::pruning_strategy)
        .def_rw("max_num_states", &gbfs_eager::Options::max_num_states)
        .def_rw("max_time_in_ms", &gbfs_eager::Options::max_time_in_ms);

    m.def("find_solution_gbfs_eager", &gbfs_eager::find_solution, "search_context"_a, "heuristic"_a, "options"_a);

    // GBFS_LAZY
    nb::class_<gbfs_lazy::Statistics>(m, "GBFSLazyStatistics")  //
        .def(nb::init<>())
        .def("__str__", [](const gbfs_lazy::Statistics& self) { return to_string(self); })
        .def("get_num_generated", &gbfs_lazy::Statistics::get_num_generated)
        .def("get_num_expanded", &gbfs_lazy::Statistics::get_num_expanded)
        .def("get_num_deadends", &gbfs_lazy::Statistics::get_num_deadends)
        .def("get_num_pruned", &gbfs_lazy::Statistics::get_num_pruned)
        .def("get_search_time_ms", &gbfs_lazy::Statistics::get_search_time_ms);

    nb::class_<gbfs_lazy::IEventHandler, IPyGBFSLazyEventHandler>(m, "IGBFSLazyEventHandler")  //
        .def(nb::init<>())
        .def("on_expand_state", &gbfs_lazy::IEventHandler::on_expand_state)
        .def("on_expand_goal_state", &gbfs_lazy::IEventHandler::on_expand_goal_state)
        .def("on_generate_state", &gbfs_lazy::IEventHandler::on_generate_state)
        .def("on_prune_state", &gbfs_lazy::IEventHandler::on_prune_state)
        .def("on_start_search", &gbfs_lazy::IEventHandler::on_start_search)
        .def("on_new_best_h_value", &gbfs_lazy::IEventHandler::on_new_best_h_value)
        .def("on_end_search", &gbfs_lazy::IEventHandler::on_end_search)
        .def("on_solved", &gbfs_lazy::IEventHandler::on_solved)
        .def("on_unsolvable", &gbfs_lazy::IEventHandler::on_unsolvable)
        .def("on_exhausted", &gbfs_lazy::IEventHandler::on_exhausted)
        .def("get_statistics", &gbfs_lazy::IEventHandler::get_statistics);

    nb::class_<gbfs_lazy::DefaultEventHandlerImpl, gbfs_lazy::IEventHandler>(m,
                                                                             "DefaultGBFSLazyEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);
    nb::class_<gbfs_lazy::DebugEventHandlerImpl, gbfs_lazy::IEventHandler>(m,
                                                                           "DebugGBFSLazyEventHandler")  //
        .def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    nb::class_<gbfs_lazy::Options>(m, "GBFSLazyOptions")  //
        .def(nb::init<>())
        .def_rw("start_state", &gbfs_lazy::Options::start_state)
        .def_rw("event_handler", &gbfs_lazy::Options::event_handler)
        .def_rw("goal_strategy", &gbfs_lazy::Options::goal_strategy)
        .def_rw("pruning_strategy", &gbfs_lazy::Options::pruning_strategy)
        .def_rw("exploration_strategy", &gbfs_lazy::Options::exploration_strategy)
        .def_rw("max_num_states", &gbfs_lazy::Options::max_num_states)
        .def_rw("max_time_in_ms", &gbfs_lazy::Options::max_time_in_ms)
        .def_rw("openlist_weights", &gbfs_lazy::Options::openlist_weights);

    m.def("find_solution_gbfs_lazy", &gbfs_lazy::find_solution, "search_context"_a, "heuristic"_a, "options"_a);

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

    nb::class_<iw::DynamicNoveltyTable>(m, "DynamicNoveltyTable")
        .def(nb::init<size_t>(), "arity"_a)
        .def(nb::init<size_t, size_t>(), "arity"_a, "num_atoms"_a)
        .def(
            "compute_novel_tuples",
            [](iw::DynamicNoveltyTable& self, State state)
            {
                std::vector<iw::AtomIndexList> out;
                self.compute_novel_tuples(state, out);
                return out;
            },
            "state"_a)
        .def("insert_tuples", &iw::DynamicNoveltyTable::insert_tuples, "tuples"_a)
        .def("test_novelty_and_update_table", nb::overload_cast<State>(&iw::DynamicNoveltyTable::test_novelty_and_update_table), "state"_a)
        .def("test_novelty_and_update_table",
             nb::overload_cast<State, State>(&iw::DynamicNoveltyTable::test_novelty_and_update_table),
             "state"_a,
             "succ_state"_a)
        .def("reset", &iw::DynamicNoveltyTable::reset)
        .def("get_tuple_index_mapper", &iw::DynamicNoveltyTable::get_tuple_index_mapper, nb::rv_policy::reference_internal);

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
        .def("__str__", [](const iw::Statistics& self) { return to_string(self); })
        .def("get_effective_width", &iw::Statistics::get_effective_width)
        .def("get_brfs_statistics_by_arity", &iw::Statistics::get_brfs_statistics_by_arity)
        .def("get_search_time_ms", &iw::Statistics::get_search_time_ms);

    nb::class_<iw::IEventHandler>(m, "IIWEventHandler")  //
        .def("get_statistics", &iw::IEventHandler::get_statistics);

    nb::class_<iw::DefaultEventHandlerImpl, iw::IEventHandler>(m, "DefaultIWEventHandler").def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    nb::class_<iw::Options>(m, "IWOptions")  //
        .def(nb::init<>())
        .def_rw("start_state", &iw::Options::start_state)
        .def_rw("iw_event_handler", &iw::Options::iw_event_handler)
        .def_rw("brfs_event_handler", &iw::Options::brfs_event_handler)
        .def_rw("goal_strategy", &iw::Options::goal_strategy)
        .def_rw("max_arity", &iw::Options::max_arity);

    m.def("find_solution_iw", &iw::find_solution, "search_context"_a, "options"_a);

    // SIW
    nb::class_<siw::Statistics>(m, "SIWStatistics")  //
        .def(nb::init<>())
        .def("__str__", [](const siw::Statistics& self) { return to_string(self); })
        .def("get_maximum_effective_width", &siw::Statistics::get_maximum_effective_width)
        .def("get_average_effective_width", &siw::Statistics::get_average_effective_width)
        .def("get_iw_statistics_by_subproblem", &siw::Statistics::get_iw_statistics_by_subproblem)
        .def("get_search_time_ms", &siw::Statistics::get_search_time_ms);

    nb::class_<siw::IEventHandler>(m, "ISIWEventHandler")  //
        .def("get_statistics", &siw::IEventHandler::get_statistics);
    nb::class_<siw::DefaultEventHandlerImpl, siw::IEventHandler>(m, "DefaultSIWEventHandler").def(nb::init<Problem, bool>(), "problem"_a, "quiet"_a = true);

    nb::class_<siw::Options>(m, "SIWOptions")  //
        .def(nb::init<>())
        .def_rw("start_state", &siw::Options::start_state)
        .def_rw("siw_event_handler", &siw::Options::siw_event_handler)
        .def_rw("iw_event_handler", &siw::Options::iw_event_handler)
        .def_rw("brfs_event_handler", &siw::Options::brfs_event_handler)
        .def_rw("goal_strategy", &siw::Options::goal_strategy)
        .def_rw("max_arity", &siw::Options::max_arity);

    m.def("find_solution_siw", &siw::find_solution, "search_context"_a, "options"_a);
}

}
