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

        # The graph is a specific instantiation of static graph which provides access to boost bgl. 
        # For more information on provided functionality, see:
        # https://github.com/simon-stahlberg/mimir/blob/main/python/examples/static_graph.py
        # Since the graph is a specific instantiation, the vertices and edges contain context-specific information.
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

    # The graph is a specific instantiation of static graph which provides access to boost bgl. 
    # For more information on provided functionality, see:
    # https://github.com/simon-stahlberg/mimir/blob/main/python/examples/static_graph.py
    # Since the graph is a specific instantiation, the vertices and edges contain context-specific information.
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

    # The graph also represents an abstraction of a collection of state spaces.
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
