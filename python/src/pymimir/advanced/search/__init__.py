# Import all classes for better IDE support

# Enums
from pymimir.pymimir.advanced.search import (
    SearchMode,
    SearchNodeStatus,
    SearchStatus,
    MatchTreeSplitMetric,
    MatchTreeSplitStrategy,
    MatchTreeOptimizationDirection,
)

# Common
from pymimir.pymimir.advanced.search import (   
    BlindHeuristic,
    IApplicableActionGenerator,
    IAxiomEvaluator,
    IHeuristic,
    Plan,
    SearchResult,
    State,
    StateList,
    StateRepository,
    SearchContext,
    SearchContextOptions,
    GeneralizedSearchContext,
)

# SatisficingBindingGenerator
from pymimir.pymimir.advanced.search import (
    ActionSatisficingBindingGenerator,
    AxiomSatisficingBindingGenerator,
    ConjunctiveConditionSatisficingBindingGenerator
)

# GoalStrategy
from pymimir.pymimir.advanced.search import (
    IGoalStrategy,
    ProblemGoalStrategy,

    IPruningStrategy,
    NoPruningStrategy,
    DuplicatePruningStrategy,
    ArityZeroNoveltyPruningStrategy,
    ArityKNoveltyPruningStrategy,
)

# AStar
from pymimir.pymimir.advanced.search import (
    AStarStatistics,
    IAStarEventHandler,
    DebugAStarEventHandler,
    DefaultAStarEventHandler,
    find_solution_astar,
)

# BrFs
from pymimir.pymimir.advanced.search import (
    BrFSStatistics,
    IBrFSEventHandler,
    DebugBrFSEventHandler,
    DefaultBrFSEventHandler,
    find_solution_brfs,
)

# GBFS
from pymimir.pymimir.advanced.search import (
    GBFSStatistics,
    IGBFSEventHandler,
    DebugGBFSEventHandler,
    DefaultGBFSEventHandler,
    find_solution_gbfs,
)

# IW
from pymimir.pymimir.advanced.search import (
    IWStatistics,
    IIWEventHandler,
    DefaultIWEventHandler,
    find_solution_iw,

    TupleIndexMapper,
    StateTupleIndexGenerator,
    StatePairTupleIndexGenerator,
)

# SIW
from pymimir.pymimir.advanced.search import (
    SIWStatistics,
    ISIWEventHandler,
    DefaultSIWEventHandler,
    find_solution_siw,
)

# Lifted
from pymimir.pymimir.advanced.search import (
    DebugLiftedApplicableActionGeneratorEventHandler,
    DefaultLiftedApplicableActionGeneratorEventHandler,
    LiftedApplicableActionGenerator,
    LiftedAxiomEvaluator,
    ILiftedApplicableActionGeneratorEventHandler,
    ILiftedAxiomEvaluatorEventHandler,
)

# Grounded
from pymimir.pymimir.advanced.search import (
    DebugGroundedApplicableActionGeneratorEventHandler,
    DefaultGroundedApplicableActionGeneratorEventHandler,
    GroundedApplicableActionGenerator,
    GroundedAxiomEvaluator,
    IGroundedApplicableActionGeneratorEventHandler,
    IGroundedAxiomEvaluatorEventHandler,
    DeleteRelaxedProblemExplorator,
    MatchTreeOptions,
)
