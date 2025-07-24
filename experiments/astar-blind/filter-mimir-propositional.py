#! /usr/bin/env python

from lab.experiment import Experiment
from lab.reports import Report
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

    "average_num_entities_per_state",
]


exp = Experiment("filtered-astar-blind-30-propositional")

def add_average_num_entities_per_state(run):
    average_num_state_atoms = run.get("average_num_state_atoms")
    average_num_slots_per_state = run.get("average_num_index_slots_per_state")
    algorithm = run.get("algorithm")
    if algorithm in ["list-grounded-astar-eager-blind", "list-grounded-astar-lazy-blind", "list-lifted-astar-eager-blind", "list-lifted-astar-lazy-blind"]:
        run["average_num_entities_per_state"] = average_num_state_atoms
    elif algorithm in ["tree-grounded-astar-eager-blind", "tree-grounded-astar-lazy-blind", "tree-lifted-astar-eager-blind", "tree-lifted-astar-lazy-blind"]:
        run["average_num_entities_per_state"] = average_num_slots_per_state
    return run

exp.add_fetcher("combined-astar-blind-30-propositional-eval", name="fetch")

exp.add_report(BaseReport(attributes=ATTRIBUTES, filter=[add_average_num_entities_per_state]))

exp.run_steps()
