#! /usr/bin/env python

from lab.experiment import Experiment
from downward.reports.scatter import ScatterPlotReport


exp = Experiment("plot-astar-blind-30")

exp.add_fetcher("filtered-astar-blind-30-eval", name="fetch")

def add_average_num_entities_per_state(run):
    average_num_state_atoms = run.get("average_num_state_atoms")
    average_num_slots_per_state = run.get("average_num_slots_per_state")
    algorithm = run.get("algorithm")
    if algorithm in ["list-grounded-astar-eager-blind", "list-grounded-astar-lazy-blind", "list-lifted-astar-eager-blind", "list-lifted-astar-lazy-blind"]:
        run["average_num_entities_per_state"] = average_num_state_atoms
    elif algorithm in ["tree-grounded-astar-eager-blind", "tree-grounded-astar-lazy-blind", "tree-lifted-astar-eager-blind", "tree-lifted-astar-lazy-blind"]:
        run["average_num_entities_per_state"] = average_num_slots_per_state
    return run


exp.add_report(
    ScatterPlotReport(
        attributes=["peak_memory_usage_in_bytes"],
        filter_algorithm=["list-lifted-astar-eager-blind", "tree-lifted-astar-eager-blind"],
        format="tex",  # Use "tex" for pgfplots output.
    ),
    name="scatterplot-peak-memory-usage-in-bytes",
)

exp.add_report(
    ScatterPlotReport(
        attributes=["total_time"],
        filter_algorithm=["list-lifted-astar-eager-blind", "tree-lifted-astar-eager-blind"],
        format="tex",  # Use "tex" for pgfplots output.
    ),
    name="scatterplot-total-time",
)

exp.add_report(
    ScatterPlotReport(
        attributes=["average_num_entities_per_state"],
        filter_algorithm=["list-lifted-astar-eager-blind", "tree-lifted-astar-eager-blind"],
        filter=[add_average_num_entities_per_state],
        format="tex",  # Use "tex" for pgfplots output.
    ),
    name="scatterplot-average-num-entities-per-state",
)

exp.run_steps()
