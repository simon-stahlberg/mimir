# Import all classes for better IDE support


from pymimir.pymimir.advanced.graphs import (
    # Color and graph isomorphism (nauty)
    Color,
    NautySparseGraph,

    # Vertices
    PyVertex,
    EmptyVertex,
    ColoredVertex,

    # Edges
    PyEdge,
    EmptyEdge,
    ColoredEdge,

    # StaticPyGraph
    StaticPyGraph,
    ImmutableStaticPyGraph,
    ForwardStaticPyGraph,
    BidirectionalStaticPyGraph,
    # DynamicPyGraph
    DynamicPyGraph,

    # StaticEmptyGraph
    StaticEmptyGraph,
    ImmutableStaticEmptyGraph,
    ForwardStaticEmptyGraph,
    BidirectionalStaticEmptyGraph,

    # StaticVertexColoredGraph
    StaticVertexColoredGraph,
    ImmutableStaticVertexColoredGraph,
    ForwardStaticVertexColoredGraph,
    BidirectionalStaticVertexColoredGraph,

    # StaticEdgeColoredGraph
    StaticEdgeColoredGraph,
    ImmutableStaticEdgeColoredGraph,
    ForwardStaticEdgeColoredGraph,
    BidirectionalStaticEdgeColoredGraph,
)
