import pymimir.advanced.search as search
import pymimir.advanced.datasets as datasets

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()

domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
problem_filepath_1 = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")
problem_filepath_2 = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

search_context_options = search.SearchContextOptions()
search_context_options.mode = search.SearchMode.GROUNDED

generalized_search_context = search.GeneralizedSearchContext.create(domain_filepath, [problem_filepath_1, problem_filepath_2], search_context_options)

# Create KnowledgeBase
state_space_options = datasets.StateSpaceOptions()
state_space_options.symmetry_pruning = True 

generalized_state_space_options = datasets.GeneralizedStateSpaceOptions()
generalized_state_space_options.symmetry_pruning = True

knowledge_base_options = datasets.KnowledgeBaseOptions()
knowledge_base_options.state_space_options = state_space_options
knowledge_base_options.generalized_state_space_options = generalized_state_space_options

knowledge_base = datasets.KnowledgeBase.create(generalized_search_context, knowledge_base_options)

# Access GeneralizedStateSpace
generalized_state_space = knowledge_base.get_generalized_state_space()

# Access GeneralizedStateSpace meta data
initial_vertices = generalized_state_space.get_initial_vertices()
unsolvable_vertices = generalized_state_space.get_unsolvable_vertices()
goal_vertices = generalized_state_space.get_goal_vertices()

# Access GeneralizedStateSpace graph
graph = generalized_state_space.get_graph()

# Iterate over vertices
for vertex in graph.get_vertices():
    # Access vertex properties (Sorry for the generic names!)
    vertex.get_index() # graph vertex index (int)
    vertex.get_property_0() # class vertex index (int)
    vertex.get_property_1() # problem index (int)

# Iterate over edges
for edge in graph.get_edges():
    # Access edge properties (Sorry for the generic names!)
    edge.get_index() # graph edge index (int)
    edge.get_source() # graph source index (int)
    edge.get_target() # graph target index (int)
    edge.get_property_0() # class edge index (int)
    edge.get_property_1() # problem index (int)

### Iteration over adjacent structures, for the example of an initial state
v_idx = initial_vertices.pop()

for vertex in graph.get_forward_adjacent_vertices(v_idx):
    pass

for vertex in graph.get_backward_adjacent_vertices(v_idx):
    pass 

for edge in graph.get_forward_adjacent_edges(v_idx):
    pass 

for edge in graph.get_backward_adjacent_edges(v_idx):
    pass 

for v2_idx in graph.get_forward_adjacent_vertex_indices(v_idx):
    pass 

for v2_idx in graph.get_backward_adjacent_vertex_indices(v_idx):
    pass 

for e2_idx in graph.get_forward_adjacent_edge_indices(v_idx):
    pass 

for e2_idx in graph.get_backward_adjacent_edge_indices(v_idx):
    pass

### To access low level information about state and actions, we can use the mappings encoded in the GeneralizedStateSpace

for vertex in graph.get_vertices():
    # Map class vertex to problem vertex
    problem_vertex = generalized_state_space.get_problem_vertex(vertex)

    state = problem_vertex.get_property_0() # state (State)
    problem = problem_vertex.get_property_1() # problem (Problem)
    problem_vertex.get_property_2() # unit goal distance (int)
    problem_vertex.get_property_3() # action goal distance (double)
    problem_vertex.get_property_4() # is initial ? (bool)
    problem_vertex.get_property_5() # is goal ? (bool)
    problem_vertex.get_property_6() # is unsolvable ? (bool)
    problem_vertex.get_property_7() # is alive (bool)

    print(state.to_string(problem))

for edge in graph.get_edges():
    # Map class edge to problem edge
    problem_edge = generalized_state_space.get_problem_edge(edge)

    action = problem_edge.get_property_0() # action (GroundAction)
    problem = problem_edge.get_property_1() # problem (Problem)
    action_cost = problem_edge.get_property_2() # cost (double)

    print(action.to_string(problem))
