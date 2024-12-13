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


exp = Experiment("ipc-astar-blind-eval")

exp.add_fetcher("ipc-mimir-sparse-astar-blind-eval")
exp.add_fetcher("ipc-mimir-dense-astar-blind-eval")
exp.add_fetcher("ipc-downward-astar-blind-eval")
exp.add_fetcher("ipc-powerlifted-astar-blind-eval")

exp.add_report(BaseReport(attributes=ATTRIBUTES, filter_algorithm=["downward-astar-blind", "mimir-sparse-astar-blind-grounded", "mimir-dense-astar-blind-grounded", "mimir-sparse-astar-blind-lifted", "mimir-dense-astar-blind-lifted"]))

exp.run_steps()
