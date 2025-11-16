#! /usr/bin/env python

from lab.experiment import Experiment
from downward.reports.scatter import ScatterPlotReport
from lab.reports import Attribute

exp = Experiment("plot-astar-blind-300-propositional")

exp.add_fetcher("filtered-astar-blind-300-propositional-eval", name="fetch")


exp.add_report(
    ScatterPlotReport(
        attributes=[Attribute("state_peak_memory_usage_in_mib", digits=0)],
        filter_algorithm=["list-lifted-astar-eager-blind", "dtdb-s-lifted-astar-eager-blind"],
        format="tex",  # Use "tex" for pgfplots output.
    ),
    name="scatterplot-state-peak-memory-usage-in-bytes",
)

exp.add_report(
    ScatterPlotReport(
        attributes=[Attribute("total_time_in_sec", digits=0)],
        filter_algorithm=["list-lifted-astar-eager-blind", "dtdb-s-lifted-astar-eager-blind"],
        format="tex",  # Use "tex" for pgfplots output.
    ),
    name="scatterplot-total-time",
)


exp.run_steps()