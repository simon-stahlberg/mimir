using System.Text.RegularExpressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;

namespace Mimir.Pddl.Tests.Parsing;

public class RoundTripTests
{
    private static readonly Regex DomainHeader = new(
        @"\(\s*define\s*\(\s*domain\b",
        RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);

    public static IEnumerable<object[]> CheckedInPddlFiles()
    {
        string repositoryRoot = GetRepositoryRoot();
        string[] corpusRoots =
        [
            Path.Combine(repositoryRoot, "Tests", "Examples"),
            Path.Combine(repositoryRoot, "Benchmark")
        ];

        foreach (string corpusRoot in corpusRoots)
        {
            foreach (string path in Directory.EnumerateFiles(corpusRoot, "*.pddl", SearchOption.AllDirectories).Order())
                yield return [Path.GetRelativePath(repositoryRoot, path)];
        }
    }

    [Theory]
    [MemberData(nameof(CheckedInPddlFiles))]
    public void CheckedInPddlFilesSerializeIdempotently(string relativePath)
    {
        string path = Path.Combine(GetRepositoryRoot(), relativePath);
        string input = File.ReadAllText(path);

        if (DomainHeader.IsMatch(input))
        {
            DomainDefinition parsed = PddlParser.ParseDomain(input);
            string serialized = parsed.ToPddlString();
            DomainDefinition reparsed = PddlParser.ParseDomain(serialized);

            Assert.Equal(parsed, reparsed);
            Assert.Equal(serialized, reparsed.ToPddlString());
            return;
        }

        ProblemDefinition parsedProblem = PddlParser.ParseProblem(input);
        string serializedProblem = parsedProblem.ToPddlString();
        ProblemDefinition reparsedProblem = PddlParser.ParseProblem(serializedProblem);

        Assert.Equal(parsedProblem, reparsedProblem);
        Assert.Equal(serializedProblem, reparsedProblem.ToPddlString());
    }

    private static string GetRepositoryRoot()
    {
        return Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../../"));
    }
}
