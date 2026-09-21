using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Planning;
using Mimir.Search.Space;
using Xunit;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class SearchSpaceTests
{
    [Fact]
    public void Builder_RejectsComponentsFromDifferentProblems()
    {
        var first = SearchTestHelpers.LoadProblem("ferry");
        var second = SearchTestHelpers.LoadProblem("ferry");
        var builder = new SearchSpaceBuilder()
            .WithInitialState(first.InitialState)
            .WithGoal(second);

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() => builder.Build());

        Assert.Contains("goal condition", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void SearchSpaceStatistics_DefensivelyCopiesDistributions()
    {
        var depthDistribution = new Dictionary<int, int> { [0] = 1 };
        var outDegreeDistribution = new Dictionary<int, int> { [1] = 1 };
        var statistics = new SearchSpaceStatistics(
            1,
            0,
            0,
            1,
            0,
            0,
            0,
            0,
            TimeSpan.Zero,
            depthDistribution,
            outDegreeDistribution);

        depthDistribution[0] = 2;
        outDegreeDistribution[2] = 1;

        Assert.Equal(1, statistics.DepthDistribution[0]);
        Assert.False(statistics.OutDegreeDistribution.ContainsKey(2));
        Assert.Throws<NotSupportedException>(
            () => ((IDictionary<int, int>)statistics.DepthDistribution).Add(2, 1));
    }

    [Fact]
    public void Builder_WithInitialStateAndGoal_BuildsSearchSpace()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var goalCondition = GoalCondition.FromProblem(problem);

        // Act
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(goalCondition)
            .Build();

        // Assert
        Assert.NotNull(searchSpace);
        Assert.Equal(problem.InitialState, searchSpace.InitialState);
        Assert.True(searchSpace.TotalStates > 0);
    }

    [Fact]
    public void Builder_MissingInitialState_ThrowsInvalidOperationException()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        var builder = new SearchSpaceBuilder()
            .WithGoal(GoalCondition.FromProblem(problem));

        // Act & Assert
        Assert.Throws<InvalidOperationException>(() => builder.Build());
    }

    [Fact]
    public void Builder_MissingGoal_ThrowsInvalidOperationException()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        var builder = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState);

        // Act & Assert
        Assert.Throws<InvalidOperationException>(() => builder.Build());
    }

    [Fact]
    public void Builder_ObtainsActionGeneratorFromProblem()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");

        var builder = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(GoalCondition.FromProblem(problem));

        // Act & Assert
        Assert.NotEmpty(builder.Build().AllStates);
    }

    [Fact]
    public void InitialState_IsInSearchSpace()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act
        var initialState = searchSpace.InitialState;
        var result = searchSpace.Query.GetDistanceResult(initialState);

        // Assert
        Assert.True(result.IsReachable);
        Assert.Equal(0, searchSpace.Query.GetDepth(initialState));
        Assert.Equal(0.0, searchSpace.Query.GetCost(initialState));
    }

    [Fact]
    public void IsGoalState_ReturnsCorrectResult()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var goalCondition = GoalCondition.FromProblem(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(goalCondition)
            .Build();

        // Find a goal state by executing a valid plan
        var planResult = new BreadthFirstPlanner().Solve(problem);
        Assert.True(planResult.IsSuccess);
        State goalState = problem.InitialState;
        foreach (var action in planResult.Plan)
        {
            goalState = goalState.Expand().Apply(action);
        }

        // Assert - initial state is not a goal (for ferry problem)
        Assert.False(searchSpace.Query.IsGoalState(problem.InitialState));

        Assert.True(searchSpace.Query.IsGoalState(goalState));
    }

    [Fact]
    public void GetDistanceToGoal_ReturnsCorrectResult()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act
        var initialDistance = searchSpace.Query.GetDistanceToGoal(problem.InitialState);

        // Assert
        Assert.Equal(7, initialDistance);
    }

    [Fact]
    public void GetCostToGoal_ReturnsCorrectResult()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("transport");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act
        var initialCost = searchSpace.Query.GetCostToGoal(problem.InitialState);

        // Assert
        Assert.Equal(54.0, initialCost);
    }

    [Fact]
    public void IsDeadEnd_ReturnsCorrectResult()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // The initial state should not be a dead-end for a solvable problem
        Assert.False(searchSpace.Query.IsDeadEnd(problem.InitialState));
    }

    [Fact]
    public void GetDepth_ReturnsCorrectResult()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act - Initial state should have depth 0
        Assert.Equal(0, searchSpace.Query.GetDepth(problem.InitialState));
    }

    [Fact]
    public void GetPredecessors_MirrorsEverySuccessorTransition()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("blocks_4");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        var missingReverseEdges = new List<(State from, State to, Action action)>();

        // Act
        foreach (var state in searchSpace.AllStates)
        {
            foreach (var (successor, action) in searchSpace.Query.GetSuccessors(state))
            {
                var predecessors = searchSpace.Query.GetPredecessors(successor);
                var hasMatchingPredecessor = predecessors.Any(predecessor =>
                    StateEqualityComparer.Instance.Equals(predecessor.Item1, state)
                    && ReferenceEquals(predecessor.Item2, action));

                if (!hasMatchingPredecessor)
                {
                    missingReverseEdges.Add((state, successor, action));
                }
            }
        }

        // Assert
        Assert.Empty(missingReverseEdges);
    }

    [Fact]
    public void GetDistanceResult_ReturnsCorrectStruct()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act - Get distance result for initial state
        var result = searchSpace.Query.GetDistanceResult(problem.InitialState);

        // Assert - should be reachable (not dead-end for solvable problem)
        Assert.True(result.IsReachable);
        Assert.False(result.IsDeadEnd);
    }

    [Fact]
    public void Query_ForUnknownState_ReturnsDocumentedEmptyResults()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        State unknownState = problem.InitialState.WithAdditionalFluentFacts(problem.Context.Fluents);
        Assert.DoesNotContain(searchSpace.AllStates, state => state.Equals(unknownState));

        Assert.Equal(StateDistanceResult.None, searchSpace.Query.GetDistanceResult(unknownState));
        Assert.Equal(-1, searchSpace.Query.GetDistanceToGoal(unknownState));
        Assert.Equal(double.PositiveInfinity, searchSpace.Query.GetCostToGoal(unknownState));
        Assert.False(searchSpace.Query.IsGoalState(unknownState));
        Assert.False(searchSpace.Query.IsDeadEnd(unknownState));
        Assert.Equal(-1, searchSpace.Query.GetDepth(unknownState));
        Assert.Equal(-1d, searchSpace.Query.GetCost(unknownState));
        Assert.Empty(searchSpace.Query.GetSuccessors(unknownState));
        Assert.Empty(searchSpace.Query.GetPredecessors(unknownState));
        Assert.Empty(searchSpace.Query.GetApplicableActions(unknownState));
    }

    [Fact]
    public void Statistics_ReturnsCorrectData()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act
        var stats = searchSpace.Query.GetStatistics();

        Assert.Equal(searchSpace.AllStates.Count, stats.TotalStates);
        Assert.Equal(searchSpace.GoalStates.Count, stats.GoalStateCount);
        Assert.Equal(searchSpace.DeadEndStates.Count, stats.DeadEndStateCount);
        Assert.Equal(searchSpace.AllStates.Max(searchSpace.Query.GetDepth), stats.MaxDepth);
        Assert.Equal(searchSpace.TotalTransitions, stats.TotalTransitions);
        Assert.Equal(
            (double)stats.TotalTransitions / stats.TotalStates,
            stats.AverageOutDegree);
        Assert.Equal(stats.AverageOutDegree, stats.AverageInDegree);

        int totalApplicableActions = searchSpace.AllStates
            .Sum(state => searchSpace.Query.GetApplicableActions(state).Count);
        Assert.Equal(
            (double)totalApplicableActions / stats.TotalStates,
            stats.AverageBranchingFactor);
    }

    [Fact]
    public void BlocksWorld_HasStateOneStepFromGoal()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("blocks_4");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act
        var hasDistanceOneState = searchSpace.AllStates
            .Any(state => searchSpace.Query.GetDistanceToGoal(state) == 1);

        // Assert
        Assert.True(hasDistanceOneState);
    }

    [Fact]
    public void GoalStates_ReturnsAllGoalStates()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act
        var goalStates = searchSpace.GoalStates;

        Assert.NotEmpty(goalStates);
        Assert.All(goalStates, state => Assert.True(searchSpace.Query.IsGoalState(state)));
        Assert.Equal(searchSpace.GoalStateCount, goalStates.Count);
    }

    [Fact]
    public void Query_GetDistanceToGoal_ReturnsConsistentResults()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act - Get distances for all states
        var distances = searchSpace.AllStates
            .Select(s => (State: s, Distance: searchSpace.Query.GetDistanceToGoal(s)))
            .ToList();

        // Assert - goal states should have distance 0
        var goalDistances = distances.Where(d => searchSpace.Query.IsGoalState(d.State)).ToList();
        foreach (var (state, distance) in goalDistances)
        {
            Assert.Equal(0, distance);
        }
    }

    [Fact]
    public void Query_GetCostToGoal_ReturnsConsistentResults()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("transport");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act - Get costs for all states
        var costs = searchSpace.AllStates
            .Select(s => (State: s, Cost: searchSpace.Query.GetCostToGoal(s)))
            .ToList();

        // Assert - goal states should have cost 0
        var goalCosts = costs.Where(c => searchSpace.Query.IsGoalState(c.State)).ToList();
        foreach (var (state, cost) in goalCosts)
        {
            Assert.Equal(0.0, cost);
        }
    }

    [Fact]
    public void Distance_ToGoal_StictlyDecreasingAlongPath()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act - For every non-goal, non-dead-end state, at least one successor has strictly decreasing distance
        var violations = new List<(State state, int dist)>();

        foreach (var state in searchSpace.AllStates)
        {
            var dist = searchSpace.Query.GetDistanceToGoal(state);
            if (searchSpace.Query.IsGoalState(state) || dist <= 0)
                continue;

            var successors = searchSpace.Query.GetSuccessors(state);
            var hasDecreasing = successors.Any(s => searchSpace.Query.GetDistanceToGoal(s.Item1) < dist);
            if (!hasDecreasing)
            {
                violations.Add((state, dist));
            }
        }

        // Assert
        Assert.Empty(violations);
    }

    [Fact]
    public void Cost_ToGoal_StictlyDecreasingAlongPath()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("transport");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act - For every non-goal, non-dead-end state with finite cost, at least one successor has strictly decreasing cost
        var violations = new List<(State state, double cost)>();

        foreach (var state in searchSpace.AllStates)
        {
            var cost = searchSpace.Query.GetCostToGoal(state);
            if (searchSpace.Query.IsGoalState(state) || cost <= 0 || cost == double.PositiveInfinity)
                continue;

            var successors = searchSpace.Query.GetSuccessors(state);
            var hasDecreasing = successors.Any(s => searchSpace.Query.GetCostToGoal(s.Item1) < cost);
            if (!hasDecreasing)
            {
                violations.Add((state, cost));
            }
        }

        // Assert
        Assert.Empty(violations);
    }

    [Fact]
    public void Depth_AlwaysDecreasesOrEqualsToOneAlongEachEdge()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act - For every edge (s1 -> s2), depth(s2) <= depth(s1) + 1
        // (BFS ensures s2's depth is the shortest path, so it could be <= fromDepth+1 if s2 was reached earlier)
        var violations = new List<(State from, State to, int fromDepth, int toDepth)>();

        foreach (var state in searchSpace.AllStates)
        {
            var fromDepth = searchSpace.Query.GetDepth(state);
            var successors = searchSpace.Query.GetSuccessors(state);
            foreach (var successorTuple in successors)
            {
                var successor = successorTuple.Item1;
                var toDepth = searchSpace.Query.GetDepth(successor);
                if (toDepth > fromDepth + 1)
                {
                    violations.Add((state, successor, fromDepth, toDepth));
                }
            }
        }

        // Assert
        Assert.Empty(violations);
    }

    [Fact]
    public void InitialState_DistanceEqualsPlanLength()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Get optimal plan length
        var planResult = new BreadthFirstPlanner().Solve(problem);
        Assert.True(planResult.IsSuccess);
        var planLength = planResult.Plan.Count;

        // Act
        var distance = searchSpace.Query.GetDistanceToGoal(problem.InitialState);

        // Assert
        Assert.Equal(planLength, distance);
    }

    [Fact]
    public void Builder_MaxStatesBelowReachableStateCount_ThrowsStateSpaceLimitExceeded()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchSpace completeSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();
        Assert.True(completeSpace.TotalStates > 1);

        var builder = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithMaxStates(completeSpace.TotalStates - 1);

        StateSpaceLimitExceededException exception =
            Assert.Throws<StateSpaceLimitExceededException>(() => builder.Build());
        Assert.Equal(
            $"Search-space expansion exceeded the maximum of {completeSpace.TotalStates - 1} states.",
            exception.Message);
    }

    [Theory]
    [InlineData(0)]
    [InlineData(-1)]
    public void Builder_NonPositiveMaxStates_Throws(int maxStates)
    {
        var builder = new SearchSpaceBuilder();

        Assert.Throws<ArgumentOutOfRangeException>(() => builder.WithMaxStates(maxStates));
    }

    [Fact]
    public void Builder_MaxStatesEqualToReachableStateCount_BuildsCompleteSpace()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchSpace completeSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithMaxStates(completeSpace.TotalStates)
            .Build();

        Assert.Equal(completeSpace.TotalStates, searchSpace.TotalStates);
        Assert.Equal(completeSpace.TotalTransitions, searchSpace.TotalTransitions);
    }

    [Fact]
    public void Spanner_DeadEndStatesHaveNoPathToGoal()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("spanner");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act
        var candidates = searchSpace.AllStates
            .Where(state => searchSpace.Query.IsDeadEnd(state))
            .ToList();

        // Assert
        Assert.NotEmpty(candidates);
        Assert.All(candidates, state =>
        {
            Assert.True(searchSpace.Query.IsDeadEnd(state));
            Assert.Equal(-1, searchSpace.Query.GetDistanceToGoal(state));
        });
    }

    [Fact]
    public void StateDistanceResult_Goal_ReturnsGoalStruct()
    {
        // Arrange
        var result = StateDistanceResult.Goal;

        // Assert
        Assert.True(result.IsReachable);
        Assert.False(result.IsDeadEnd);
        Assert.True(result.IsGoalState);
        Assert.Equal(0, result.Distance);
        Assert.Equal(0.0, result.Cost);
    }

    [Fact]
    public void StateDistanceResult_DeadEnd_ReturnsDeadEndStruct()
    {
        // Arrange
        var result = StateDistanceResult.DeadEnd;

        // Assert
        Assert.False(result.IsReachable);
        Assert.True(result.IsDeadEnd);
        Assert.False(result.IsGoalState);
        Assert.Equal(-1, result.Distance);
        Assert.Equal(double.PositiveInfinity, result.Cost);
    }

    [Fact]
    public void StateDistanceResult_None_ReturnsNoneStruct()
    {
        // Arrange
        var result = StateDistanceResult.None;

        // Assert
        Assert.False(result.IsReachable);
        Assert.False(result.IsDeadEnd);
        Assert.False(result.IsGoalState);
        Assert.Equal(-1, result.Distance);
        Assert.Equal(double.PositiveInfinity, result.Cost);
    }

    [Fact]
    public void StateDistanceResult_Reachable_ReturnsReachableStruct()
    {
        // Arrange
        var result = StateDistanceResult.Reachable(5, 10.0);

        // Assert
        Assert.True(result.IsReachable);
        Assert.False(result.IsDeadEnd);
        Assert.False(result.IsGoalState);
        Assert.Equal(5, result.Distance);
        Assert.Equal(10.0, result.Cost);
    }

    [Fact]
    public void Statistics_DepthDistribution_ReturnsCorrectDistribution()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act
        var stats = searchSpace.Query.GetStatistics();

        // Assert
        Assert.NotNull(stats.DepthDistribution);
        Assert.True(stats.DepthDistribution.Count > 0);

        // Verify sum of depth distribution equals total states
        var totalFromDistribution = stats.DepthDistribution.Values.Sum();
        Assert.Equal(searchSpace.TotalStates, totalFromDistribution);
    }

    [Fact]
    public void Statistics_OutDegreeDistribution_ReturnsCorrectDistribution()
    {
        // Arrange
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        // Act
        var stats = searchSpace.Query.GetStatistics();

        // Assert
        Assert.Equal(searchSpace.TotalStates, stats.OutDegreeDistribution.Values.Sum());
        int transitionsFromDistribution = stats.OutDegreeDistribution
            .Sum(pair => pair.Key * pair.Value);
        Assert.Equal(searchSpace.TotalTransitions, transitionsFromDistribution);
    }

    [Fact]
    public void SearchSpace_ExposesReadOnlyCollections()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchSpace searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();

        Assert.Equal(searchSpace.TotalStates, searchSpace.AllStates.Count);
        Assert.Equal(searchSpace.GoalStateCount, searchSpace.GoalStates.Count);
        Assert.Equal(searchSpace.DeadEndStateCount, searchSpace.DeadEndStates.Count);

        State initialState = problem.InitialState;
        Assert.Throws<NotSupportedException>(
            () => ((IList<State>)searchSpace.AllStates).Add(initialState));
        Assert.Throws<NotSupportedException>(
            () => ((IList<Action>)searchSpace.Query.GetApplicableActions(initialState)).Clear());
        Assert.Throws<NotSupportedException>(
            () => ((IList<(State Successor, Action Action)>)searchSpace.Query.GetSuccessors(initialState)).Clear());
        Assert.Throws<NotSupportedException>(
            () => ((IList<(State Predecessor, Action Action)>)searchSpace.Query.GetPredecessors(initialState)).Clear());
    }

    [Fact]
    public void Builder_GroundsFromItsInitialState()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        GroundedApplicableActionGenerator initialGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Action action = initialGenerator.GetApplicableActions(problem.InitialState.Expand()).First();
        State customStart = problem.InitialState.Expand().Apply(action);

        SearchSpace searchSpace = new SearchSpaceBuilder()
            .WithInitialState(customStart)
            .WithGoal(problem)
            .Build();

        Assert.Equal(customStart, searchSpace.InitialState);
    }

    [Fact]
    public void Builder_WithCanceledToken_ThrowsOperationCanceledException()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        var builder = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem);

        Assert.Throws<OperationCanceledException>(() => builder.Build(new CancellationToken(canceled: true)));
    }

}
