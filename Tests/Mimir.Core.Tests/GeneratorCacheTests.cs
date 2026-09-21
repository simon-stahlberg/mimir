using System.Runtime.CompilerServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;

namespace Mimir.Core.Tests;

public sealed class GeneratorCacheTests
{
    [Fact]
    public void CacheUsesStateIdentityAndKeepsInitialGeneratorPinned()
    {
        Problem problem = CreateProblem();
        IApplicableActionGenerator initial = problem.GetApplicableActionGenerator(problem.InitialState);
        State first = CreateState(problem);
        State equal = CreateState(problem);
        Assert.Equal(first, equal);
        Assert.NotSame(first, equal);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(first);
        Assert.Same(generator, problem.GetApplicableActionGenerator(first));
        Assert.NotSame(generator, problem.GetApplicableActionGenerator(equal));

        var states = new List<State>();
        for (int i = 0; i < 2 * Problem.StateGeneratorCacheCapacity; i++)
        {
            State state = CreateState(problem);
            states.Add(state);
            problem.GetApplicableActionGenerator(state);
        }

        Assert.Same(initial, problem.GetApplicableActionGenerator(problem.InitialState));
        Assert.NotSame(generator, problem.GetApplicableActionGenerator(first));
        Assert.NotEmpty(generator.GetApplicableActions(first.Expand()));
        GC.KeepAlive(states);
    }

    [Fact]
    public void CacheEvictsInInsertionOrderEvenAfterAHit()
    {
        Problem problem = CreateProblem();
        State[] states = Enumerable.Range(0, Problem.StateGeneratorCacheCapacity + 1)
            .Select(_ => CreateState(problem)).ToArray();
        IApplicableActionGenerator first = problem.GetApplicableActionGenerator(states[0]);
        IApplicableActionGenerator second = problem.GetApplicableActionGenerator(states[1]);
        for (int i = 2; i < Problem.StateGeneratorCacheCapacity; i++)
            problem.GetApplicableActionGenerator(states[i]);

        Assert.Same(first, problem.GetApplicableActionGenerator(states[0]));
        problem.GetApplicableActionGenerator(states[^1]);
        Assert.Same(second, problem.GetApplicableActionGenerator(states[1]));
        Assert.NotSame(first, problem.GetApplicableActionGenerator(states[0]));
        GC.KeepAlive(states);
    }

    [Fact]
    public void CacheDoesNotRetainUnusedStateOrGenerator()
    {
        Problem problem = CreateProblem();
        (WeakReference state, WeakReference generator) = PopulateWeakEntry(problem);
        GC.Collect();
        GC.WaitForPendingFinalizers();
        GC.Collect();
        Assert.False(state.IsAlive);
        Assert.False(generator.IsAlive);
        GC.KeepAlive(problem);
    }

    [Fact]
    public void LiftedGeneratorIsSharedAcrossStates()
    {
        Problem problem = CreateProblem(ApplicableActionGeneratorType.Lifted);
        Assert.Same(problem.GetApplicableActionGenerator(problem.InitialState),
            problem.GetApplicableActionGenerator(CreateState(problem)));
    }

    [Fact]
    public void ForeignStateIsRejected()
    {
        Problem problem = CreateProblem();
        Assert.Throws<ArgumentException>(() => problem.GetApplicableActionGenerator(CreateProblem().InitialState));
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    private static (WeakReference State, WeakReference Generator) PopulateWeakEntry(Problem problem)
    {
        State state = CreateState(problem);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(state);
        return (new WeakReference(state), new WeakReference(generator));
    }

    private static State CreateState(Problem problem)
        => problem.InitialState.WithAdditionalFluentFacts([problem.Context.Fluents.Single()]);

    private static Problem CreateProblem(ApplicableActionGeneratorType generatorType = ApplicableActionGeneratorType.Grounded)
    {
        Domain domain = Domain.FromText("""
            (define (domain cache) (:requirements :strips) (:predicates (p))
              (:action set :parameters () :precondition () :effect (p))
              (:action clear :parameters () :precondition (p) :effect (not (p))))
            """);
        return Problem.FromText(domain,
            "(define (problem p) (:domain cache) (:init) (:goal (p)))", generatorType);
    }
}
