#! /usr/bin/env python

from lab.parser import Parser
from lab import tools


def coverage(content, props):
    props["coverage"] = int("cost" in props)

def unsolvable(content, props):
    props["unsolvable"] = int("exhausted" in props)

def out_of_memory(content, props):
    props["out_of_memory"] = int("std_bad_alloc" in props)

def out_of_time(content, props):
    props["out_of_time"] = int(props["out_of_memory"] == 0 and props["coverage"] == 0 and props["unsolvable"] == 0)

def invalid_plan_reported(content, props):
    props["invalid_plan_reported"] = int("val_plan_invalid" in props)

def resolve_unexplained_errors(content, props):
    if "unexplained_errors" in props and (props["out_of_memory"] == 1 or props["out_of_time"] == 1):
        del props["unexplained_errors"]

def ensure_minimum_times(content, props):
    for attr in ["search_time", "total_time"]:
        time = props.get(attr, None)
        if time is not None:
            props[attr] = max(time, 1) 

def make_add_score(max_memory_in_bytes: int):
    def add_scores(content, props):
        success = props["coverage"] or props["unsolvable"]

        props["score_peak_memory_usage_in_bytes"] = tools.compute_log_score(
            success, props.get("peak_memory_usage_in_bytes"), lower_bound=2_000_000, upper_bound=max_memory_in_bytes
        )

    return add_scores

def postprocess_initial_h_value(content, props):
    if "initial_h_value" in props:
        props["initial_h_value"] = float(props["initial_h_value"])

        if props["initial_h_value"] == float("inf"):
            props["initial_h_value"] = 4_294_967_295


class SearchParser(Parser):
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
    def __init__(self, max_memory_in_bytes):
        super().__init__()
        self.max_memory_in_bytes = max_memory_in_bytes

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
        self.add_pattern("peak_memory_usage_in_bytes", r"Peak memory usage in bytes: (\d+)", type=int)

        self.add_pattern("std_bad_alloc", r".*(std::bad_alloc).*", type=str, file="run.err")

        self.add_pattern("num_of_states", r"Number of states: (\d+)", type=int)
        self.add_pattern("num_of_nodes", r"Number of nodes: (\d+)", type=int)
        self.add_pattern("num_of_actions", r"Number of actions: (\d+)", type=int)
        self.add_pattern("num_of_axioms", r"Number of axioms: (\d+)", type=int)

        self.add_pattern("num_slots", r"Number of slots: (\d+)", type=int)
        self.add_pattern("average_num_fluent_state_atoms", r"Average number of fluent state atoms: (\d+)", type=float)
        self.add_pattern("average_num_derived_state_atoms", r"Average number of derived state atoms: (\d+)", type=float)
        self.add_pattern("average_num_state_atoms", r"Average number of state atoms: (\d+)", type=float)
        self.add_pattern("average_num_slots_per_state", r"Average number of slots per state: (\d+)", type=float)

        self.add_pattern("initial_h_value", r"Initial h_value: (.+)", type=str)

        self.add_pattern("cost", r"Plan cost: (.+)", type=float)
        self.add_pattern("length", r"Plan length: (.+)", type=int)
        
        self.add_pattern("exhausted", r"(Exhausted!)", type=str)
        self.add_pattern("val_plan_invalid", r"(Plan invalid)", type=str)

        self.add_function(coverage)
        self.add_function(unsolvable)
        self.add_function(out_of_memory)
        self.add_function(out_of_time)
        self.add_function(invalid_plan_reported)

        self.add_function(resolve_unexplained_errors)
        self.add_function(ensure_minimum_times)
        self.add_function(make_add_score(self.max_memory_in_bytes))

        self.add_function(postprocess_initial_h_value)

        
