#! /usr/bin/env python

from lab.parser import Parser
from lab import tools
import re



class SymmetriesParser(Parser):
    """
    [KPKCTrain] Number of objects: 32
    [KPKCTrain] Branching factors: [328, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 192, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 199, 199, 199, 199, 199, 199, 199, 199, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 192, 192, 192, 192, 192, 192, 192, 192, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 192, 192, 192, 192, 192, 192, 192, 192, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 199, 199, 199, 199, 199, 199, 199, 199, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 199, 199, 199, 199, 199, 199, 199, 199, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 199, 199, 199, 199, 199, 199, 199, 199, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 192, 192, 192, 192, 192, 192, 192, 192, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 199, 192, 192, 192, 192, 192, 192, 192, 192, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 199, 199, 199, 199, 199, 199, 199, 199, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 199, 199, 199, 199, 199, 199, 199, 199, 328, 328, 328, 328, 328, 328, 328, 192, 192, 192, 192, 192, 192, 192, 192, 328, 328, 328, 328, 328, 328, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190, 190]
    [KPKCTrain] Branching factor - Average: 207.24
    [KPKCTrain] Branching factor - Variance (population): 666.417
    [KPKCTrain] Branching factor - Stddev  (population): 25.8151
    [KPKCTrain] Branching factor - Variance (sample): 668.284
    [KPKCTrain] Branching factor - Stddev  (sample): 25.8512
    [KPKCTrain] Number of generated successors: 74192
    [KPKCTrain] Number of unique generated ground actions: 368
    [KPKCTrain] Number of unique generated successors: 16768
    [KPKCTrain] Total time: 110ms
    """
    def __init__(self):
        super().__init__()

        self.add_pattern("kpkc_num_objects", r"\[KPKCTrain\] Number of objects: (\d+)", type=int)
        self.add_pattern("kpkc_branching_factor_average", r"\[KPKCTrain\] Branching factor - Average: (.+)", type=float)
        self.add_pattern("kpkc_branching_factor_variance_population", r"\[KPKCTrain\] Branching factor - Variance (population): (.+)", type=float)
        self.add_pattern("kpkc_branching_factor_stddev_population", r"\[KPKCTrain\] Branching factor - Stddev (population): (.+)", type=float)
        self.add_pattern("kpkc_branching_factor_variance_sample", r"\[KPKCTrain\] Branching factor - Variance (sample): (.+)", type=float)
        self.add_pattern("kpkc_branching_factor_stddev_sample", r"\[KPKCTrain\] Branching factor - Stddev (sample): (.+)", type=float)
        self.add_pattern("kpkc_num_generated_successors", r"\[KPKCTrain\] Number of generated successors: (\d+)", type=int)
        self.add_pattern("kpkc_num_unique_generated_ground_actions", r"\[KPKCTrain\] Number of unique generated ground actions: (\d+)", type=int)
        self.add_pattern("kpkc_num_unique_generated_successors", r"\[KPKCTrain\] Number of unique generated successors: (\d+)", type=int)
        self.add_pattern("kpkc_total_time", r"\[KPKCTrain\] Total time: (\d+)ms", type=int)
        

        