import pymimir.advanced.formalism as formalism
import pymimir.advanced.search as search

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()

domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
problem_filepath = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

search_context = search.SearchContext.create(domain_filepath, problem_filepath)

initial_state, initial_metric_value = search_context.get_state_repository().get_or_create_initial_state()

#######################################
# Utility
#######################################

## TupleIndexGenerators

# Generate all tuples of atoms of size at most 4 in the initial state
arity = 4
max_num_atoms = 100  # max_num_atoms must be sufficiently large
tuple_index_mapper = search.TupleIndexMapper(arity, max_num_atoms) 
tuple_index_mapper.initialize(arity, max_num_atoms)  # Call initialize to resize dynamically if necessary

tuple_index_generator = search.StateTupleIndexGenerator(tuple_index_mapper)
for tuple_index in tuple_index_generator.__iter__(initial_state):
    print(tuple_index_mapper.to_atom_indices(tuple_index))

## PruningStategy

arity_k_pruning_stategy = search.ArityKNoveltyPruningStrategy.create(arity, max_num_atoms)

#######################################
# Search
#######################################

class CustomBrFSEventHandler(search.IBrFSEventHandler):
    """A custom event handler to react on events during iw search.
    
    It is useful to collect custom statistics or create a search tree.
    """
    def __init__(self, problem: formalism.Problem):
        super().__init__()

        self.problem = problem
        self.statistics = search.BrFSStatistics()

    def on_expand_state(self, state : search.State):
        pass

    def on_expand_goal_state(self, state : search.State):
        pass

    def on_generate_state(self, state : search.State, action : formalism.GroundAction, action_cost: float, successor_state: search.State):
        pass

    def on_generate_state_in_search_tree(self, state : search.State, action : formalism.GroundAction, action_cost: float, successor_state: search.State):
        print("Generated state:", successor_state.to_string(self.problem))

    def on_generate_state_not_in_search_tree(self, state : search.State, action : formalism.GroundAction, action_cost: float, successor_state: search.State):
        pass

    def on_finish_g_layer(self):
        pass

    def on_start_search(self, start_state : search.State):
        pass

    def on_end_search(self, num_reached_fluent_atoms : int, num_reached_derived_atoms: int, num_bytes_for_unextended_state_portion: int, num_bytes_for_extended_state_portion: int, num_bytes_for_nodes: int, num_bytes_for_actions: int, num_bytes_for_axioms: int, num_states: int, num_nodes: int, num_actions: int, num_axioms: int):
        pass

    def on_solved(self, plan: search.Plan):
        pass

    def on_unsolvable(self):
        pass

    def on_exhausted(self):
        pass

    def get_statistics(self):
        return self.statistics


iw_event_handler = search.DefaultIWEventHandler(search_context.get_problem())
brfs_event_handler = CustomBrFSEventHandler(search_context.get_problem())
goal_strategy = search.ProblemGoalStrategy.create(search_context.get_problem())

result = search.find_solution_iw(search_context, initial_state, 3, iw_event_handler, brfs_event_handler, goal_strategy)

assert(result.status == search.SearchStatus.SOLVED)
assert(len(result.plan) == 5)
