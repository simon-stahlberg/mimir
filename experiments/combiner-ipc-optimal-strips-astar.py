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
    "search_time",
    "num_generated",
    "num_expanded",
    "num_expanded_until_last_f_layer",
    "num_generated_until_last_f_layer",
    "cost",
    "length",
    "invalid_plan_reported",
]


exp = Experiment("github/ipc-optimal-strips-astar-blind/combined_results")

exp.add_fetcher("github/ipc-optimal-strips-astar-blind/downward-ipc-optimal-strips-astar-blind-eval")
exp.add_fetcher("github/ipc-optimal-strips-astar-blind/powerlifted-ipc-optimal-strips-astar-blind-eval")
exp.add_fetcher("github/ipc-optimal-strips-astar-blind/mimir-ipc-optimal-strips-astar-blind-eval")

exp.add_report(BaseReport(attributes=ATTRIBUTES, filter_algorithm=["downward-astar-blind", "powerlifted-astar-blind", "mimir-grounded-sparse-astar-blind","mimir-lifted-sparse-astar-blind"]))

exp.run_steps()
