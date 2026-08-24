using System.Collections.Immutable;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;

namespace Mimir.Pddl.Tests.Ast;

public class TypedListSerializationTests
{
    [Fact]
    public void SerializerPreservesTypesAcrossGroupBoundaries()
    {
        PredicateDeclaration[] declarations =
        [
            Predicate("?root", "object", "?typed", "base"),
            new PredicateDeclaration(
                "p",
                ImmutableArray.Create(
                    new Parameter("?typed", "base"),
                    new Parameter("?first"),
                    new Parameter("?second"))),
            Predicate("?first", "base", "?second", "base"),
            new PredicateDeclaration(
                "p",
                ImmutableArray.Create(
                    new Parameter("?a", "base"),
                    new Parameter("?b"),
                    new Parameter("?c", "base")))
        ];

        foreach (PredicateDeclaration declaration in declarations)
            Assert.Equal(declaration, ReparsePredicate(declaration));
    }

    [Fact]
    public void EveryDomainTypedListContextRoundTripsWithoutTypeLoss()
    {
        const string input = """
            (define (domain typed-lists)
              (:requirements :typing)
              (:types root - object child - root)
              (:constants plain - object typed - root)
              (:predicates (p ?plain - object ?typed - root))
              (:functions (f ?plain - object ?typed - root))
              (:action a
                :parameters (?plain - object ?typed - root)
                :precondition (forall (?plain - object ?typed - root) (p ?plain ?typed))
                :effect (forall (?plain - object ?typed - root) (p ?plain ?typed))))
            """;

        DomainDefinition parsed = PddlParser.ParseDomain(input);
        DomainDefinition reparsed = PddlParser.ParseDomain(parsed.ToPddlString());

        Assert.Equal(parsed, reparsed);
    }

    [Fact]
    public void ProblemObjectsRoundTripWithoutTypeLoss()
    {
        const string input = """
            (define (problem typed-problem)
              (:domain typed-lists)
              (:objects plain - object typed - root)
              (:init)
              (:goal ()))
            """;

        ProblemDefinition parsed = PddlParser.ParseProblem(input);
        ProblemDefinition reparsed = PddlParser.ParseProblem(parsed.ToPddlString());

        Assert.Equal(parsed, reparsed);
    }

    private static PredicateDeclaration Predicate(
        string firstName,
        string firstType,
        string secondName,
        string secondType)
    {
        return new PredicateDeclaration(
            "p",
            ImmutableArray.Create(
                new Parameter(firstName, firstType),
                new Parameter(secondName, secondType)));
    }

    private static PredicateDeclaration ReparsePredicate(PredicateDeclaration declaration)
    {
        DomainDefinition domain = PddlParser.ParseDomain(
            $"(define (domain typed-list) (:predicates {declaration.ToPddlString()}))");
        return Assert.Single(domain.Predicates);
    }
}
