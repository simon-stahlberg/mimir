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

    # Create GeneralizedSearchContext
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
        print(graph)

        # The graph is a specific instantiation of static graph which provides access to boost bgl. 
        # For more information on provided functionality, see:
        # https://github.com/simon-stahlberg/mimir/blob/main/python/examples/static_graph.py
        # Since the graph is a specific instantiation, the vertices and edges contain context-specific information.
        # Iterate over vertices
        for vertex in graph.get_vertices():
            # Access vertex properties through get_property_i or specific free function
            vertex.get_index()      
            datasets.get_state(vertex)
            datasets.get_problem(vertex)
            datasets.get_unit_goal_distance(vertex)
            datasets.get_action_goal_distance(vertex)
            datasets.is_initial(vertex)
            datasets.is_goal(vertex)
            datasets.is_unsolvable(vertex)
            datasets.is_alive(vertex)
        # Iterate over edges
        for edge in graph.get_edges():
            # Access edge properties through get_property_i or specific free function
            edge.get_index()  
            edge.get_source()   
            edge.get_target()   
            datasets.get_action(edge)
            datasets.get_problem(edge)
            datasets.get_action_cost(edge)
    

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

    # The graph is a specific instantiation of static graph which provides access to boost bgl. 
    # For more information on provided functionality, see:
    # https://github.com/simon-stahlberg/mimir/blob/main/python/examples/static_graph.py
    # Since the graph is a specific instantiation, the vertices and edges contain context-specific information.
    # Iterate over vertices
    for vertex in graph.get_vertices():
        # Access vertex properties get_property_i or specific free function
        vertex.get_index()      
        datasets.get_problem_vertex_index(vertex)
        datasets.get_problem_index(vertex)
    # Iterate over edges
    for edge in graph.get_edges():
        # Access edge properties get_property_i or specific free function
        edge.get_index()  
        edge.get_source()   
        edge.get_target()   
        datasets.get_problem_edge_index(edge)
        datasets.get_problem_index(edge)

    # The graph also represents an abstraction of a collection of state spaces.
    # Access low-level information about state and actions using the mappings encoded in the GeneralizedStateSpace.
    for vertex in graph.get_vertices():
        # Map class vertex to problem vertex
        problem_vertex = generalized_state_space.get_problem_vertex(vertex)
        # Acces vertex properties get_property_i or specific free function
        problem_vertex.get_index()
        datasets.get_state(problem_vertex)
        datasets.get_problem(problem_vertex)
        datasets.get_unit_goal_distance(problem_vertex)
        datasets.get_action_goal_distance(problem_vertex)
        datasets.is_initial(problem_vertex)
        datasets.is_goal(problem_vertex)
        datasets.is_unsolvable(problem_vertex)
        datasets.is_alive(problem_vertex)
    for edge in graph.get_edges():
        # Map class edge to problem edge
        problem_edge = generalized_state_space.get_problem_edge(edge)
        # Access edge properties get_property_i or specific free function
        problem_edge.get_index()
        problem_edge.get_source()
        problem_edge.get_target()
        datasets.get_action(problem_edge)
        datasets.get_problem(problem_edge)
        datasets.get_action_cost(problem_edge)

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
                datasets.get_atom_tuple(vertex)
                datasets.get_problem_vertices(vertex)


if __name__ == "__main__":
    main()
