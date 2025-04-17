#! /usr/bin/env python

from lab.parser import Parser


def coverage(content, props):
    props["coverage"] = int("cost" in props)

def unsolvable(content, props):
    props["unsolvable"] = int("exhausted" in props)

def invalid_plan_reported(content, props):
    props["invalid_plan_reported"] = int("val_plan_invalid" in props)

def resolve_unexplained_errors(content, props):
    print("unexplained_errors" in props, props["out_of_memory"], props["out_of_time"])
    if "unexplained_errors" in props and (props["out_of_memory"] == 1 or props["out_of_time"] == 1):
        del props["unexplained_errors"]

def ensure_minimum_times(content, props):
    for attr in ["search_time", "total_time"]:
        time = props.get(attr, None)
        if time is not None:
            props[attr] = max(time, 1) 

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
        self.add_pattern("search_time", r"Search time: (\d+)ms", type=int)
        self.add_pattern("total_time", r"Total time: (\d+)ms", type=int)
        self.add_pattern("num_expanded", r"Number of expanded states: (\d+)", type=int)
        self.add_pattern("num_generated", r"Number of generated states: (\d+)", type=int)
        self.add_pattern("num_expanded_until_last_f_layer", r"Number of expanded states until last f-layer: (\d+)", type=int)
        self.add_pattern("num_generated_until_last_f_layer", r"Number of generated states until last f-layer: (\d+)", type=int)
        self.add_pattern("num_pruned_until_last_f_layer", r"Number of pruned states until last f-layer: (\d+)", type=int)

        self.add_pattern("num_reachable_fluent_atoms", r"Number of reached fluent atoms: (\d+)", type=int)
        self.add_pattern("num_reachable_derived_atoms", r"Number of reached derived atoms: (\d+)", type=int)

        self.add_pattern("memory_in_bytes_for_nodes", r"Number of bytes for nodes: (\d+)", type=int)
        self.add_pattern("memory_in_bytes_per_node", r"Number of bytes per node: (\d+)", type=int)
        self.add_pattern("memory_in_bytes_for_problem", r"Number of bytes for states: (\d+)", type=int)
        self.add_pattern("total_memory_in_bytes", r"Total number of bytes used: (\d+)", type=int)

        self.add_pattern("num_of_states", r"Number of states: (\d+)", type=int)
        self.add_pattern("num_of_nodes", r"Number of nodes: (\d+)", type=int)
        self.add_pattern("num_of_actions", r"Number of actions: (\d+)", type=int)
        self.add_pattern("num_of_axioms", r"Number of axioms: (\d+)", type=int)

        self.add_pattern("cost", r"Plan cost: (.+)", type=float)
        self.add_pattern("length", r"Plan length: (.+)", type=int)
        
        self.add_pattern("exhausted", r"(Exhausted!)", type=str)
        self.add_pattern("val_plan_invalid", r"(Plan invalid)", type=str)

        self.add_function(coverage)
        self.add_function(unsolvable)
        self.add_function(invalid_plan_reported)

        self.add_function(resolve_unexplained_errors)
        self.add_function(ensure_minimum_times)
