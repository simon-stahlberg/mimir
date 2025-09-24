#! /usr/bin/env python

from lab.experiment import Experiment
from downward.reports.scatter import ScatterPlotReport


exp = Experiment("plot-astar-blind-300-numeric")

exp.add_fetcher("combined-astar-blind-300-numeric-eval", name="fetch")

exp.add_report(
    ScatterPlotReport(
        attributes=["state_peak_memory_usage_in_bytes"],
        filter_algorithm=["list-lifted-astar-eager-blind", "dtdb-s-lifted-astar-eager-blind"],
        format="tex",  # Use "tex" for pgfplots output.
    ),
    name="scatterplot-state-peak-memory-usage-in-bytes",
)

exp.add_report(
    ScatterPlotReport(
        attributes=["total_time"],
        filter_algorithm=["list-lifted-astar-eager-blind", "dtdb-s-lifted-astar-eager-blind"],
        format="tex",  # Use "tex" for pgfplots output.
    ),
    name="scatterplot-total-time",
)

exp.run_steps()