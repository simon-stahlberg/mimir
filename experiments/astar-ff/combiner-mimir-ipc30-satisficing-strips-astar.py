#! /usr/bin/env python

from lab.experiment import Experiment
from downward.reports.absolute import AbsoluteReport

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
]


exp = Experiment("combined-ipc30-satisficing-strips-astar-ff")

def rename_algorithm_lhs(properties):
    """Rename algorithm dynamically during fetching."""
    if properties["algorithm"] == "mimir-grounded-astar-eager-ff":
        properties["algorithm"] = "list-grounded-astar-eager-ff"
        properties["id"][0] = "list-grounded-astar-eager-ff"
    if properties["algorithm"] == "mimir-grounded-astar-lazy-ff":
        properties["algorithm"] = "list-grounded-astar-lazy-ff"
        properties["id"][0] = "list-grounded-astar-lazy-ff"
    if properties["algorithm"] == "mimir-lifted-astar-eager-ff":
        properties["algorithm"] = "list-lifted-astar-eager-ff"
        properties["id"][0] = "list-lifted-astar-eager-ff"
    if properties["algorithm"] == "mimir-lifted-astar-lazy-ff":
        properties["algorithm"] = "list-lifted-astar-lazy-ff"
        properties["id"][0] = "list-lifted-astar-lazy-ff"
    return properties

def rename_algorithm_rhs(properties):
    """Rename algorithm dynamically during fetching."""
    if properties["algorithm"] == "mimir-grounded-astar-eager-ff":
        properties["algorithm"] = "tree-grounded-astar-eager-ff"
        properties["id"][0] = "tree-grounded-astar-eager-ff"
    if properties["algorithm"] == "mimir-grounded-astar-lazy-ff":
        properties["algorithm"] = "tree-grounded-astar-lazy-ff"
        properties["id"][0] = "tree-grounded-astar-lazy-ff"
    if properties["algorithm"] == "mimir-lifted-astar-eager-ff":
        properties["algorithm"] = "tree-lifted-astar-eager-ff"
        properties["id"][0] = "tree-lifted-astar-eager-ff"
    if properties["algorithm"] == "mimir-lifted-astar-lazy-ff":
        properties["algorithm"] = "tree-lifted-astar-lazy-ff"
        properties["id"][0] = "tree-lifted-astar-lazy-ff"
    return properties

exp.add_fetcher("baseline/2025-05-06-astar30-ipc-satisficing-strips-eval", name="list-fetch", filter=rename_algorithm_lhs)
exp.add_fetcher("tree-compression/2025-05-06-astar30-ipc-satisficing-strips-eval", name="tree-fetch", filter=rename_algorithm_rhs)

exp.add_report(BaseReport(attributes=ATTRIBUTES, filter_algorithm=["list-grounded-astar-lazy-ff", "tree-grounded-astar-lazy-ff"]))

exp.run_steps()
