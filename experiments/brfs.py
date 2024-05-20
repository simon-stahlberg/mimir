#! /usr/bin/env python

import platform
import re
import sys

from pathlib import Path

from downward import suites
from downward.reports.absolute import AbsoluteReport
from lab.environments import TetralithEnvironment, LocalEnvironment
from lab.experiment import Experiment
from lab.reports import Attribute, geometric_mean
from brfs_parser import BrfsParser

# Create custom report class with suitable info and error attributes.
class BaseReport(AbsoluteReport):
    INFO_ATTRIBUTES = ["time_limit", "memory_limit"]
    ERROR_ATTRIBUTES = [
        #"domain",
        #"problem",
        #"algorithm",
        "unexplained_errors",
        "error",
        "node",
    ]

DIR = Path(__file__).resolve().parent
REPO = DIR.parent
BENCHMARKS_DIR = REPO / "data"

NODE = platform.node()
REMOTE = re.match(r"tetralith\d+.nsc.liu.se|n\d+", NODE)
if REMOTE:
    ENV = TetralithEnvironment(
        setup=TetralithEnvironment.DEFAULT_SETUP,
        extra_options="#SBATCH --account=naiss2023-5-314")
    SUITE = [
        "blocks_3",
        "delivery",
        "miconic-fulladl",
        "reward",
    ]
    TIME_LIMIT = 60 * 60 * 24 * 1  # 2 days
else:
    ENV = LocalEnvironment(processes=12)
    SUITE = [
        "blocks_3:p213.pddl",
        "delivery:instance_2_1_3_28.pddl",
        "miconic-fulladl:f6-3.pddl",
        "reward:instance_2x2_10.pddl",
    ]
    TIME_LIMIT = 10
ATTRIBUTES = [
    "run_dir",
    "coverage",
    "unsolvable",
    Attribute("num_generated", absolute=True, min_wins=False, scale="linear"),
    Attribute("num_expanded", absolute=True, min_wins=False, scale="linear"),
    Attribute("cost", absolute=True, min_wins=False, scale="linear"),
]

MEMORY_LIMIT = 96000

# Create a new experiment.
exp = Experiment(environment=ENV)
# Add custom parser for FF.
exp.add_parser(BrfsParser())

PLANNER_DIR = REPO / "build" / "exe" / "planner"

exp.add_resource("planner_exr", PLANNER_DIR)

for task in suites.build_suite(BENCHMARKS_DIR, SUITE):
    ################ Grounded ################
    run = exp.add_run()
    run.add_resource("domain", task.domain_file, symlink=True)
    run.add_resource("problem", task.problem_file, symlink=True)
    # 'ff' binary has to be on the PATH.
    # We could also use exp.add_resource().
    run.add_command(
        "run-planner",
        [str(PLANNER_DIR), "{domain}", "{problem}", "1", "0"],
        time_limit=TIME_LIMIT,
        memory_limit=MEMORY_LIMIT,
    )
    # AbsoluteReport needs the following properties:
    # 'domain', 'problem', 'algorithm', 'coverage'.
    run.set_property("domain", task.domain)
    run.set_property("problem", task.problem)
    run.set_property("algorithm", "brfs-grounded")
    # BaseReport needs the following properties:
    # 'time_limit', 'memory_limit'.
    run.set_property("time_limit", TIME_LIMIT)
    run.set_property("memory_limit", MEMORY_LIMIT)
    # Every run has to have a unique id in the form of a list.
    # The algorithm name is only really needed when there are
    # multiple algorithms.
    run.set_property("id", ["brfs-grounded", task.domain, task.problem])

    ################ Lifted ################
    run = exp.add_run()
    run.add_resource("domain", task.domain_file, symlink=True)
    run.add_resource("problem", task.problem_file, symlink=True)
    # 'ff' binary has to be on the PATH.
    # We could also use exp.add_resource().
    run.add_command(
        "run-planner",
        [str(PLANNER_DIR), "{domain}", "{problem}", "0", "0"],
        time_limit=TIME_LIMIT,
        memory_limit=MEMORY_LIMIT,
    )
    # AbsoluteReport needs the following properties:
    # 'domain', 'problem', 'algorithm', 'coverage'.
    run.set_property("domain", task.domain)
    run.set_property("problem", task.problem)
    run.set_property("algorithm", "brfs-lifted")
    # BaseReport needs the following properties:
    # 'time_limit', 'memory_limit'.
    run.set_property("time_limit", TIME_LIMIT)
    run.set_property("memory_limit", MEMORY_LIMIT)
    # Every run has to have a unique id in the form of a list.
    # The algorithm name is only really needed when there are
    # multiple algorithms.
    run.set_property("id", ["brfs-lifted", task.domain, task.problem])

# Add step that writes experiment files to disk.
exp.add_step("build", exp.build)

# Add step that executes all runs.
exp.add_step("start", exp.start_runs)

exp.add_step("parse", exp.parse)

# Add step that collects properties from run directories and
# writes them to *-eval/properties.
exp.add_fetcher(name="fetch")

# Make a report.
exp.add_report(BaseReport(attributes=ATTRIBUTES), outfile="report.html")

# Parse the commandline and run the specified steps.
exp.run_steps()