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
    state_space_options.symmetry_pruning = False 
    generalized_state_space_options = datasets.GeneralizedStateSpaceOptions()
    tuple_graph_options = datasets.TupleGraphOptions()
    tuple_graph_options.width = 1
    knowledge_base_options = datasets.KnowledgeBaseOptions()
    knowledge_base_options.state_space_options = state_space_options
    knowledge_base_options.generalized_state_space_options = generalized_state_space_options
    knowledge_base_options.tuple_graph_options = tuple_graph_options
    knowledge_base = datasets.KnowledgeBase.create(generalized_search_context, knowledge_base_options)

    # 1. Access the StateSpaces.
    for state_space in knowledge_base.get_state_spaces():
        graph = state_space.get_graph()
        print("Dot representation of the StateSpace:")

        # Run basic graph algorithms with switchable edge direction
        edge_weights = [1] * graph.get_num_edges()
        source_indices = [0,1,2]  # some vertices

        graph.compute_strong_components()

        graph.compute_forward_breadth_first_search(source_indices)
        graph.compute_forward_depth_first_search(source_indices)
        graph.compute_forward_floyd_warshall_all_pairs_shortest_paths(edge_weights)
        graph.compute_forward_dijkstra_shortest_paths(edge_weights, source_indices)
        try:
            graph.compute_forward_topological_sort()  # graph is cyclic => this will throw ValueError
        except ValueError:
            pass
        
        graph.compute_backward_breadth_first_search(source_indices)
        graph.compute_backward_depth_first_search(source_indices)
        graph.compute_backward_floyd_warshall_all_pairs_shortest_paths(edge_weights)
        graph.compute_backward_dijkstra_shortest_paths(edge_weights, source_indices)
        try:
            graph.compute_backward_topological_sort()  # graph is cyclic => this will throw ValueError
        except ValueError:
            pass

        # Iterate over vertices
        for vertex in graph.get_vertices():
            # Access vertex properties through generic member
            vertex.get_index()      
            vertex.get_property_0()  # State
            vertex.get_property_1()  # Problem
            vertex.get_property_2()  # Unit goal distance
            vertex.get_property_3()  # Action goal distance
            vertex.get_property_4()  # Is initial?
            vertex.get_property_5()  # Is goal?
            vertex.get_property_6()  # Is unsolvable?
            vertex.get_property_7()  # Is alive?

            # Iterate over adjacent structures
            for adj_vertex in graph.get_forward_adjacent_vertices(vertex.get_index()):
                pass

            for adj_vertex in graph.get_backward_adjacent_vertices(vertex.get_index()):
                pass 

            for adj_edge in graph.get_forward_adjacent_edges(vertex.get_index()):
                pass 

            for adj_edge in graph.get_backward_adjacent_edges(vertex.get_index()):
                pass 

            for adj_v_idx in graph.get_forward_adjacent_vertex_indices(vertex.get_index()):
                pass 

            for adj_v_idx in graph.get_backward_adjacent_vertex_indices(vertex.get_index()):
                pass 

            for adj_e_idx in graph.get_forward_adjacent_edge_indices(vertex.get_index()):
                pass 

            for adj_e_idx in graph.get_backward_adjacent_edge_indices(vertex.get_index()):
                pass
                
        # Iterate over edges
        for edge in graph.get_edges():
            # Access edge properties through generic member
            edge.get_index()  
            edge.get_source()   
            edge.get_target()   
            edge.get_property_0()  # GroundAction
            edge.get_property_1()  # Problem
            edge.get_property_2()  # Action cost
    

    # 2. Access GeneralizedStateSpace
    generalized_state_space = knowledge_base.get_generalized_state_space()

    # Access GeneralizedStateSpace meta data
    generalized_state_space.get_initial_vertices()
    generalized_state_space.get_unsolvable_vertices()
    generalized_state_space.get_goal_vertices()

    # Access GeneralizedStateSpace graph
    graph = generalized_state_space.get_graph()
    print("Dot represention of the GeneralizedStateSpace:")
    print(graph)

    # Run basic graph algorithms with switchable edge direction
    edge_weights = [1] * graph.get_num_edges()
    source_indices = [0,1,2]  # some vertices
    graph.compute_strong_components()

    graph.compute_forward_breadth_first_search(source_indices)
    graph.compute_forward_depth_first_search(source_indices)
    graph.compute_forward_floyd_warshall_all_pairs_shortest_paths(edge_weights)
    graph.compute_forward_dijkstra_shortest_paths(edge_weights, source_indices)
    try:
        graph.compute_forward_topological_sort()  # graph is cyclic => this will throw ValueError
    except ValueError:
        pass
    
    graph.compute_backward_breadth_first_search(source_indices)
    graph.compute_backward_depth_first_search(source_indices)
    graph.compute_backward_floyd_warshall_all_pairs_shortest_paths(edge_weights)
    graph.compute_backward_dijkstra_shortest_paths(edge_weights, source_indices)
    try:
        graph.compute_backward_topological_sort()  # graph is cyclic => this will throw ValueError
    except ValueError:
        pass

    # Iterate over vertices
    for vertex in graph.get_vertices():
        # Access vertex properties through generic member
        vertex.get_index()      
        vertex.get_property_0()  # Problem graph vertex index
        vertex.get_property_1()  # Problem index

        # Iterate over adjacent structures
        for adj_vertex in graph.get_forward_adjacent_vertices(vertex.get_index()):
            pass

        for adj_vertex in graph.get_backward_adjacent_vertices(vertex.get_index()):
            pass 

        for adj_edge in graph.get_forward_adjacent_edges(vertex.get_index()):
            pass 

        for adj_edge in graph.get_backward_adjacent_edges(vertex.get_index()):
            pass 

        for adj_v_idx in graph.get_forward_adjacent_vertex_indices(vertex.get_index()):
            pass 

        for adj_v_idx in graph.get_backward_adjacent_vertex_indices(vertex.get_index()):
            pass 

        for adj_e_idx in graph.get_forward_adjacent_edge_indices(vertex.get_index()):
            pass 

        for adj_e_idx in graph.get_backward_adjacent_edge_indices(vertex.get_index()):
            pass
        
    # Iterate over edges
    for edge in graph.get_edges():
        # Access edge properties through generic member
        edge.get_index()  
        edge.get_source()   
        edge.get_target()   
        edge.get_property_0()  # Problem graph edge index
        edge.get_property_1()  # Problem index

    # Access low-level information about state and actions using the mappings encoded in the GeneralizedStateSpace.
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

    # 3. Access the tuple graphs of each state space.
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
