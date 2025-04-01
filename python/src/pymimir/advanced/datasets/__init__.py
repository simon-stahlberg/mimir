# Import all classes for better IDE support

from pymimir._pymimir.advanced.datasets import (
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

    # StateSpace
    StateSpace,
    StateSpaceOptions,

    # GeneralizedStateSpace
    GeneralizedStateSpace,
    GeneralizedStateSpaceOptions,

    # TupleGraph
    TupleGraphOptions,

    # KnowledgeBase
    KnowledgeBase,
    KnowledgeBaseOptions,
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