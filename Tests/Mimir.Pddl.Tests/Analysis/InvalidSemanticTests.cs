using System;
using System.IO;
using Mimir.Pddl.Analysis;
using Mimir.Pddl.Parsing;
using Xunit;

namespace Mimir.Pddl.Tests.Analysis;

public class InvalidSemanticTests
{
    [Theory]
    [InlineData("blocks_4", "domain-invalid.pddl", "Undeclared predicate 'fake-pred'")]
    [InlineData("rovers", "domain-invalid.pddl", "Arity mismatch for predicate 'at'")]
    [InlineData("logistics", "domain-invalid.pddl", "Undeclared variable '?fakevar'")]
    [InlineData("airport", "domain-invalid.pddl", "Type mismatch")]
    [InlineData("assembly", "domain-invalid.pddl", "Arity mismatch for predicate 'available'")]
    [InlineData("barman", "domain-invalid.pddl", "Arity mismatch for fluent 'total-cost'")]
    [InlineData("childsnack", "p01-invalid.pddl", "Undeclared constant or object 'fake-tray'")]
    [InlineData("delivery", "p01-invalid.pddl", "Undeclared constant or object 'fake-truck'")]
    [InlineData("driverlog", "p01-invalid.pddl", "Undeclared constant or object 'fake-truck'")]
    [InlineData("ferry", "p01-invalid.pddl", "Arity mismatch for predicate 'at-ferry'")]
    [InlineData("grid", "p01-invalid.pddl", "Undeclared predicate 'fake-pred'")]
    [InlineData("gripper", "domain-invalid.pddl", "Duplicate parameter '?r' in predicate 'room'")]
    [InlineData("hiking", "domain-invalid.pddl", "Undeclared predicate 'fake-tent'")]
    [InlineData("miconic", "domain-invalid.pddl", "Type mismatch")]
    [InlineData("satellite", "domain-invalid.pddl", "Undeclared variable '?fake'")]
    [InlineData("visitall", "domain-invalid.pddl", "Arity mismatch for predicate 'at-robot'")]
    [InlineData("miconic-fulladl", "domain-invalid.pddl", "Undeclared predicate 'fake-destin'")]
    [InlineData("miconic-simpleadl", "domain-invalid.pddl", "Undeclared predicate 'fake-origin'")]
    [InlineData("refuel", "domain-invalid.pddl", "Undeclared numeric fluent 'fake-fuel'")]
    [InlineData("refuel-adl", "domain-invalid.pddl", "Arity mismatch for fluent 'fuel-level'")]
    [InlineData("zenotravel", "domain-invalid.pddl", "Undeclared variable '?fake'")]
    [InlineData("schedule", "domain-invalid.pddl", "Undeclared variable '?fakebit'")]
    public void Validator_CatchesSemanticErrors_InBenchmarkDomains(string domainDir, string invalidFile, string expectedError)
    {
        var basePath = Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");
        var invalidPath = Path.Combine(basePath, domainDir, invalidFile);

        var input = File.ReadAllText(invalidPath);

        if (invalidFile.StartsWith("domain"))
        {
            var domain = PddlParser.ParseDomain(input);
            var ex = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateDomain(domain));
            Assert.Contains(expectedError, ex.Message);
        }
        else
        {
            var problem = PddlParser.ParseProblem(input);

            var domainFile = Path.Combine(basePath, domainDir, "domain.pddl");
            var domainInput = File.ReadAllText(domainFile);
            var domain = PddlParser.ParseDomain(domainInput);

            var ex = Assert.Throws<PddlValidationException>(() => SemanticValidator.ValidateProblem(domain, problem));
            Assert.Contains(expectedError, ex.Message);
        }
    }
}
