#! /usr/bin/env python

from lab.parser import Parser

def out_of_memory(content, props):
    props["out_of_memory"] = int("out_of_memory" in props)

def out_of_time(content, props):
    props["out_of_time"] = int("out_of_time_line_detected" in props)


class ErrorParser(Parser):
    """
    Out of memory:
        terminate called after throwing an instance of 'std::bad_alloc'
        what():  std::bad_alloc
        ../../astar_run_planner.sh: line 28: 992534 Aborted                 "$planner_exe" "$domain_file" "$problem_file" "$plan_file" "$flag1" "$flag2" "$flag3"

    Out of time:
        ../../astar_run_planner.sh: line 28: 575061 CPU time limit exceeded "$planner_exe" "$domain_file" "$problem_file" "$plan_file" "$flag1" "$flag2" "$flag3"
    """
    def __init__(self):
        super().__init__()
        self.add_pattern("out_of_memory_line_detected", r".*(std::bad_alloc).*", file="run.err", type=str)
        self.add_pattern("out_of_time_line_detected", r".*(CPU time limit exceeded).*", file="run.err", type=str)

        self.add_function(out_of_memory)
        self.add_function(out_of_time)


