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


exp = Experiment("github/5/ipc-optimal-strips-astar-blind/combined_results")

def remove_nonstrips_domains(properties):
    if properties["domain"] in {
        "data-network-opt18-strips",
        "pathways",
        "quantum-layout-opt23-strips",
        "snake-opt18-strips",
        "termes-opt18-strips",
        "tetris-opt14-strips",
        "tidybot-opt11-strips",
        "spider-opt18-strips",
        "storage",
    }:
        return False
    return True

exp.add_fetcher("github/5/ipc-optimal-strips-astar-blind/downward-ipc-optimal-strips-astar-blind-eval", filter=remove_nonstrips_domains)
exp.add_fetcher("github/5/ipc-optimal-strips-astar-blind/powerlifted-ipc-optimal-strips-astar-blind-eval", filter=remove_nonstrips_domains)
exp.add_fetcher("github/5/ipc-optimal-strips-astar-blind/mimir-sparse-ipc-optimal-strips-astar-blind-eval", filter=remove_nonstrips_domains)

exp.add_report(BaseReport(attributes=ATTRIBUTES, filter_algorithm=["downward-astar-blind", "mimir-grounded-sparse-astar-blind", "powerlifted-astar-blind", "mimir-lifted-sparse-astar-blind"]))

exp.run_steps()
