#! /usr/bin/env python

from lab.experiment import Experiment
from downward.reports.absolute import AbsoluteReport
from downward.reports.scatter import ScatterPlotReport
from lab.reports import Attribute, geometric_mean

# Create custom report class with suitable info and error attributes.
class BaseReport(AbsoluteReport):
    INFO_ATTRIBUTES = ["time_limit", "memory_limit"]
    ERROR_ATTRIBUTES = [
        "domain",
        "problem",
        "algorithm",
        "unexplained_errors",
        "error",
        "node",
    ]

ATTRIBUTES = [
    "run_dir",
    "coverage",
    "unsolvable",
    "out_of_memory",
    "out_of_time",
    "search_time",
    Attribute("total_time", function=sum),
    "num_generated",
    "num_expanded",
    "num_expanded_until_last_f_layer",
    "num_generated_until_last_f_layer",
    "num_pruned_until_last_f_layer",

    "num_reachable_fluent_atoms",
    "num_reachable_derived_atoms",

    "memory_in_bytes_for_nodes",
    "memory_in_bytes_per_node",
    "memory_in_bytes_for_problem",
    "total_memory_in_bytes",
    "peak_memory_usage_in_bytes",
    "state_peak_memory_usage_in_bytes",

    "score_peak_memory_usage_in_bytes",
    "score_state_peak_memory_usage_in_bytes",

    "num_of_states",
    "num_of_nodes",
    "num_of_actions",
    "num_of_axioms",

    "num_index_slots",
    "num_double_slots",
    "num_slots",
    "average_num_fluent_state_variables",
    "average_num_derived_state_variables",
    "average_num_numeric_state_variables",
    "average_num_state_variables",
    "average_num_index_slots_per_state",
    "average_num_double_slots_per_state",
    "average_num_slots_per_state",

    "initial_h_value",

    "cost",
    "length",
    "invalid_plan_reported",

    "num_generated_valid_action_base_bindings",
    "num_generated_invalid_action_base_bindings",
    "num_generated_valid_axiom_base_bindings",
    "num_generated_invalid_axiom_base_bindings",
    "num_generated_valid_base_bindings",
    "num_generated_invalid_base_bindings",

    "num_generated_valid_action_base_bindings_until_last_f_layer",
    "num_generated_invalid_action_base_bindings_until_last_f_layer",
    "num_generated_valid_axiom_base_bindings_until_last_f_layer",
    "num_generated_invalid_axiom_base_bindings_until_last_f_layer",
    "num_generated_valid_base_bindings_until_last_f_layer",
    "num_generated_invalid_base_bindings_until_last_f_layer",

    "num_generated_valid_action_derived_bindings",
    "num_generated_invalid_action_derived_bindings",
    "num_generated_valid_axiom_derived_bindings",
    "num_generated_invalid_axiom_derived_bindings",
    "num_generated_valid_derived_bindings",
    "num_generated_invalid_derived_bindings",

    "num_generated_valid_action_derived_bindings_until_last_f_layer",
    "num_generated_invalid_action_derived_bindings_until_last_f_layer",
    "num_generated_valid_axiom_derived_bindings_until_last_f_layer",
    "num_generated_invalid_axiom_derived_bindings_until_last_f_layer",
    "num_generated_valid_derived_bindings_until_last_f_layer",
    "num_generated_invalid_derived_bindings_until_last_f_layer",

    Attribute("overapproximation_ratio", function=geometric_mean),
    Attribute("overapproximation_ratio_until_last_f_layer", function=geometric_mean),

    Attribute("num_predicates_by_arity_greater_or_equal_5", function=max),
    Attribute("num_functions_by_arity_greater_or_equal_5", function=max),
    Attribute("num_constraints_by_arity_greater_or_equal_5", function=max),
    Attribute("num_actions_by_arity_greater_or_equal_5", function=max),
    Attribute("num_axioms_by_arity_greater_or_equal_5", function=max),
    Attribute("num_schemas_by_arity_greater_or_equal_5", function=max)
] \
    + [Attribute(f"num_predicates_by_arity_{i}", function=max) for i in range(5)] \
    + [Attribute(f"num_functions_by_arity_{i}", function=max) for i in range(5)] \
    + [Attribute(f"num_constraints_by_arity_{i}", function=max) for i in range(5)] \
    + [Attribute(f"num_actions_by_arity_{i}", function=max) for i in range(5)] \
    + [Attribute(f"num_axioms_by_arity_{i}", function=max) for i in range(5)] \
    + [Attribute(f"num_schemas_by_arity_{i}", function=max) for i in range(5)]


# exp = Experiment("combined-astar-blind-30")
# exp = Experiment("combined-astar-blind-30-ipc")
exp = Experiment("combined-astar-blind-30-mine")

exp.add_fetcher("data-kpkc-propositional/2025-09-19-grounded-astar30-ipc-numeric-eval", name="fetch-0")
exp.add_fetcher("data-kpkc-propositional/2025-09-19-lifted-exhaustive-astar30-ipc-numeric-eval", name="fetch-1")
exp.add_fetcher("data-kpkc-numeric/2025-09-19-lifted-kpkc-propositional-astar30-ipc-numeric-eval", name="fetch-2")
exp.add_fetcher("data-kpkc-propositional/2025-09-19-lifted-kpkc-numeric-astar30-ipc-numeric-eval", name="fetch-3")

exp.add_fetcher("data-kpkc-propositional/2025-09-19-grounded-astar30-minepddl-numeric-eval", name="fetch-4")
exp.add_fetcher("data-kpkc-propositional/2025-09-19-lifted-exhaustive-astar30-minepddl-numeric-eval", name="fetch-5")
exp.add_fetcher("data-kpkc-numeric/2025-09-19-lifted-kpkc-propositional-astar30-minepddl-numeric-eval", name="fetch-6")
exp.add_fetcher("data-kpkc-propositional/2025-09-19-lifted-kpkc-numeric-astar30-minepddl-numeric-eval", name="fetch-7")


exp.add_report(BaseReport(attributes=ATTRIBUTES, filter_algorithm=["mimir-grounded-astar-eager-blind", "mimir-lifted-exhaustive-astar-eager-blind", "mimir-lifted-kpkc-propositional-astar-eager-blind", "mimir-lifted-kpkc-numeric-astar-eager-blind"]))

exp.run_steps()