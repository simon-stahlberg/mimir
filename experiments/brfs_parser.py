#! /usr/bin/env python

from lab.parser import Parser


def coverage(content, props):
    props["coverage"] = int("cost" in props)

def unsolvable(content, props):
    props["unsolvable"] = int("exhausted" in props)

def invalid_plan_reported(content, props):
    props["invalid_plan_reported"] = int("val_plan_invalid" in props)

class BrfsParser(Parser):
    """
    Successful Run:
    [AlgorithmStatistics] Search time: 135ms
    [AlgorithmStatistics] Number of generated states: 18119
    [AlgorithmStatistics] Number of expanded states: 1756
    [AlgorithmStatistics] Number of generated states until last f-layer: 17938
    [AlgorithmStatistics] Number of expanded states until last f-layer: 1741
    [Algorithm] Plan found with cost: 7

    Unsolvable Run:
    [Algorithm] Exhausted!

    Validate output:
    Plan executed successfully - checking goal
    Plan valid
    Final value: 11

    Plan executed successfully - checking goal
    Goal not satisfied
    Plan invalid
    """
    def __init__(self):
        super().__init__()
        self.add_pattern("search_time", r"\[AlgorithmStatistics\] Search time: (\d+)ms", type=int)
        self.add_pattern("num_expanded", r"\[AlgorithmStatistics\] Number of expanded states: (\d+)", type=int)
        self.add_pattern("num_generated", r"\[AlgorithmStatistics\] Number of generated states: (\d+)", type=int)
        self.add_pattern("num_expanded_until_last_f_layer", r"\[AlgorithmStatistics\] Number of expanded states until last f-layer: (\d+)", type=int)
        self.add_pattern("num_generated_until_last_f_layer", r"\[AlgorithmStatistics\] Number of generated states until last f-layer: (\d+)", type=int)
        self.add_pattern("cost", r"\[Algorithm\] Plan found with cost: (.+)", type=float)
        self.add_pattern("exhausted", r"(\[Algorithm\] Exhausted!)", type=str)
        self.add_pattern("val_plan_invalid", r"(Plan invalid)", type=str)

        self.add_function(coverage)
        self.add_function(unsolvable)
        self.add_function(invalid_plan_reported)
