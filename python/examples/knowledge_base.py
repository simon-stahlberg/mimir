import pymimir as mm

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()

domain_filepath = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
problem_filepath_1 = str(ROOT_DIR / "data" / "gripper" / "p-1-0.pddl")
problem_filepath_2 = str(ROOT_DIR / "data" / "gripper" / "p-2-0.pddl")

search_context_options = mm.SearchContextOptions()
search_context_options.mode = mm.SearchMode.GROUNDED

generalized_search_context = mm.GeneralizedSearchContext(domain_filepath, [problem_filepath_1, problem_filepath_2], search_context_options)

knowledge_base_options = mm.KnowledgeBaseOptions()
knowledge_base_options.state_space_options.problem_options.symmetry_pruning = True

# Create KnowledgeBase
knowledge_base = mm.KnowledgeBase(generalized_search_context, knowledge_base_options)

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
    vertex.get_property_2() # problem vertex index (int)
    vertex.get_property_3() # unit goal distance (int)
    vertex.get_property_4() # action goal distance (double)
    vertex.get_property_5() # is initial ? (bool)
    vertex.get_property_6() # is goal ? (bool)
    vertex.get_property_7() # is unsolvable ? (bool)
    vertex.get_property_8() # is alive (bool)

# Iterate over edges
for edge in graph.get_edges():
    # Access edge properties (Sorry for the generic names!)
    edge.get_index() # graph edge index (int)
    edge.get_source() # graph source index (int)
    edge.get_target() # graph target index (int)
    edge.get_property_0() # class edge index (int)
    edge.get_property_1() # problem index (int)
    edge.get_property_2() # problem edge index (int)
    edge.get_property_3() # action cost

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
    problem_vertex = generalized_state_space.get_problem_vertex(vertex)

    problem_vertex.get_property_0() # class vertex index (int) maps back to the top level graph 
    state = problem_vertex.get_property_1() # state (State)

    problem = generalized_state_space.get_problem(vertex)

    print(state.to_string(problem))

for edge in graph.get_edges():
    problem_edge = generalized_state_space.get_problem_edge(edge)

    problem_edge.get_property_0() # class edge index (int) maps back to the top level graph 
    action = problem_edge.get_property_1() # action (GroundAction)
    problem_edge.get_property_2() # cost (double)

    problem = generalized_state_space.get_problem(edge)

    print(action.to_string(problem))
