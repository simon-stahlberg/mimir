#! /usr/bin/env python

from lab.parser import Parser
from lab import tools
import re



class SymmetriesParser(Parser):
    """
    [KPKCTrain] Number of objects: 121
    [KPKCTrain] Branching factor: 220
    [KPKCTrain] Preprocessing time: 0ms
    """
    def __init__(self):
        super().__init__()

        self.add_pattern("kpkc_num_objects", r"\[KPKCTrain\] Number of objects: (\d+)", type=int)
        self.add_pattern("kpkc_branching_factor", r"\[KPKCTrain\] Branching factor: (.+)", type=float)
        self.add_pattern("kpkc_total_time", r"\[KPKCTrain\] Preprocessing time: (\d+)ms", type=int)
        

        