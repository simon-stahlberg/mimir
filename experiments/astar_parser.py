#! /usr/bin/env python

from lab.parser import Parser


def coverage(content, props):
    props["coverage"] = int("cost" in props)

def unsolvable(content, props):
    props["unsolvable"] = int("exhausted" in props)

def invalid_plan_reported(content, props):
    props["invalid_plan_reported"] = int("val_plan_invalid" in props)

class AStarParser(Parser):
    """
    Successful Run:
    [AStar] Search time: 56ms
    [AStar] Number of generated states: 9543
    [AStar] Number of expanded states: 2000
    [AStar] Number of pruned states: 0
    [AStar] Number of generated states until last f-layer: 5782
    [AStar] Number of expanded states until last f-layer: 1059
    [AStar] Number of pruned states until last f-layer: 0
    [AStar] Plan found.
    [AStar] Plan cost: 6
    [AStar] Plan length: 6

    Unsolvable Run:
    [AStar] Exhausted!

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
        self.add_pattern("search_time", r"\[AStar\] Search time: (\d+)ms", type=int)
        self.add_pattern("num_expanded", r"\[AStar\] Number of expanded states: (\d+)", type=int)
        self.add_pattern("num_generated", r"\[AStar\] Number of generated states: (\d+)", type=int)
        self.add_pattern("num_expanded_until_last_f_layer", r"\[AStar\] Number of expanded states until last f-layer: (\d+)", type=int)
        self.add_pattern("num_generated_until_last_f_layer", r"\[AStar\] Number of generated states until last f-layer: (\d+)", type=int)
        self.add_pattern("num_pruned_until_last_f_layer", r"\[AStar\] Number of pruned states until last f-layer: (\d+)", type=int)
        self.add_pattern("memory_in_mb_used_for_states", r"Memory used for states: (\d+) MB.", type=int)
        self.add_pattern("cost", r"\[AStar\] Plan cost: (.+)", type=float)
        self.add_pattern("length", r"\[AStar\] Plan length: (.+)", type=int)
        self.add_pattern("exhausted", r"(\[AStar\] Exhausted!)", type=str)
        self.add_pattern("val_plan_invalid", r"(Plan invalid)", type=str)

        self.add_function(coverage)
        self.add_function(unsolvable)
        self.add_function(invalid_plan_reported)
