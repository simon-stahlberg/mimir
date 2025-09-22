#! /usr/bin/env python

from lab.experiment import Experiment
from downward.reports.absolute import AbsoluteReport
from downward.reports.scatter import ScatterPlotReport

# Create custom report class with suitable info and error attributes.
class BaseReport(AbsoluteReport):
    INFO_ATTRIBUTES = ["time_limit", "memory_limit"]
    ERROR_ATTRIBUTES = [
        "domain",
        "problem",
        "algorithm",
        "unexplained_errors",
        "error",
        "node",
    ]

ATTRIBUTES = [
    "run_dir",
    "coverage",
    "unsolvable",
    "out_of_memory",
    "out_of_time",
    "search_time",
    "total_time",
    "num_generated",
    "num_expanded",
    "num_expanded_until_last_f_layer",
    "num_generated_until_last_f_layer",
    "num_pruned_until_last_f_layer",

    "num_reachable_fluent_atoms",
    "num_reachable_derived_atoms",

    "memory_in_bytes_for_nodes",
    "memory_in_bytes_per_node",
    "memory_in_bytes_for_problem",
    "total_memory_in_bytes",
    "peak_memory_usage_in_bytes",
    "state_peak_memory_usage_in_bytes",

    "score_peak_memory_usage_in_bytes",
    "score_state_peak_memory_usage_in_bytes",

    "num_of_states",
    "num_of_nodes",
    "num_of_actions",
    "num_of_axioms",

    "num_index_slots",
    "num_double_slots",
    "num_slots",
    "average_num_fluent_state_variables",
    "average_num_derived_state_variables",
    "average_num_numeric_state_variables",
    "average_num_state_variables",
    "average_num_index_slots_per_state",
    "average_num_double_slots_per_state",
    "average_num_slots_per_state",

    "initial_h_value",

    "cost",
    "length",
    "invalid_plan_reported",

    "num_generated_valid_action_bindings",
    "num_generated_invalid_action_bindings",
    "num_generated_valid_axiom_bindings",
    "num_generated_invalid_axiom_bindings",
    "num_generated_valid_bindings",
    "num_generated_invalid_bindings",

    "num_generated_valid_action_bindings_until_last_f_layer",
    "num_generated_invalid_action_bindings_until_last_f_layer",
    "num_generated_valid_axiom_bindings_until_last_f_layer",
    "num_generated_invalid_axiom_bindings_until_last_f_layer",
    "num_generated_valid_bindings_until_last_f_layer",
    "num_generated_invalid_bindings_until_last_f_layer",

    "overapproximation_ratio",
    "overapproximation_ratio_until_last_f_layer",

    "num_predicates_by_arity_0",
    "num_predicates_by_arity_1",
    "num_predicates_by_arity_2",
    "num_predicates_by_arity_3",
    "num_predicates_by_arity_4",
    "num_predicates_by_arity_5",
    "num_predicates_by_arity_6",
    "num_predicates_by_arity_7",
    "num_predicates_by_arity_8",

    "num_functions_by_arity_0",
    "num_functions_by_arity_1",
    "num_functions_by_arity_2",
    "num_functions_by_arity_3",
    "num_functions_by_arity_4",
    "num_functions_by_arity_5",
    "num_functions_by_arity_6",
    "num_functions_by_arity_7",
    "num_functions_by_arity_8",

    "num_constraints_by_arity_0",
    "num_constraints_by_arity_1",
    "num_constraints_by_arity_2",
    "num_constraints_by_arity_3",
    "num_constraints_by_arity_4",
    "num_constraints_by_arity_5",
    "num_constraints_by_arity_6",
    "num_constraints_by_arity_7",
    "num_constraints_by_arity_8",
]


exp = Experiment("combined-generator-astar-blind-30-numeric")

def rename_algorithm_kpkc_propositional(properties):
    """Rename algorithm dynamically during fetching."""
    if properties["algorithm"] == "mimir-exhaustive-astar-eager-blind":
        properties["algorithm"] = "exhaustive-propositional-astar-eager-blind"
        properties["id"][0] = "exhaustive-propositional-astar-eager-blind"
    if properties["algorithm"] == "mimir-kpkc-astar-eager-blind":
        properties["algorithm"] = "kpkc-propositional-astar-eager-blind"
        properties["id"][0] = "kpkc-propositional-astar-eager-blind"
    return properties

def rename_algorithm_kpkc_numeric(properties):
    """Rename algorithm dynamically during fetching."""
    if properties["algorithm"] == "mimir-exhaustive-astar-eager-blind":
        properties["algorithm"] = "exhaustive-numeric-astar-eager-blind"
        properties["id"][0] = "exhaustive-numeric-astar-eager-blind"
    if properties["algorithm"] == "mimir-kpkc-astar-eager-blind":
        properties["algorithm"] = "kpkc-numeric-astar-eager-blind"
        properties["id"][0] = "kpkc-numeric-astar-eager-blind"
    return properties

exp.add_fetcher("data-kpkc-numeric/2025-09-19-generator-astar30-ipc-numeric-eval", name="list-fetch-ipc-numeric", filter=rename_algorithm_kpkc_numeric)
exp.add_fetcher("data-kpkc-propositional/2025-09-19-generator-astar30-ipc-numeric-eval", name="dtdb-s-fetch-ipc-numeric", filter=rename_algorithm_kpkc_propositional)

exp.add_fetcher("data-kpkc-numeric/2025-09-19-generator-astar30-minepddl-numeric-eval", name="list-fetch-minepddl-numeric", filter=rename_algorithm_kpkc_numeric)
exp.add_fetcher("data-kpkc-propositional/2025-09-19-generator-astar30-minepddl-numeric-eval", name="dtdb-s-fetch-minepddl-numeric", filter=rename_algorithm_kpkc_propositional)


exp.add_report(BaseReport(attributes=ATTRIBUTES, filter_algorithm=["exhaustive-propositional-astar-eager-blind", "kpkc-propositional-astar-eager-blind", "kpkc-numeric-astar-eager-blind"]))

exp.run_steps()