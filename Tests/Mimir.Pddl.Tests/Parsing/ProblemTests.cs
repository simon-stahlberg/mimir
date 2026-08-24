using System;
using System.IO;
using Mimir.Pddl.Analysis;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;
using Xunit;

namespace Mimir.Pddl.Tests.Parsing;

public class ProblemTests
{
    [Theory]
    [InlineData("blocks_4")]
    [InlineData("rovers")]
    [InlineData("logistics")]
    [InlineData("airport")]
    [InlineData("assembly")]
    [InlineData("barman")]
    [InlineData("childsnack")]
    [InlineData("delivery")]
    [InlineData("driverlog")]
    [InlineData("ferry")]
    [InlineData("grid")]
    [InlineData("gripper")]
    [InlineData("hiking")]
    [InlineData("miconic")]
    [InlineData("satellite")]
    [InlineData("visitall")]
    [InlineData("miconic-fulladl")]
    [InlineData("miconic-simpleadl")]
    [InlineData("refuel")]
    [InlineData("refuel-adl")]
    [InlineData("zenotravel")]
    [InlineData("schedule")]
    public void ParsesAndValidatesBenchmarkProblems(string domainDir)
    {
        // Navigate to the repository Tests/Examples folder relative to the test binary execution path.
        var basePath = Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");
        var path = Path.Combine(basePath, domainDir, "p01.pddl");

        var input = File.ReadAllText(path);
        var problem = PddlParser.ParseProblem(input);

        var domainFile = Path.Combine(basePath, domainDir, "domain.pddl");
        var domainInput = File.ReadAllText(domainFile);
        var domain = PddlParser.ParseDomain(domainInput);

        ProblemDefinition validated = SemanticValidator.ValidateProblem(domain, problem);
        Assert.Equal(domain.Name, validated.DomainName, ignoreCase: true);
    }

        [Fact]
        public void ParsesNumericInitializationsAndMetric()
        {
                var input = """
(define (problem p)
    (:domain d)
    (:init
        (ready)
        (= (distance a b) 2.5)
        (= (total-cost) 0))
    (:goal (ready))
    (:metric minimize (total-cost)))
""";

        var problem = PddlParser.ParseProblem(input);

        Assert.Equal(3, problem.Init.Length);
        PredicateCall ready = Assert.Single(problem.Init.OfType<PredicateCall>());
        Assert.Equal("ready", ready.Name);

        NumericInitialization distance = Assert.Single(
            problem.Init.OfType<NumericInitialization>(),
            initialization => initialization.Fluent.Name == "distance");
        Assert.Equal(2.5m, distance.Value.Value);

        NumericInitialization totalCost = Assert.Single(
            problem.Init.OfType<NumericInitialization>(),
            initialization => initialization.Fluent.Name == "total-cost");
        Assert.Empty(totalCost.Fluent.Arguments);

        Metric metric = Assert.IsType<Metric>(problem.Metric);
        Assert.Equal(MetricDirection.Minimize, metric.Direction);
        Assert.Equal("total-cost", Assert.IsType<FluentCall>(metric.Expression).Name);
        }

    [Fact]
    public void QuantifiedPreconditionsAliasNormalizesForDomainsAndProblems()
    {
        DomainDefinition domain = PddlParser.ParseDomain(
            "(define (domain d) (:requirements :typing :QuAnTiFiEd-PrEcOnDiTiOnS " +
            ":ExIsTeNtIaL-PrEcOnDiTiOnS :negative-preconditions))");
        ProblemDefinition problem = PddlParser.ParseProblem(
            "(define (problem p) (:domain d) (:requirements :typing :qUaNtIfIeD-pReCoNdItIoNs " +
            ":ExIsTeNtIaL-PrEcOnDiTiOnS :negative-preconditions) (:init) (:goal ()))");

        HashSet<PddlRequirement> expectedRequirements =
        [
            PddlRequirement.Typing,
            PddlRequirement.ExistentialPreconditions,
            PddlRequirement.UniversalPreconditions,
            PddlRequirement.NegativePreconditions
        ];
        Assert.True(expectedRequirements.SetEquals(domain.Requirements));
        Assert.True(expectedRequirements.SetEquals(problem.Requirements));

        string domainPddl = domain.ToPddlString();
        string problemPddl = problem.ToPddlString();
        Assert.DoesNotContain(":quantified-preconditions", domainPddl.ToLowerInvariant());
        Assert.DoesNotContain(":quantified-preconditions", problemPddl.ToLowerInvariant());
        Assert.True(expectedRequirements.SetEquals(PddlParser.ParseDomain(domainPddl).Requirements));
        Assert.True(expectedRequirements.SetEquals(PddlParser.ParseProblem(problemPddl).Requirements));
    }
}
