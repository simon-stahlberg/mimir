using System.Collections;
using System.Reflection;
using Mimir.Core.Schemas;

namespace Mimir.Learning.Tests;

public class EncodingContextTests
{
    [Fact]
    public void AccumulatesBatchMetadataAndResetsActiveInstanceState()
    {
        (Domain domain, Problem first) = CreateProblem("first");
        (_, Problem second) = CreateProblem("second", domain);
        var context = new EncodingContext();

        Assert.Equal(0, context.BatchCount);
        Assert.Equal(0, context.NodeCount);
        Assert.Empty(context.Relations);
        Assert.Throws<InvalidOperationException>(() => context.EndInstance());
        Assert.Throws<InvalidOperationException>(() => context.NewActionId());
        Assert.Throws<InvalidOperationException>(() => _ = context.CurrentNodeOffset);
        Assert.Throws<InvalidOperationException>(() => _ = context.CurrentNodeCount);

        context.BeginInstance(first);
        Assert.Same(first, context.Problem);
        Assert.Equal(0, context.CurrentNodeOffset);
        Assert.Equal(2, context.CurrentNodeCount);
        Assert.Equal([0, 1], context.ObjectIdValues);
        Assert.Equal(0, context.GetObjectId(first.AllObjects[0]));
        Assert.Equal(2, context.NewVirtualId());
        Assert.Equal(3, context.NewActionId());
        Assert.Equal(2, context.NewOrExistingVirtualId());
        Assert.Equal(
            4,
            context.NewOrExistingObjectPairId(
                first.AllObjects[0],
                first.AllObjects[1]));
        Assert.Equal(
            4,
            context.NewOrExistingObjectPairId(
                first.AllObjects[0],
                first.AllObjects[1]));
        Assert.Equal(
            0,
            context.NewOrExistingObjectPairId(
                first.AllObjects[0],
                first.AllObjects[0]));
        Assert.Equal(5, context.NewAuxiliaryId());
        Assert.Equal(6, context.NewActionId());
        Assert.Equal([3, 6], context.ActionIds);
        Assert.Equal([2], context.VirtualIds);
        Assert.Equal([4, 5], context.AuxiliaryIds);
        Assert.Equal(7, context.NodeCount);
        Assert.Equal(7, context.CurrentNodeCount);
        Assert.Throws<InvalidOperationException>(() => context.BeginInstance(first));
        context.EndInstance();

        Assert.Equal(1, context.BatchCount);
        Assert.Equal([7], context.NodeSizes);
        Assert.Equal([2], context.ObjectSizes);
        Assert.Equal([0, 1], context.ObjectIndices);
        Assert.Equal([2], context.ActionSizes);
        Assert.Equal([3, 6], context.ActionIndices);
        Assert.Equal([1], context.VirtualSizes);
        Assert.Equal([2], context.VirtualIndices);
        Assert.Equal([2], context.AuxiliarySizes);
        Assert.Equal([4, 5], context.AuxiliaryIndices);
        Assert.Throws<InvalidOperationException>(() => _ = context.Problem);
        Assert.Throws<InvalidOperationException>(() => _ = context.ActionIds);
        Assert.Throws<InvalidOperationException>(() => _ = context.CurrentNodeOffset);
        Assert.Throws<InvalidOperationException>(() => _ = context.CurrentNodeCount);

        context.BeginInstance(second);
        Assert.Equal(7, context.CurrentNodeOffset);
        Assert.Equal(2, context.CurrentNodeCount);
        Assert.Equal([7, 8], context.ObjectIdValues);
        context.EndInstance();

        Assert.Equal(2, context.BatchCount);
        Assert.Equal(9, context.NodeCount);
        Assert.Equal([7, 2], context.NodeSizes);
        Assert.Equal([2, 2], context.ObjectSizes);
        Assert.Equal([0, 1, 7, 8], context.ObjectIndices);
        Assert.Equal([2, 0], context.ActionSizes);
        Assert.Equal([1, 0], context.VirtualSizes);
        Assert.Equal([2, 0], context.AuxiliarySizes);
    }

    [Fact]
    public void RequiresTheExactDomainAndCurrentProblemObjects()
    {
        (Domain domain, Problem problem) = CreateProblem("owned");
        (_, Problem sameDomain) = CreateProblem("same", domain);
        (_, Problem foreign) = CreateProblem("foreign");
        var context = new EncodingContext();

        context.BeginInstance(problem);
        Assert.Throws<ArgumentException>(() =>
            context.GetObjectId(sameDomain.DeclaredObjects[0]));
        context.EndInstance();
        context.BeginInstance(sameDomain);
        context.EndInstance();

        Assert.Throws<ArgumentException>(() => context.BeginInstance(foreign));
        Assert.Equal(2, context.BatchCount);
    }

    [Fact]
    public void ObjectPairLookupIsOrderedAndScopedToOneInstance()
    {
        (Domain domain, Problem problem) = CreateProblem("pairs");
        (_, Problem second) = CreateProblem("second-pairs", domain);
        var context = new EncodingContext();

        context.BeginInstance(problem);
        Constant first = problem.AllObjects[0];
        Constant secondObject = problem.AllObjects[1];
        Assert.False(context.TryGetObjectPairId(first, secondObject, out _));
        Assert.Equal(2, context.NewOrExistingObjectPairId(first, secondObject));
        Assert.True(context.TryGetObjectPairId(first, secondObject, out int forward));
        Assert.Equal(2, forward);
        Assert.False(context.TryGetObjectPairId(secondObject, first, out _));
        Assert.Equal(3, context.NewOrExistingObjectPairId(secondObject, first));
        Assert.True(context.TryGetObjectPairId(first, first, out int diagonal));
        Assert.Equal(0, diagonal);
        context.EndInstance();

        context.BeginInstance(second);
        Assert.False(context.TryGetObjectPairId(
            second.AllObjects[0],
            second.AllObjects[1],
            out _));
        Assert.Equal(6, context.NewOrExistingObjectPairId(
            second.AllObjects[0],
            second.AllObjects[1]));
        context.EndInstance();

        Assert.Equal([2, 1], context.AuxiliarySizes);
        Assert.Equal([2, 3, 6], context.AuxiliaryIndices);
    }

    [Fact]
    public void MetadataAndRelationsAreReadOnly()
    {
        (_, Problem problem) = CreateProblem("readonly");
        var context = new EncodingContext();
        context.BeginInstance(problem);
        Encoding.EncodeState(context, problem.InitialState.Expand());
        context.EndInstance();

        Assert.Throws<NotSupportedException>(() =>
            ((IList)context.NodeSizes).Add(1));
        Assert.Throws<NotSupportedException>(() =>
            ((IDictionary)context.Relations).Add("other", Array.Empty<int>()));
        IReadOnlyList<int> ready = context.Relations["relation_ready"];
        Assert.Throws<NotSupportedException>(() => ((IList)ready).Add(1));
    }

    [Fact]
    public void IndexedRelationsMatchThePublicDictionaryAndValidateBounds()
    {
        (_, Problem problem) = CreateProblem("indexed-relations");
        var context = new EncodingContext();
        context.BeginInstance(problem);
        Encoding.EncodeState(context, problem.InitialState.Expand());
        context.EndInstance();

        var indexedNames = new HashSet<string>(StringComparer.Ordinal);
        Assert.Equal(context.Relations.Count, context.RelationCount);
        for (int relationIndex = 0; relationIndex < context.RelationCount; relationIndex++)
        {
            KeyValuePair<string, IReadOnlyList<int>> relation = context.GetRelation(
                relationIndex);
            Assert.True(indexedNames.Add(relation.Key));
            Assert.Same(context.Relations[relation.Key], relation.Value);
        }
        Assert.Equal(context.Relations.Keys.ToHashSet(StringComparer.Ordinal), indexedNames);
        Assert.Throws<ArgumentOutOfRangeException>(() => context.GetRelation(-1));
        Assert.Throws<ArgumentOutOfRangeException>(() =>
            context.GetRelation(context.RelationCount));
    }

    [Fact]
    public void CopiesAllRelationsInEnumerationOrderWithoutPartialWrites()
    {
        (_, Problem problem) = CreateProblem("packed-relations");
        var context = new EncodingContext();
        context.BeginInstance(problem);
        Encoding.EncodeState(context, problem.InitialState.Expand());
        context.EndInstance();

        int[] expected = context.Relations.Values
            .SelectMany(values => values)
            .ToArray();
        Assert.Equal(expected.Length, context.RelationValueCount);

        var destination = new int[expected.Length];
        Assert.Equal(expected.Length, context.CopyRelationValues(destination));
        Assert.Equal(expected, destination);

        var tooSmall = Enumerable.Repeat(-1, expected.Length - 1).ToArray();
        Assert.Throws<ArgumentException>(() =>
            context.CopyRelationValues(tooSmall));
        Assert.All(tooSmall, value => Assert.Equal(-1, value));

        var empty = new EncodingContext();
        Assert.Equal(0, empty.RelationValueCount);
        Assert.Equal(0, empty.CopyRelationValues(Span<int>.Empty));
    }

    [Fact]
    public void RejectsNodeCountOverflowBeforeMutatingCategories()
    {
        Domain domain = Domain.FromText("""
(define (domain empty-overflow)
  (:requirements :strips)
  (:predicates (done)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem empty-overflow-problem)
  (:domain empty-overflow)
  (:init)
  (:goal (done)))
""");
        var context = new EncodingContext();
        FieldInfo nodeCount = typeof(EncodingContext).GetField(
            "_nodeCount",
            BindingFlags.Instance | BindingFlags.NonPublic)
            ?? throw new InvalidOperationException("Node-count field was not found.");
        nodeCount.SetValue(context, int.MaxValue);
        context.BeginInstance(problem);

        Assert.Throws<OverflowException>(() => context.NewVirtualId());
        Assert.Empty(context.VirtualIds);
        Assert.Empty(context.VirtualIndices);
        Assert.Equal(int.MaxValue, context.NodeCount);
    }

    private static (Domain Domain, Problem Problem) CreateProblem(
        string name,
        Domain? existingDomain = null)
    {
        Domain domain = existingDomain ?? Domain.FromText("""
(define (domain context-batch)
  (:requirements :strips :typing)
  (:types item)
  (:constants shared - item)
  (:predicates (ready ?item - item)))
""");
        Problem problem = Problem.FromText(domain, $$"""
(define (problem {{name}})
  (:domain context-batch)
  (:objects local - item)
  (:init (ready shared) (ready local))
  (:goal (ready local)))
""");
        return (domain, problem);
    }
}
