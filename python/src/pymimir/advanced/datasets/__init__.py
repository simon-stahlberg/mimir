# Import all classes for better IDE support

# StateSpace
from pymimir.pymimir.advanced.datasets import (
    ProblemVertex,
    get_state,
    get_problem,
    get_unit_goal_distance,
    get_action_goal_distance,
    is_initial,
    is_goal,
    is_unsolvable,
    is_alive,
    ProblemEdge,
    get_action,
    get_problem,
    get_action_cost,
    StaticProblemGraph,
    ImmutableStaticProblemGraph,
    ForwardStaticProblemGraph,
    BidirectionalStaticProblemGraph,

    StateSpace,
    StateSpaceOptions,
)

# GeneralizedStateSpace
from pymimir.pymimir.advanced.datasets import (
    ClassVertex,
    get_problem_vertex_index,
    get_problem_index,
    ClassEdge,
    get_problem_edge_index,
    get_problem_index,
    StaticClassGraph,
    ImmutableStaticClassGraph,
    ForwardStaticClassGraph,
    BidirectionalStaticClassGraph,

    GeneralizedStateSpace,
    GeneralizedStateSpaceOptions,
)

# TupleGraph
from pymimir.pymimir.advanced.datasets import (
    TupleGraphVertex,
    get_atom_tuple,
    get_problem_vertices,
    StaticTupleGraph,
    ImmutableStaticTupleGraph,
    ForwardStaticTupleGraph,
    BidirectionalStaticTupleGraph,

    TupleGraphOptions,
    TupleGraph,
)

# KnowledgeBase
from pymimir.pymimir.advanced.datasets import (
    KnowledgeBase,
    KnowledgeBaseOptions,
)

# ObjectGraph 
from pymimir.pymimir.advanced.datasets import (
    create_object_graph
)

__all__ = [
    # StateSpace 
    ProblemVertex,
    ProblemEdge,
    StaticProblemGraph,
    ImmutableStaticProblemGraph,
    ForwardStaticProblemGraph,
    BidirectionalStaticProblemGraph,

    StateSpace,
    StateSpaceOptions,

    # GeneralizedStateSpace
    ClassVertex,
    ClassEdge,
    StaticClassGraph,
    ImmutableStaticClassGraph,
    ForwardStaticClassGraph,
    BidirectionalStaticClassGraph,

    GeneralizedStateSpace,
    GeneralizedStateSpaceOptions,

    # TupleGraph
    TupleGraphVertex,
    StaticTupleGraph,
    ImmutableStaticTupleGraph,
    ForwardStaticTupleGraph,
    BidirectionalStaticTupleGraph,

    TupleGraphOptions,
    TupleGraph,

    # KnowledgeBase
    KnowledgeBase,
    KnowledgeBaseOptions,

    # ObjectGraph
    create_object_graph
]