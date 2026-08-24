using System;
using System.IO;
using Mimir.Pddl.Parsing;
using Mimir.Pddl.Analysis;
using Xunit;

namespace Mimir.Pddl.Tests.Parsing;

public class DomainTests
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
    public void ParsesValidatesAndCanonicalizesBenchmarkDomains(string domainDir)
    {
        // Navigate to the repository Tests/Examples folder relative to the test binary execution path.
        var basePath = Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");
        var domainFile = Path.Combine(basePath, domainDir, "domain.pddl");

        var input = File.ReadAllText(domainFile);
        var domain = PddlParser.ParseDomain(input);

        domain = SemanticValidator.ValidateDomain(domain);
        var compiled = Canonicalizer.Compile(domain);
        Assert.True(compiled.IsCanonical());
    }
}
