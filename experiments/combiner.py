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
    "invalid_plan_reported",
]


exp = Experiment("combined-brfs-eval")

exp.add_fetcher("brfs-downward-eval")
exp.add_fetcher("brfs-mimir-eval")

exp.add_report(BaseReport(attributes=ATTRIBUTES, filter_algorithm=["downward-astar-blind", "brfs-grounded", "brfs-lifted"]))

exp.run_steps()
