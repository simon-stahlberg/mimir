# Import all classes for better IDE support

from _pymimir.advanced.datasets import (
    # Vertices
    ProblemVertex,
    ProblemEdge,

    # Edges
    ClassVertex,
    ClassEdge,

    # ProblemStaticGraph
    ProblemStaticGraph,
    ImmutableProblemStaticGraph,
    ForwardProblemStaticGraph,
    BidirectionalProblemStaticGraph,

    # ClassStaticGraph
    ClassStaticGraph,
    ImmutableClassStaticGraph,
    ForwardClassStaticGraph,
    BidirectionalClassStaticGraph,

    # StateSpaces
    GeneralizedStateSpace,
    GeneralizedStateSpaceOptions,
    GeneralizedStateSpaceProblemOptions,

    # KnowledgeBase
    KnowledgeBase,
    KnowledgeBaseOptions,
    TupleGraphCollectionOptions,
)

__all__ = [
    "ProblemVertex",
    "ProblemEdge",
    "ClassVertex",
    "ClassEdge",

    "ProblemStaticGraph",
    "ImmutableProblemStaticGraph",
    "ForwardProblemStaticGraph",
    "BidirectionalProblemStaticGraph",

    "ClassStaticGraph",
    "ImmutableClassStaticGraph",
    "ForwardClassStaticGraph",
    "BidirectionalClassStaticGraph",

    "GeneralizedStateSpace",
    "GeneralizedStateSpaceOptions",
    "GeneralizedStateSpaceProblemOptions",

    "KnowledgeBase",
    "KnowledgeBaseOptions",
    "TupleGraphCollectionOptions",
]