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

    "memory_in_bytes_for_unextended_state_portions",
    "memory_in_bytes_per_unextended_state_portion",
    "memory_in_bytes_for_extended_state_portions",
    "memory_in_bytes_per_extended_state_portion",
    "memory_in_bytes_for_states",
    "memory_in_bytes_per_state",
    "memory_in_bytes_for_nodes",
    "memory_in_bytes_per_node",
    "memory_in_bytes_for_actions",
    "memory_in_bytes_per_action",
    "memory_in_bytes_for_axioms",
    "memory_in_bytes_per_axiom",
    "total_memory_in_bytes",

    "num_of_states",
    "num_of_nodes",
    "num_of_actions",
    "num_of_axioms",

    "cost",
    "length",
    "invalid_plan_reported",
]


exp = Experiment("combined_mimir-ipc5-optimal-strips-astar-blind")

def rename_algorithm(properties):
    """Rename algorithm dynamically during fetching."""
    if properties["algorithm"] == "mimir-grounded-astar-blind":
        properties["algorithm"] = "old-mimir-grounded-astar-blind"
        properties["id"][0] = "old-mimir-grounded-astar-blind"
    if properties["algorithm"] == "mimir-lifted-astar-blind":
        properties["algorithm"] = "old-mimir-lifted-astar-blind"
        properties["id"][0] = "old-mimir-lifted-astar-blind"
    return properties

exp.add_fetcher("old-mimir-ipc5-optimal-strips-astar-blind-eval", filter=rename_algorithm)
exp.add_fetcher("new-mimir-ipc5-optimal-strips-astar-blind-eval")

exp.add_report(BaseReport(attributes=ATTRIBUTES, filter_algorithm=["old-mimir-grounded-astar-blind", "mimir-grounded-astar-blind","old-mimir-lifted-astar-blind", "mimir-lifted-astar-blind"]))

exp.run_steps()
