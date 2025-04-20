""" This example illustrates the interface of the central component for data generation.
"""

import pymimir.advanced.search as search
import pymimir.advanced.datasets as datasets

from pathlib import Path

ROOT_DIR = (Path(__file__).parent.parent.parent).absolute()


def main():
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
    tuple_graph_options = datasets.TupleGraphOptions()
    tuple_graph_options.width = 1
    knowledge_base_options = datasets.KnowledgeBaseOptions()
    knowledge_base_options.state_space_options = state_space_options
    knowledge_base_options.generalized_state_space_options = generalized_state_space_options
    knowledge_base_options.tuple_graph_options = tuple_graph_options
    knowledge_base = datasets.KnowledgeBase.create(generalized_search_context, knowledge_base_options)

    # Access GeneralizedStateSpace
    generalized_state_space = knowledge_base.get_generalized_state_space()

    # Access GeneralizedStateSpace meta data
    generalized_state_space.get_initial_vertices()
    generalized_state_space.get_unsolvable_vertices()
    generalized_state_space.get_goal_vertices()

    # Access GeneralizedStateSpace graph
    graph = generalized_state_space.get_graph()

    # Iterate over vertices
    for vertex in graph.get_vertices():
        # Access vertex properties through generic member
        vertex.get_index()      
        vertex.get_property_0()  # Problem graph vertex index
        vertex.get_property_1()  # Problem index
        

    # Iterate over edges
    for edge in graph.get_edges():
        # Access edge properties through generic member
        edge.get_index()  
        edge.get_source()   
        edge.get_target()   
        edge.get_property_0()  # Problem graph edge index
        edge.get_property_1()  # Problem index

    ### Iterate over adjacent structures, for the example of some initial state
    v_idx = generalized_state_space.get_initial_vertices().pop()

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

    ### Access low-level information about state and actions using the mappings encoded in the GeneralizedStateSpace.

    for vertex in graph.get_vertices():
        # Map class vertex to problem vertex
        problem_vertex = generalized_state_space.get_problem_vertex(vertex)
        # Acces vertex properties through generic member
        problem_vertex.get_index()
        problem_vertex.get_property_0()  # State
        problem_vertex.get_property_1()  # Problem
        problem_vertex.get_property_2()  # Unit goal distance
        problem_vertex.get_property_3()  # Action goal distance
        problem_vertex.get_property_4()  # Is initial?
        problem_vertex.get_property_5()  # Is goal?
        problem_vertex.get_property_6()  # Is unsolvable?
        problem_vertex.get_property_7()  # Is alive?

    for edge in graph.get_edges():
        # Map class edge to problem edge
        problem_edge = generalized_state_space.get_problem_edge(edge)
        # Access edge properties through generic member
        problem_edge.get_index()
        problem_edge.get_source()
        problem_edge.get_target()
        problem_edge.get_property_0()  # GroundAction
        problem_edge.get_property_1()  # Problem
        problem_edge.get_property_2()  # Action cost

    # Access the tuple graphs of each state space.
    for tuple_graphs in knowledge_base.get_tuple_graphs():
        # Access the tuple graph in the state space.
        for tuple_graph in tuple_graphs:
            tuple_graph.get_graph()
            tuple_graph.get_state_space()
            tuple_graph.get_problem_vertex_indices_grouped_by_distance()
            tuple_graph.get_tuple_vertex_indices_grouped_by_distance()

            for vertex in tuple_graph.get_graph().get_vertices():
                vertex.get_index()
                vertex.get_property_0()  # Atom tuple
                vertex.get_property_1()  # Problem graph vertex indices


if __name__ == "__main__":
    main()
