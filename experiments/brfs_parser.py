#! /usr/bin/env python

from lab.parser import Parser


def coverage(content, props):
    props["coverage"] = int("cost" in props)

class BrfsParser(Parser):
    """
    [AlgorithmStatistics] Number of generated states: 97
    [AlgorithmStatistics] Number of expanded states: 37
    [Algorithm] Plan found with cost: 7
    """
    def __init__(self):
        super().__init__()
        self.add_pattern("num_expanded", r"\[AlgorithmStatistics\] Number of generated states: (\d+)", type=int)
        self.add_pattern("num_generated", r"\[AlgorithmStatistics\] Number of expanded states: (\d+)", type=int)
        self.add_pattern("cost", r"\[Algorithm\] Plan found with cost: (\d+)", type=int)

        self.add_function(coverage)
