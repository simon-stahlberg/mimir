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
    [BrFS] Search time: 50ms
    [BrFS] Number of generated states: 8147
    [BrFS] Number of expanded states: 1711
    [BrFS] Number of pruned states: 5336
    [BrFS] Number of generated states until last f-layer: 5782
    [BrFS] Number of expanded states until last f-layer: 1059
    [BrFS] Number of pruned states until last f-layer: 3750
    [BrFS] Plan found.
    [BrFS] Plan cost: 6
    [BrFS] Plan length: 6

    Unsolvable Run:
    [BrFS] Exhausted!

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
        self.add_pattern("search_time", r"\[BrFS\] Search time: (\d+)ms", type=int)
        self.add_pattern("num_expanded", r"\[BrFS\] Number of expanded states: (\d+)", type=int)
        self.add_pattern("num_generated", r"\[BrFS\] Number of generated states: (\d+)", type=int)
        self.add_pattern("num_expanded_until_last_g_layer", r"\[BrFS\] Number of expanded states until last g-layer: (\d+)", type=int)
        self.add_pattern("num_generated_until_last_g_layer", r"\[BrFS\] Number of generated states until last g-layer: (\d+)", type=int)
        self.add_pattern("num_pruned_until_last_g_layer", r"\[BrFS\] Number of pruned states until last g-layer: (\d+)", type=int)
        self.add_pattern("cost", r"\[BrFS\] Plan cost: (.+)", type=float)
        self.add_pattern("length", r"\[BrFS\] Plan length: (.+)", type=float)
        self.add_pattern("exhausted", r"(\[BrFS\] Exhausted!)", type=str)
        self.add_pattern("val_plan_invalid", r"(Plan invalid)", type=str)

        self.add_function(coverage)
        self.add_function(unsolvable)
        self.add_function(invalid_plan_reported)
