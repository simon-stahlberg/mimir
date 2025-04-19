# Import all classes for better IDE support


from pymimir.pymimir.advanced.graphs import (
    # Color and graph isomorphism (nauty)
    IColor,
    Color,
    NautySparseGraph,

    # Vertices
    EmptyVertex,
    ColoredVertex,

    # Edges
    EmptyEdge,
    ColoredEdge,

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
