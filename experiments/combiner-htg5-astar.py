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
    "total_time",
    "num_generated",
    "num_expanded",
    "num_expanded_until_last_f_layer",
    "num_generated_until_last_f_layer",
    "cost",
    "length",
    "invalid_plan_reported",
]


exp = Experiment("github/5/htg-astar-blind/combined_results")

def remove_nonstrips_domains(properties):
    if properties["domain"] in {
        "genome-edit-distance-positional",
    }:
        return False
    return True

exp.add_fetcher("github/5/htg-astar-blind/mimir-sparse-htg-astar-blind-eval", filter=remove_nonstrips_domains)
exp.add_fetcher("github/5/htg-astar-blind/downward-htg-astar-blind-eval", filter=remove_nonstrips_domains)
exp.add_fetcher("github/5/htg-astar-blind/powerlifted-htg-astar-blind-eval", filter=remove_nonstrips_domains)

exp.add_report(BaseReport(attributes=ATTRIBUTES, filter_algorithm=["downward-astar-blind", "mimir-grounded-sparse-astar-blind", "mimir-grounded-dense-astar-blind", "powerlifted-astar-blind", "mimir-lifted-sparse-astar-blind", "mimir-lifted-dense-astar-blind"]))

exp.run_steps()
