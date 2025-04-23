""" This example illustrates the interface of a generic static graph taking arbitrary python tuples as vertex and edge property.
    Static graphs dont allow vertex or edge removal allowing for most efficient data representation. 
"""

import pymimir.advanced.graphs as graphs


def main():
    graph = graphs.StaticPyGraph()
    v0 = graph.add_vertex((0,0))
    v1 = graph.add_vertex((0,1))
    v2 = graph.add_vertex((1,1))
    v3 = graph.add_vertex((1,0))
    graph.add_undirected_edge(v0, v1, ("a",1))
    graph.add_undirected_edge(v1, v2, ("b",2))
    graph.add_undirected_edge(v2, v3, ("c",3))
    graph.add_undirected_edge(v3, v0, ("d",4))

    # Print dot representation of the graph.

    print(graph)

    # Check properties of the graph.

    assert(graphs.is_undirected(graph))
    assert(graphs.is_loopless(graph))
    assert(not graphs.is_acyclic(graph))
    assert(not graphs.is_multi(graph))

    # Translate to immutable forward and bidirectional traversal optimized graphs 
    # relevant for algorithms that traverse vertices or edges adjacent to a given vertex.
    forward_graph = graphs.ForwardStaticPyGraph(graph)
    bidirectional_graph = graphs.BidirectionalStaticPyGraph(graph)

    # Run basic graph algorithms with switchable edge direction

    edge_weights = [1] * graph.get_num_edges()
    source_indices = [0,1]

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
        vertex.get_property_0()  # python tuple

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
        edge.get_property_0()  # python tuple



if __name__ == "__main__":
    main()