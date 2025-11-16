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

exp = Experiment("filtered-astar-blind-300-propositional")

def total_time_to_sec(properties):
    if "total_time" in properties:
        properties["total_time_in_sec"] = max(1, round(properties["total_time"] / 1000))
    return properties

def state_memory_usage_to_mib(properties):
    if "state_peak_memory_usage_in_bytes" in properties:
        properties["state_peak_memory_usage_in_mib"] = max(1, round(properties["state_peak_memory_usage_in_bytes"] / 1024 / 1024))
    return properties

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

def filter_list(properties):
    total_time_to_sec(properties)
    state_memory_usage_to_mib(properties)
    rename_algorithm_list(properties)
    return properties

def filter_swiss(properties):
    total_time_to_sec(properties)
    state_memory_usage_to_mib(properties)
    rename_algorithm_tree_swiss(properties)
    return properties

def filter_hashid(properties):
    total_time_to_sec(properties)
    state_memory_usage_to_mib(properties)
    rename_algorithm_tree_hashid(properties)
    return properties

exp.add_fetcher("data-list/2025-09-17-astar300-beluga-scalability-deterministic-eval", name="list-fetch-beluga-scalability-deterministic", filter=filter_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-beluga-scalability-deterministic-eval", name="dtdb-s-fetch-beluga-scalability-deterministic", filter=filter_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-beluga-scalability-deterministic-eval", name="dtdb-h-fetch-beluga-scalability-deterministic", filter=filter_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-pushworld-eval", name="list-fetch-pushworld", filter=filter_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-pushworld-eval", name="dtdb-s-fetch-pushworld", filter=filter_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-pushworld-eval", name="dtdb-h-fetch-pushworld", filter=filter_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-minepddl-eval", name="list-fetch-minepddl", filter=filter_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-minepddl-eval", name="dtdb-s-fetch-minepddl", filter=filter_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-minepddl-eval", name="dtdb-h-fetch-minepddl", filter=filter_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-ipc-optimal-strips-eval", name="list-fetch-ipc-optimal-strips", filter=filter_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-ipc-optimal-strips-eval", name="dtdb-s-fetch-ipc-optimal-strips", filter=filter_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-ipc-optimal-strips-eval", name="dtdb-h-fetch-ipc-optimal-strips", filter=filter_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-ipc-optimal-adl-eval", name="list-fetch-ipc-optimal-adl", filter=filter_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-ipc-optimal-adl-eval", name="dtdb-s-fetch-ipc-optimal-adl", filter=filter_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-ipc-optimal-adl-eval", name="dtdb-h-fetch-ipc-optimal-adl", filter=filter_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-ipc-learning-eval", name="list-fetch-ipc-learning", filter=filter_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-ipc-learning-eval", name="dtdb-s-fetch-ipc-learning", filter=filter_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-ipc-learning-eval", name="dtdb-h-fetch-ipc-learning", filter=filter_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-htg-eval", name="list-fetch-htg", filter=filter_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-htg-eval", name="dtdb-s-fetch-htg", filter=filter_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-htg-eval", name="dtdb-h-fetch-htg", filter=filter_hashid)

exp.add_fetcher("data-list/2025-09-17-astar300-cnot-eval", name="list-fetch-cnot", filter=filter_list)
exp.add_fetcher("data-dtdb-s/2025-09-17-astar300-cnot-eval", name="dtdb-s-fetch-cnot", filter=filter_swiss)
exp.add_fetcher("data-dtdb-h/2025-09-17-astar300-cnot-eval", name="dtdb-h-fetch-cnot", filter=filter_hashid)

exp.run_steps()