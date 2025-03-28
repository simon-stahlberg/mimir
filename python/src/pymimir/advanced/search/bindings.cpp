#include "../init_declarations.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/trampoline.h>

using namespace mimir;
using namespace mimir::search;
using namespace mimir::formalism;

namespace mimir::bindings
{

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
    NB_TRAMPOLINE(astar::IEventHandler, 13);

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
        NB_OVERRIDE_PURE(on_end_search,
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
    void on_solved(const Plan& plan) override { NB_OVERRIDE_PURE(on_solved, plan); }
    void on_unsolvable() override { NB_OVERRIDE_PURE(on_unsolvable); }
    void on_exhausted() override { NB_OVERRIDE_PURE(on_exhausted); }
    const astar::Statistics& get_statistics() const override { NB_OVERRIDE_PURE(get_statistics); }
};

void bind_search(nb::module_& m)
{
    /* Enums */
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

        .def(nb::init<Problem, SearchContext::Options>(), nb::arg("problem"), nb::arg("options") = SearchContext::Options())

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
        .def("get_fluent_atoms", nb::overload_cast<>(&StateImpl::get_atoms<FluentTag>, nb::const_))
        .def("get_derived_atoms", nb::overload_cast<>(&StateImpl::get_atoms<DerivedTag>, nb::const_))
        .def("literal_holds", nb::overload_cast<GroundLiteral<FluentTag>>(&StateImpl::literal_holds<FluentTag>, nb::const_), nb::arg("literal"))
        .def("literal_holds", nb::overload_cast<GroundLiteral<DerivedTag>>(&StateImpl::literal_holds<DerivedTag>, nb::const_), nb::arg("literal"))
        .def("literals_hold", nb::overload_cast<const GroundLiteralList<FluentTag>&>(&StateImpl::literals_hold<FluentTag>, nb::const_), nb::arg("literals"))
        .def("literals_hold", nb::overload_cast<const GroundLiteralList<DerivedTag>&>(&StateImpl::literals_hold<DerivedTag>, nb::const_), nb::arg("literals"))
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
            nb::arg("state"),
            nb::arg("max_num_groundings"));

    nb::class_<ActionSatisficingBindingGenerator>(m, "ActionSatisficingBindingGenerator")  //
        .def(nb::init<Action, Problem>(), nb::arg("action"), nb::arg("problem"))
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
            nb::arg("state"),
            nb::arg("max_num_groundings"));

    nb::class_<AxiomSatisficingBindingGenerator>(m, "AxiomSatisficingBindingGenerator")  //
        .def(nb::init<Axiom, Problem>(), nb::arg("axiom"), nb::arg("problem"))
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
    nb::class_<LiftedApplicableActionGenerator::IEventHandler>(m,
                                                               "ILiftedApplicableActionGeneratorEventHandler");  //
    nb::class_<LiftedApplicableActionGenerator::DefaultEventHandler, LiftedApplicableActionGenerator::IEventHandler>(
        m,
        "DefaultLiftedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<LiftedApplicableActionGenerator::DebugEventHandler, LiftedApplicableActionGenerator::IEventHandler>(
        m,
        "DebugLiftedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<LiftedApplicableActionGenerator, IApplicableActionGenerator>(m,
                                                                            "LiftedApplicableActionGenerator")  //
        .def(nb::init<Problem>(), nb::arg("problem"))
        .def(nb::init<Problem, std::shared_ptr<LiftedApplicableActionGenerator::IEventHandler>>(), nb::arg("problem"), nb::arg("event_handler"));

    // Grounded
    nb::class_<GroundedApplicableActionGenerator::IEventHandler>(m,
                                                                 "IGroundedApplicableActionGeneratorEventHandler");  //
    nb::class_<GroundedApplicableActionGenerator::DefaultEventHandler, GroundedApplicableActionGenerator::IEventHandler>(
        m,
        "DefaultGroundedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<GroundedApplicableActionGenerator::DebugEventHandler, GroundedApplicableActionGenerator::IEventHandler>(
        m,
        "DebugGroundedApplicableActionGeneratorEventHandler")  //
        .def(nb::init<>());
    nb::class_<GroundedApplicableActionGenerator, IApplicableActionGenerator>(m, "GroundedApplicableActionGenerator");

    /* IAxiomEvaluator */
    nb::class_<IAxiomEvaluator>(m, "IAxiomEvaluator")  //
        .def("get_problem", &IAxiomEvaluator::get_problem);

    // Lifted
    nb::class_<LiftedAxiomEvaluator::IEventHandler>(m, "ILiftedAxiomEvaluatorEventHandler");  //
    nb::class_<LiftedAxiomEvaluator::DefaultEventHandler, LiftedAxiomEvaluator::IEventHandler>(m,
                                                                                               "DefaultLiftedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<LiftedAxiomEvaluator::DebugEventHandler, LiftedAxiomEvaluator::IEventHandler>(m,
                                                                                             "DebugLiftedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<LiftedAxiomEvaluator, IAxiomEvaluator>(m, "LiftedAxiomEvaluator")  //
        .def(nb::init<Problem>(), nb::arg("problem"))
        .def(nb::init<Problem, std::shared_ptr<LiftedAxiomEvaluator::IEventHandler>>(), nb::arg("problem"), nb::arg("event_handler"));

    // Grounded
    nb::class_<GroundedAxiomEvaluator::IEventHandler>(m, "IGroundedAxiomEvaluatorEventHandler");  //
    nb::class_<GroundedAxiomEvaluator::DefaultEventHandler, GroundedAxiomEvaluator::IEventHandler>(m,
                                                                                                   "DefaultGroundedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<GroundedAxiomEvaluator::DebugEventHandler, GroundedAxiomEvaluator::IEventHandler>(m,
                                                                                                 "DebugGroundedAxiomEvaluatorEventHandler")  //
        .def(nb::init<>());
    nb::class_<GroundedAxiomEvaluator, IAxiomEvaluator>(m, "GroundedAxiomEvaluator")  //
        ;

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
             nb::arg("axiom_evaluator_event_handler") = nullptr)
        .def("create_grounded_applicable_action_generator",
             &DeleteRelaxedProblemExplorator::create_grounded_applicable_action_generator,
             nb::arg("match_tree_options") = match_tree::Options(),
             nb::arg("axiom_evaluator_event_handler") = nullptr);

    /* Heuristics */
    nb::class_<IHeuristic, IPyHeuristic>(m, "IHeuristic")  //
        .def(nb::init<>())
        .def("compute_heuristic", &IHeuristic::compute_heuristic);

    nb::class_<BlindHeuristic, IHeuristic>(m, "BlindHeuristic").def(nb::init<Problem>());

    /* Algorithms */

    // SearchResult
    nb::class_<SearchResult>(m, "SearchResult")
        .def(nb::init<>())
        .def_rw("status", &SearchResult::status)
        .def_rw("plan", &SearchResult::plan)
        .def_rw("goal_state", &SearchResult::goal_state);

    // AStar
    nb::class_<astar::Statistics>(m, "AStarStatistics")  //
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

    nb::class_<astar::DefaultEventHandler, astar::IEventHandler>(m,
                                                                 "DefaultAStarEventHandler")  //
        .def(nb::init<Problem, bool>(), nb::arg("problem"), nb::arg("quiet") = true);
    nb::class_<astar::DebugEventHandler, astar::IEventHandler>(m,
                                                               "DebugAStarEventHandler")  //
        .def(nb::init<Problem, bool>(), nb::arg("problem"), nb::arg("quiet") = true);

    m.def("find_solution_astar",
          &astar::find_solution,
          nb::arg("search_context"),
          nb::arg("heuristic"),
          nb::arg("start_state") = nullptr,
          nb::arg("astar_event_handler") = nullptr,
          nb::arg("goal_strategy") = nullptr,
          nb::arg("pruning_strategy") = nullptr);

    // BrFS
    nb::class_<brfs::Statistics>(m, "BrFSStatistics")  //
        .def("get_num_generated", &brfs::Statistics::get_num_generated)
        .def("get_num_expanded", &brfs::Statistics::get_num_expanded)
        .def("get_num_deadends", &brfs::Statistics::get_num_deadends)
        .def("get_num_pruned", &brfs::Statistics::get_num_pruned)
        .def("get_num_generated_until_g_value", &brfs::Statistics::get_num_generated_until_g_value)
        .def("get_num_expanded_until_g_value", &brfs::Statistics::get_num_expanded_until_g_value)
        .def("get_num_deadends_until_g_value", &brfs::Statistics::get_num_deadends_until_g_value)
        .def("get_num_pruned_until_g_value", &brfs::Statistics::get_num_pruned_until_g_value);

    nb::class_<brfs::IEventHandler>(m, "IBrFSEventHandler")  //
        .def("get_statistics", &brfs::IEventHandler::get_statistics);
    nb::class_<brfs::DefaultEventHandler, brfs::IEventHandler>(m,
                                                               "DefaultBrFSEventHandler")  //
        .def(nb::init<Problem, bool>(), nb::arg("problem"), nb::arg("quiet") = true);
    nb::class_<brfs::DebugEventHandler, brfs::IEventHandler>(m,
                                                             "DebugBrFSEventHandler")  //
        .def(nb::init<Problem, bool>(), nb::arg("problem"), nb::arg("quiet") = true);

    m.def("find_solution_brfs",
          &brfs::find_solution,
          nb::arg("search_context"),
          nb::arg("start_state") = nullptr,
          nb::arg("brfs_event_handler") = nullptr,
          nb::arg("goal_strategy") = nullptr,
          nb::arg("pruning_strategy") = nullptr,
          nb::arg("exhaustive") = false);

    // IW
    nb::class_<iw::TupleIndexMapper>(m, "TupleIndexMapper")  //
        .def("to_tuple_index", &iw::TupleIndexMapper::to_tuple_index, nb::arg("atom_indices"))
        .def(
            "to_atom_indices",
            [](const iw::TupleIndexMapper& self, const iw::TupleIndex tuple_index)
            {
                auto atom_indices = iw::AtomIndexList {};
                self.to_atom_indices(tuple_index, atom_indices);
                return atom_indices;
            },
            nb::arg("tuple_index"))
        .def("tuple_index_to_string", &iw::TupleIndexMapper::tuple_index_to_string, nb::arg("tuple_index"))
        .def("get_num_atoms", &iw::TupleIndexMapper::get_num_atoms)
        .def("get_arity", &iw::TupleIndexMapper::get_arity)
        .def("get_factors", &iw::TupleIndexMapper::get_factors, nb::rv_policy::reference_internal)
        .def("get_max_tuple_index", &iw::TupleIndexMapper::get_max_tuple_index)
        .def("get_empty_tuple_index", &iw::TupleIndexMapper::get_empty_tuple_index);

    nb::class_<iw::Statistics>(m, "IWStatistics")  //
        .def("get_effective_width", &iw::Statistics::get_effective_width)
        .def("get_brfs_statistics_by_arity", &iw::Statistics::get_brfs_statistics_by_arity);

    nb::class_<iw::IEventHandler>(m, "IIWEventHandler")  //
        .def("get_statistics", &iw::IEventHandler::get_statistics);

    nb::class_<iw::DefaultEventHandler, iw::IEventHandler>(m, "DefaultIWEventHandler")
        .def(nb::init<Problem, bool>(), nb::arg("problem"), nb::arg("quiet") = true);

    m.def("find_solution_iw",
          &iw::find_solution,
          nb::arg("search_context"),
          nb::arg("start_state") = nullptr,
          nb::arg("max_arity") = nullptr,
          nb::arg("iw_event_handler") = nullptr,
          nb::arg("brfs_event_handler") = nullptr,
          nb::arg("goal_strategy") = nullptr);

    // SIW
    nb::class_<siw::Statistics>(m, "SIWStatistics")  //
        .def("get_maximum_effective_width", &siw::Statistics::get_maximum_effective_width)
        .def("get_average_effective_width", &siw::Statistics::get_average_effective_width)
        .def("get_iw_statistics_by_subproblem", &siw::Statistics::get_iw_statistics_by_subproblem);
    nb::class_<siw::IEventHandler>(m, "ISIWEventHandler")  //
        .def("get_statistics", &siw::IEventHandler::get_statistics);
    nb::class_<siw::DefaultEventHandler, siw::IEventHandler>(m, "DefaultSIWEventHandler")
        .def(nb::init<Problem, bool>(), nb::arg("problem"), nb::arg("quiet") = true);

    m.def("find_solution_siw",
          &siw::find_solution,
          nb::arg("search_context"),
          nb::arg("start_state") = nullptr,
          nb::arg("max_arity") = nullptr,
          nb::arg("siw_event_handler") = nullptr,
          nb::arg("iw_event_handler") = nullptr,
          nb::arg("brfs_event_handler") = nullptr,
          nb::arg("goal_strategy") = nullptr);
}

}
