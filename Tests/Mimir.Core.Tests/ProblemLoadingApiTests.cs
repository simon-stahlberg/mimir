using System.Reflection;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Tests;

public class ProblemLoadingApiTests
{
    [Fact]
    public void FromTextFlattensNestedLiteralConjunction()
    {
        Domain domain = CreateDomain();

        Problem problem = Problem.FromText(domain, """
(define (problem nested-goal)
  (:domain goal-shapes)
  (:objects a b)
  (:init)
  (:goal (and (p a) (and (not (q a)) (p b)))))
""");

        Assert.Collection(
            problem.Goal,
            literal => AssertLiteral(literal, "p", Polarity.Positive, "a"),
            literal => AssertLiteral(literal, "q", Polarity.Negative, "a"),
            literal => AssertLiteral(literal, "p", Polarity.Positive, "b"));
    }

    [Fact]
    public void FromTextAcceptsEmptyConjunctionGoal()
    {
        Domain domain = CreateDomain();

        Problem problem = Problem.FromText(domain, """
(define (problem empty-goal)
  (:domain goal-shapes)
  (:objects)
  (:init)
  (:goal (and)))
""");

        Assert.Empty(problem.Goal);
    }

    [Theory]
    [InlineData("(or (p a) (q a))")]
    [InlineData("(imply (p a) (q a))")]
    [InlineData("(forall (?x) (p ?x))")]
    [InlineData("(exists (?x) (p ?x))")]
    [InlineData("(not (and (p a)))")]
    public void FromTextRejectsGoalsThatAreNotLiteralConjunctions(string goal)
    {
        Domain domain = CreateDomain();

        PddlLoadException exception = Assert.Throws<PddlLoadException>(() => Problem.FromText(domain, $"""
(define (problem unsupported-goal)
  (:domain goal-shapes)
  (:objects a)
  (:init)
  (:goal {goal}))
"""));

        Assert.Equal(PddlDocumentType.Problem, exception.DocumentType);
        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Null(exception.SourcePath);
    }

    [Fact]
    public void FromTextAcceptsEqualityGoalAsAnOrdinaryStaticLiteral()
    {
        Domain domain = CreateDomain();

        Problem problem = Problem.FromText(domain, """
(define (problem equality-goal)
  (:domain goal-shapes)
  (:objects a)
  (:init)
  (:goal (= a a)))
""");

        Literal<Fact> literal = Assert.Single(problem.Goal);
        Assert.Equal("=", literal.Value.Predicate.Name);
        Assert.True(problem.InitialState.Expand().IsTrue(literal.Value));
    }

    [Fact]
    public void FromTextMapsParseFailureAndSpanToCoreException()
    {
        Domain domain = CreateDomain();

        PddlLoadException exception = Assert.Throws<PddlLoadException>(
            () => Problem.FromText(domain, "(define (problem broken)"));

        Assert.Equal(PddlDocumentType.Problem, exception.DocumentType);
        Assert.NotNull(exception.Span);
        Assert.Null(exception.SourcePath);
        Assert.NotNull(exception.InnerException);
        Assert.NotEqual("Mimir.Pddl", exception.GetType().Assembly.GetName().Name);
    }

    [Fact]
    public void DomainFromTextMapsParseFailureAndSpanToCoreException()
    {
        PddlLoadException exception = Assert.Throws<PddlLoadException>(
            () => Domain.FromText("(define (domain broken)"));

        Assert.Equal(PddlDocumentType.Domain, exception.DocumentType);
        Assert.NotNull(exception.Span);
        Assert.Null(exception.SourcePath);
        Assert.NotNull(exception.InnerException);
    }

    [Fact]
    public void FromFilePreservesSourcePathAndFileErrors()
    {
        Domain domain = CreateDomain();
        string tempDirectory = Path.Combine(Path.GetTempPath(), "Mimir.Core.Tests", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempDirectory);
        string invalidProblemPath = Path.Combine(tempDirectory, "invalid.pddl");
        string missingProblemPath = Path.Combine(tempDirectory, "missing.pddl");
        File.WriteAllText(invalidProblemPath, "(define (problem broken)");

        try
        {
            PddlLoadException exception = Assert.Throws<PddlLoadException>(
                () => Problem.FromFile(domain, invalidProblemPath));

            Assert.Equal(invalidProblemPath, exception.SourcePath);
            Assert.Throws<FileNotFoundException>(() => Problem.FromFile(domain, missingProblemPath));
        }
        finally
        {
            Directory.Delete(tempDirectory, recursive: true);
        }
    }

    [Fact]
    public void DomainFromFilePreservesSourcePathAndFileErrors()
    {
        string tempDirectory = Path.Combine(
            Path.GetTempPath(),
            "Mimir.Core.Tests",
            Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempDirectory);
        string invalidDomainPath = Path.Combine(tempDirectory, "invalid-domain.pddl");
        string missingDomainPath = Path.Combine(tempDirectory, "missing-domain.pddl");
        File.WriteAllText(invalidDomainPath, "(define (domain broken)");

        try
        {
            PddlLoadException exception = Assert.Throws<PddlLoadException>(
                () => Domain.FromFile(invalidDomainPath));

            Assert.Equal(PddlDocumentType.Domain, exception.DocumentType);
            Assert.Equal(invalidDomainPath, exception.SourcePath);
            Assert.Throws<FileNotFoundException>(() => Domain.FromFile(missingDomainPath));
        }
        finally
        {
            Directory.Delete(tempDirectory, recursive: true);
        }
    }

    [Fact]
    public void PublicCoreSignaturesDoNotReferencePddlAssemblyTypes()
    {
        Assembly coreAssembly = typeof(Problem).Assembly;
        var exposedTypes = new List<Type>();

        foreach (Type type in coreAssembly.GetExportedTypes())
        {
            exposedTypes.AddRange(type.GetConstructors().SelectMany(constructor =>
                constructor.GetParameters().Select(parameter => parameter.ParameterType)));
            exposedTypes.AddRange(type.GetMethods(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static | BindingFlags.DeclaredOnly)
                .SelectMany(method => method.GetParameters().Select(parameter => parameter.ParameterType).Append(method.ReturnType)));
            exposedTypes.AddRange(type.GetProperties().Select(property => property.PropertyType));
            exposedTypes.AddRange(type.GetFields().Select(field => field.FieldType));
            exposedTypes.AddRange(type.GetEvents().Select(eventInfo => eventInfo.EventHandlerType!));
        }

        Assert.DoesNotContain(exposedTypes, ReferencesPddlAssembly);
    }

    private static Domain CreateDomain()
    {
        return Domain.FromText("""
(define (domain goal-shapes)
  (:requirements :strips :adl :equality)
  (:predicates (p ?x) (q ?x)))
""");
    }

    private static void AssertLiteral(
        Literal<Fact> literal,
        string predicateName,
        Polarity polarity,
        string argumentName)
    {
        Assert.Equal(predicateName, literal.Value.Predicate.Name);
        Assert.Equal(polarity, literal.Polarity);
        Assert.Equal(argumentName, Assert.Single(literal.Value.Arguments).Name);
    }

    private static bool ReferencesPddlAssembly(Type type)
    {
        if (type.Assembly.GetName().Name == "Mimir.Pddl")
            return true;

        if (type.HasElementType)
            return ReferencesPddlAssembly(type.GetElementType()!);

        return type.IsGenericType && type.GetGenericArguments().Any(ReferencesPddlAssembly);
    }
}
