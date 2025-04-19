""" WIP: This example illustrates the interface for graph isomorphism of undirected vertex colored graphs using Nauty with Traces.
"""

import pymimir.advanced.graphs as graphs 

class MyColor(graphs.IColor):
    """ MyColor defines some color.
        We could define multiple ones if needed.
    """
    def __init__(self, a: int, b: float):
        super().__init__()
        self._color = (a, b)

    def __eq__(self, other : "MyColor"):
        return self._color == other._color 

    def __lt__(self, other : "MyColor"):
        return self._color < other._color

    def __str__(self):
        return str(self._color)

    def __hash__(self):
        return hash(self._color)

graph1 = graphs.StaticVertexColoredGraph()
graph1.add_vertex(graphs.Color(MyColor(1, 2)))
graph1.add_vertex(graphs.Color(MyColor(2, 1)))

graph2 = graphs.StaticVertexColoredGraph()
graph2.add_vertex(graphs.Color(MyColor(2, 1)))
graph2.add_vertex(graphs.Color(MyColor(1, 2)))

nauty_graph1 = graphs.NautySparseGraph(graph1)
nauty_graph2 = graphs.NautySparseGraph(graph2)

assert(nauty_graph1 != nauty_graph2)

nauty_graph1.canonize()
nauty_graph2.canonize()

assert(nauty_graph1 == nauty_graph2)
