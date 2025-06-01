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
    is_applicable,
    IApplicableActionGenerator,
    IAxiomEvaluator,
    Plan,
    PartiallyOrderedPlan,
    SearchResult,
    State,
    StateList,
    StateRepository,
    compute_state_metric_value,
    SearchContext,
    SearchContextOptions,
    GeneralizedSearchContext,
)

# Heuristics 
from pymimir.pymimir.advanced.search import (  
    PreferredActions, 
    IHeuristic,
    BlindHeuristic,
    PerfectHeuristic,
    MaxHeuristic,
    AddHeuristic,
    SetAddHeuristic,
    FFHeuristic
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

    IExplorationStrategy,
)

# AStar_EAGER
from pymimir.pymimir.advanced.search import (
    AStarEagerStatistics,
    IAStarEagerEventHandler,
    DebugAStarEagerEventHandler,
    DefaultAStarEagerEventHandler,
    AStarEagerOptions,
    find_solution_astar_eager,
)

# AStar_LAZY
from pymimir.pymimir.advanced.search import (
    AStarLazyStatistics,
    IAStarLazyEventHandler,
    DebugAStarLazyEventHandler,
    DefaultAStarLazyEventHandler,
    AStarLazyOptions,
    find_solution_astar_lazy,
)

# BrFs
from pymimir.pymimir.advanced.search import (
    BrFSStatistics,
    IBrFSEventHandler,
    DebugBrFSEventHandler,
    DefaultBrFSEventHandler,
    BrFSOptions,
    find_solution_brfs,
)

# GBFS_EAGER
from pymimir.pymimir.advanced.search import (
    GBFSEagerStatistics,
    IGBFSEagerEventHandler,
    DebugGBFSEagerEventHandler,
    DefaultGBFSEagerEventHandler,
    GBFSEagerOptions,
    find_solution_gbfs_eager,
)

# GBFS_LAZY
from pymimir.pymimir.advanced.search import (
    GBFSLazyStatistics,
    IGBFSLazyEventHandler,
    DebugGBFSLazyEventHandler,
    DefaultGBFSLazyEventHandler,
    GBFSLazyOptions,
    find_solution_gbfs_lazy,
)

# IW
from pymimir.pymimir.advanced.search import (
    IWStatistics,
    IIWEventHandler,
    DefaultIWEventHandler,
    IWOptions,
    find_solution_iw,

    TupleIndexMapper,
    DynamicNoveltyTable,
    StateTupleIndexGenerator,
    StatePairTupleIndexGenerator,
)

# SIW
from pymimir.pymimir.advanced.search import (
    SIWStatistics,
    ISIWEventHandler,
    DefaultSIWEventHandler,
    SIWOptions,
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
