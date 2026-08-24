using Mimir.Core.Schemas;

namespace Mimir.Core.Tests;

public sealed class SemanticValidationLoadingTests
{
    [Theory]
    [InlineData(
        "(define (domain duplicate-actions) (:requirements :strips) (:predicates (ready)) "
        + "(:action move :parameters () :effect (ready)) "
        + "(:action MOVE :parameters () :effect (ready)))",
        "Duplicate action declaration 'MOVE'")]
    [InlineData(
        "(define (domain duplicate-predicate-parameters) "
        + "(:predicates (connected ?item ?ITEM)))",
        "Duplicate parameter '?ITEM' in predicate 'connected'")]
    [InlineData(
        "(define (domain duplicate-function-parameters) (:requirements :numeric-fluents) "
        + "(:functions (distance ?item ?ITEM)))",
        "Duplicate parameter '?ITEM' in numeric fluent 'distance'")]
    public void DomainFromText_MapsCaseInsensitiveDeclarationDuplicatesToValidationError(
        string domainPddl,
        string expectedMessage)
    {
        PddlLoadException exception = Assert.Throws<PddlLoadException>(
            () => Domain.FromText(domainPddl));

        Assert.Equal(PddlDocumentType.Domain, exception.DocumentType);
        Assert.Equal(PddlLoadErrorCode.Validation, exception.ErrorCode);
        Assert.Contains(expectedMessage, exception.Message);
    }

    [Fact]
    public void ProblemFromFile_MapsDuplicateNumericInitializationToValidationError()
    {
        Domain domain = Domain.FromText("""
(define (domain duplicate-numeric-init)
  (:requirements :strips :typing :numeric-fluents)
  (:types item)
  (:predicates (ready))
  (:functions (amount ?item - item)))
""");
        string problemPath = Path.Combine(
            Path.GetTempPath(),
            $"mimir-duplicate-numeric-{Guid.NewGuid():N}.pddl");
        File.WriteAllText(problemPath, """
(define (problem duplicate-init)
  (:domain duplicate-numeric-init)
  (:objects first - item)
  (:init
    (ready)
    (= (amount first) 1)
    (= (AMOUNT FIRST) 2))
  (:goal (ready)))
""");

        try
        {
            PddlLoadException exception = Assert.Throws<PddlLoadException>(
                () => Problem.FromFile(domain, problemPath));

            Assert.Equal(PddlDocumentType.Problem, exception.DocumentType);
            Assert.Equal(PddlLoadErrorCode.Validation, exception.ErrorCode);
            Assert.Equal(problemPath, exception.SourcePath);
            Assert.Contains("initialized more than once", exception.Message);
        }
        finally
        {
            File.Delete(problemPath);
        }
    }

    [Fact]
    public void ProblemFromText_RejectsObjectThatReusesDomainConstantNameIgnoringCase()
    {
        Domain domain = Domain.FromText("""
(define (domain object-constant-collision)
  (:requirements :strips :typing)
  (:types vehicle location)
  (:constants shared - vehicle)
  (:predicates (ready)))
""");

        PddlLoadException exception = Assert.Throws<PddlLoadException>(() => Problem.FromText(domain, """
(define (problem collision)
  (:domain object-constant-collision)
  (:objects SHARED - location)
  (:init (ready))
  (:goal (ready)))
"""));

        Assert.Equal(PddlDocumentType.Problem, exception.DocumentType);
        Assert.Equal(PddlLoadErrorCode.Validation, exception.ErrorCode);
        Assert.Contains("conflicts with a domain constant", exception.Message);
    }
}
