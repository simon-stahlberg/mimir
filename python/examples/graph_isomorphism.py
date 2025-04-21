""" This example illustrates the interface for graph isomorphism testing of undirected vertex colored graphs 
    using a wrapper around nauty and Traces (https://pallini.di.uniroma1.it/).
"""

import pymimir.advanced.graphs as graphs 

def main():
    graph1 = graphs.StaticVertexColoredGraph()
    v1_1 = graph1.add_vertex(graphs.Color(1, "a"))
    v2_1 = graph1.add_vertex(graphs.Color(2, "b"))
    v3_1 = graph1.add_vertex(graphs.Color(3, "c"))
    graph1.add_undirected_edge(v1_1, v3_1)

    print(graph1)  # print dot representation of graph1
 
    # graph2 is isomorphic to graph1 but it is vertices are added in a different order.
    graph2 = graphs.StaticVertexColoredGraph()
    v1_2 = graph2.add_vertex(graphs.Color(2, "b"))
    v2_2 = graph2.add_vertex(graphs.Color(1, "a"))
    v3_2 = graph2.add_vertex(graphs.Color(3, "c"))
    graph2.add_undirected_edge(v2_2, v3_2)

    print(graph2)  # print dot representation of graph2

    nauty_graph1 = graphs.NautySparseGraph(graph1)
    nauty_graph2 = graphs.NautySparseGraph(graph2)
    
    assert(nauty_graph1 != nauty_graph2)
    
    nauty_graph1.canonize()
    nauty_graph2.canonize()
     
    assert(nauty_graph1 == nauty_graph2)

if __name__ == "__main__":
    main()
