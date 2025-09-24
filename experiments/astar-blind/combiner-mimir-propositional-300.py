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
]


exp = Experiment("combined-astar-blind-300-propositional")

def rename_algorithm_list(properties):
    """Rename algorithm dynamically during fetching."""
    if properties["algorithm"] == "mimir-grounded-astar-eager-blind":
        properties["algorithm"] = "list-grounded-astar-eager-blind"
        properties["id"][0] = "list-grounded-astar-eager-blind"
    if properties["algorithm"] == "mimir-grounded-astar-lazy-blind":
        properties["algorithm"] = "list-grounded-astar-lazy-blind"
        properties["id"][0] = "list-grounded-astar-lazy-blind"
    if properties["algorithm"] == "mimir-lifted-astar-eager-blind":
        properties["algorithm"] = "list-lifted-astar-eager-blind"
        properties["id"][0] = "list-lifted-astar-eager-blind"
    if properties["algorithm"] == "mimir-lifted-astar-lazy-blind":
        properties["algorithm"] = "list-lifted-astar-lazy-blind"
        properties["id"][0] = "list-lifted-astar-lazy-blind"
    return properties

def rename_algorithm_tree_swiss(properties):
    """Rename algorithm dynamically during fetching."""
    if properties["algorithm"] == "mimir-grounded-astar-eager-blind":
        properties["algorithm"] = "dtdb-s-grounded-astar-eager-blind"
        properties["id"][0] = "dtdb-s-grounded-astar-eager-blind"
    if properties["algorithm"] == "mimir-grounded-astar-lazy-blind":
        properties["algorithm"] = "dtdb-s-grounded-astar-lazy-blind"
        properties["id"][0] = "dtdb-s-grounded-astar-lazy-blind"
    if properties["algorithm"] == "mimir-lifted-astar-eager-blind":
        properties["algorithm"] = "dtdb-s-lifted-astar-eager-blind"
        properties["id"][0] = "dtdb-s-lifted-astar-eager-blind"
    if properties["algorithm"] == "mimir-lifted-astar-lazy-blind":
        properties["algorithm"] = "dtdb-s-lifted-astar-lazy-blind"
        properties["id"][0] = "dtdb-s-lifted-astar-lazy-blind"
    return properties

def rename_algorithm_tree_hashid(properties):
    """Rename algorithm dynamically during fetching."""
    if properties["algorithm"] == "mimir-grounded-astar-eager-blind":
        properties["algorithm"] = "dtdb-h-grounded-astar-eager-blind"
        properties["id"][0] = "dtdb-h-grounded-astar-eager-blind"
    if properties["algorithm"] == "mimir-grounded-astar-lazy-blind":
        properties["algorithm"] = "dtdb-h-grounded-astar-lazy-blind"
        properties["id"][0] = "dtdb-h-grounded-astar-lazy-blind"
    if properties["algorithm"] == "mimir-lifted-astar-eager-blind":
        properties["algorithm"] = "dtdb-h-lifted-astar-eager-blind"
        properties["id"][0] = "dtdb-h-lifted-astar-eager-blind"
    if properties["algorithm"] == "mimir-lifted-astar-lazy-blind":
        properties["algorithm"] = "dtdb-h-lifted-astar-lazy-blind"
        properties["id"][0] = "dtdb-h-lifted-astar-lazy-blind"
    return properties

exp.add_fetcher("data-list/2025-09-17-astar300-beluga-scalability-deterministic-eval", name="list-fetch-beluga-scalability-deterministic", filter=rename_algorithm_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-beluga-scalability-deterministic-eval", name="dtdb-s-fetch-beluga-scalability-deterministic", filter=rename_algorithm_tree_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-beluga-scalability-deterministic-eval", name="dtdb-h-fetch-beluga-scalability-deterministic", filter=rename_algorithm_tree_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-pushworld-eval", name="list-fetch-pushworld", filter=rename_algorithm_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-pushworld-eval", name="dtdb-s-fetch-pushworld", filter=rename_algorithm_tree_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-pushworld-eval", name="dtdb-h-fetch-pushworld", filter=rename_algorithm_tree_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-minepddl-eval", name="list-fetch-minepddl", filter=rename_algorithm_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-minepddl-eval", name="dtdb-s-fetch-minepddl", filter=rename_algorithm_tree_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-minepddl-eval", name="dtdb-h-fetch-minepddl", filter=rename_algorithm_tree_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-ipc-optimal-strips-eval", name="list-fetch-ipc-optimal-strips", filter=rename_algorithm_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-ipc-optimal-strips-eval", name="dtdb-s-fetch-ipc-optimal-strips", filter=rename_algorithm_tree_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-ipc-optimal-strips-eval", name="dtdb-h-fetch-ipc-optimal-strips", filter=rename_algorithm_tree_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-ipc-optimal-adl-eval", name="list-fetch-ipc-optimal-adl", filter=rename_algorithm_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-ipc-optimal-adl-eval", name="dtdb-s-fetch-ipc-optimal-adl", filter=rename_algorithm_tree_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-ipc-optimal-adl-eval", name="dtdb-h-fetch-ipc-optimal-adl", filter=rename_algorithm_tree_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-htg-eval", name="list-fetch-htg", filter=rename_algorithm_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-htg-eval", name="dtdb-s-fetch-htg", filter=rename_algorithm_tree_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-htg-eval", name="dtdb-h-fetch-htg", filter=rename_algorithm_tree_hashid)

exp.add_report(BaseReport(attributes=ATTRIBUTES, filter_algorithm=["list-grounded-astar-eager-blind", "dtdb-s-grounded-astar-eager-blind", "dtdb-h-grounded-astar-eager-blind", "list-lifted-astar-eager-blind", "dtdb-s-lifted-astar-eager-blind", "dtdb-h-lifted-astar-eager-blind"]))

exp.run_steps()