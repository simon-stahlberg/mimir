#! /usr/bin/env python

from lab.parser import Parser


def coverage(content, props):
    props["coverage"] = int("cost" in props)

def unsolvable(content, props):
    props["unsolvable"] = int("exhausted" in props)

class BrfsParser(Parser):
    """
    Successful Run:
    [AlgorithmStatistics] Number of generated states: 97
    [AlgorithmStatistics] Number of expanded states: 37
    [Algorithm] Plan found with cost: 7

    Failed Run:
    [Algorithm] Exhausted!
    """
    def __init__(self):
        super().__init__()
        self.add_pattern("num_expanded", r"\[AlgorithmStatistics\] Number of generated states: (\d+)", type=int)
        self.add_pattern("num_generated", r"\[AlgorithmStatistics\] Number of expanded states: (\d+)", type=int)
        self.add_pattern("cost", r"\[Algorithm\] Plan found with cost: (\d+)", type=int)
        self.add_pattern("exhausted", r"(\[Algorithm\] Exhausted!)", type=str)

        self.add_function(coverage)
        self.add_function(unsolvable)
