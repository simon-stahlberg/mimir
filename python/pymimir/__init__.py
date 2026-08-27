"""Mimir Python bindings.

This package exposes a high-level Python API for the non-numerical fragment of
PDDL.

Load models with :class:`Problem.from_files`, construct prototype values through
problem-bound factories, and search with :func:`bfs`, :func:`ucs`,
:func:`astar`, :func:`gbfs`, or :func:`iw`. Low-level ctypes signatures under
:mod:`pymimir.advanced` are explicitly unstable.
"""

from . import builders, heuristics, learning, model, search, state_space
from .builders import (
    ActionCost,
    ActionCostSpec,
    ActionListBuilder,
    ActionSchemaBuilder,
    ConditionalEffectBuilder,
    ConstantListBuilder,
    DerivedPredicateListBuilder,
    DomainBuilder,
    GoalBuilder,
    InitialStateBuilder,
    Logic,
    LogicalExpressionSpec,
    NumericFunctionListBuilder,
    PredicateListBuilder,
    ProblemBuilder,
    ProblemObjectListBuilder,
    RequirementListBuilder,
    TypeListBuilder,
)
from .model import (
    Action,
    ActionGenerator,
    Atom,
    ConditionalEffect,
    ConjunctiveCondition,
    Domain,
    Effect,
    GroundAction,
    GroundAtom,
    GroundConditionalEffect,
    GroundConjunctiveCondition,
    GroundEffect,
    GroundLiteral,
    Literal,
    Object,
    Predicate,
    PredicateType,
    Problem,
    State,
    Term,
    Variable,
)
from .search import (
    Solution,
    SearchResult,
    SearchStatistics,
    SearchStatus,
    Transition,
)
from .search import astar, bfs, gbfs, iw, ucs
from .heuristics import (
    AddHeuristic,
    BlindHeuristic,
    FFHeuristic,
    GoalCountHeuristic,
    H2Heuristic,
    LiftedFFHeuristic,
    MaxHeuristic,
    PerfectHeuristic,
    SetAddHeuristic,
)
from .state_space import StateLabel, StateSpace
from .errors import MimirError, PddlError, StateSpaceLimitExceeded
from .learning import (
    EncodingContext,
    RelationBuffer,
    RelationDescriptor,
    encode_action_list,
    encode_expressive_goal,
    encode_expressive_state,
    encode_goal,
    encode_state,
    encode_transition_effects,
    encode_virtual_node,
)

__version__: str = "0.14.0b3"

__all__ = [
    # Model
    "model", "Variable", "Object", "Term", "Predicate", "Atom", "GroundAtom",
    "Literal", "GroundLiteral", "Effect", "ConditionalEffect", "GroundEffect",
    "GroundConditionalEffect", "Action", "GroundAction", "Domain", "Problem", "State",
    "ConjunctiveCondition", "GroundConjunctiveCondition", "PredicateType", "ActionGenerator",
    # Builders
    "builders", "DomainBuilder", "RequirementListBuilder", "TypeListBuilder",
    "ConstantListBuilder", "PredicateListBuilder", "NumericFunctionListBuilder",
    "ActionListBuilder", "ActionSchemaBuilder", "ConditionalEffectBuilder",
    "DerivedPredicateListBuilder", "ProblemBuilder", "ProblemObjectListBuilder",
    "InitialStateBuilder", "GoalBuilder", "Logic", "LogicalExpressionSpec",
    "ActionCost", "ActionCostSpec",
    # Search
    "search", "Solution", "SearchResult", "SearchStatistics", "SearchStatus", "Transition",
    "bfs", "ucs", "astar", "gbfs", "iw",
    # Heuristics
    "heuristics", "BlindHeuristic", "GoalCountHeuristic", "FFHeuristic", "LiftedFFHeuristic",
    "AddHeuristic", "MaxHeuristic", "H2Heuristic", "SetAddHeuristic", "PerfectHeuristic",
    # Datasets
    "state_space", "StateLabel", "StateSpace",
    # Learning
    "learning", "EncodingContext", "RelationBuffer", "RelationDescriptor",
    "encode_state", "encode_goal",
    "encode_action_list", "encode_transition_effects", "encode_virtual_node",
    "encode_expressive_state", "encode_expressive_goal",
    # Errors
    "MimirError", "PddlError", "StateSpaceLimitExceeded",
]
