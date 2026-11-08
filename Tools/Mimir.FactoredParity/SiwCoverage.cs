using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Evaluation;
using Mimir.Search.Planning;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.FactoredParity;

/// <summary>Compares iterated width against its serialized form, and records the width each round settled on.</summary>
internal static class SiwCoverage
{
    private readonly record struct Measurement(bool Solved, double Seconds, int Expanded, int Generated, int PlanLength);

    private sealed class Tally
    {
        public int Solved;
        public int Invalid;
        public int Unsupported;
        public double Seconds;
        public readonly SortedDictionary<int, int> RoundWidths = new();
    }

    /// <summary>
    /// Runs serialized iterated width over the benchmark once per generator. The grounded generator shares no
    /// code with the factored one, so agreeing with it is a check on the factored path rather than a restatement
    /// of it; differing coverage is expected, since each generator breaks ties its own way.
    /// </summary>
    public static void Compare(string benchmarkRoot, int seconds, int maximumWidth, string generatorCodes, int instancesPerDomain)
    {
        ApplicableActionGeneratorType[] generators = ParseGenerators(generatorCodes);

        var tallies = generators.ToDictionary(generator => generator, _ => new Tally());
        var solvedBy = new Dictionary<string, HashSet<ApplicableActionGeneratorType>>(StringComparer.Ordinal);
        var perInstance = new Dictionary<(string Instance, ApplicableActionGeneratorType Generator), Measurement>();
        int instances = 0;

        foreach ((string domainPath, string problemPath) in Discover(benchmarkRoot, instancesPerDomain))
        {
            Domain domain;
            try
            {
                domain = Domain.FromFile(domainPath);
            }
            catch (Exception exception) when (exception is Mimir.Pddl.Analysis.PddlValidationException or PddlLoadException)
            {
                continue;
            }

            string name = Path.GetRelativePath(benchmarkRoot, problemPath);
            solvedBy[name] = new HashSet<ApplicableActionGeneratorType>();
            instances++;

            foreach (ApplicableActionGeneratorType generator in generators)
            {
                Tally tally = tallies[generator];
                Problem problem;
                try
                {
                    problem = Problem.FromFile(domain, problemPath, generator);
                }
                catch (Exception exception) when (exception is NotSupportedException
                                                      or Mimir.Pddl.Analysis.PddlValidationException or PddlLoadException)
                {
                    tally.Unsupported++;
                    continue;
                }

                var goal = GoalCondition.FromProblem(problem);
                var planner = new SerializedIteratedWidthPlanner(maximumWidth);
                var widths = new List<int>();
                planner.RoundCompleted += widths.Add;

                // Printed as it goes: grounding a large instance can exhaust memory and take the process with
                // it, and a run that dies should still leave behind what it measured.
                Measurement run = Solve(planner, problem, goal, seconds, _ => tally.Invalid++);
                tally.Seconds += run.Seconds;
                perInstance[(name, generator)] = run;
                Console.WriteLine(
                    $"{name,-46}{generator,-10}{(run.Solved ? "ok" : "-"),4}{run.Seconds,9:F2}"
                    + $"{run.Expanded,10}{run.Generated,12}{run.PlanLength,7}");
                Console.Out.Flush();
                if (!run.Solved) continue;

                tally.Solved++;
                solvedBy[name].Add(generator);
                foreach (int width in widths)
                    tally.RoundWidths[width] = tally.RoundWidths.GetValueOrDefault(width) + 1;
            }
        }

        foreach ((string name, HashSet<ApplicableActionGeneratorType> solvers) in solvedBy)
        {
            if (solvers.Count is 0 || solvers.Count == generators.Length) continue;
            Console.WriteLine($"{name,-50}solved by: {string.Join(", ", solvers.Order())}");
        }

        // Only instances every generator solved can be timed against each other; elsewhere the clock is
        // measuring a timeout rather than the work.
        string[] common = solvedBy
            .Where(entry => entry.Value.Count == generators.Length)
            .Select(entry => entry.Key)
            .Order(StringComparer.Ordinal)
            .ToArray();

        Console.WriteLine();
        Console.WriteLine($"Instances solved by every generator: {common.Length}");
        foreach (ApplicableActionGeneratorType generator in generators)
        {
            double total = common.Sum(instance => perInstance[(instance, generator)].Seconds);
            long generated = common.Sum(instance => (long)perInstance[(instance, generator)].Generated);
            Console.WriteLine($"  {generator,-12} common total: {total,8:F2}s   generated: {generated,12}");
        }

        Console.WriteLine();
        Console.WriteLine($"instances : {instances}");
        Console.WriteLine($"{"generator",-12}{"solved",8}{"invalid",9}{"unsupported",13}{"seconds",10}  rounds by width");
        foreach (ApplicableActionGeneratorType generator in generators)
        {
            Tally tally = tallies[generator];
            string widths = string.Join(", ", tally.RoundWidths.Select(entry => $"k={entry.Key}: {entry.Value}"));
            Console.WriteLine(
                $"{generator,-12}{tally.Solved,8}{tally.Invalid,9}{tally.Unsupported,13}{tally.Seconds,10:F1}  {widths}");
        }
    }

    private static ApplicableActionGeneratorType[] ParseGenerators(string codes)
        => codes.Select(code => code switch
        {
            'g' => ApplicableActionGeneratorType.Grounded,
            'l' => ApplicableActionGeneratorType.Lifted,
            'f' => ApplicableActionGeneratorType.Factored,
            _ => throw new ArgumentException($"Unknown generator code '{code}'.", nameof(codes)),
        }).ToArray();

    /// <summary>
    /// Runs every instance of a single domain in name order, which for the usual naming is also size order, so the
    /// table shows how each generator degrades as instances grow rather than how it does on one sample.
    /// </summary>
    public static void Scale(string domainDirectory, int seconds, int maximumWidth, string generatorCodes)
    {
        ApplicableActionGeneratorType[] generators = ParseGenerators(generatorCodes);
        string domainPath = Path.Combine(domainDirectory, "domain.pddl");
        var domain = Domain.FromFile(domainPath);
        string label = Path.GetFileName(Path.TrimEndingDirectorySeparator(domainDirectory));

        Console.WriteLine($"# {label}  (SIW, kmax={maximumWidth}, {seconds}s cap)");
        Console.WriteLine($"{"instance",-34}{"generator",-10}{"ok",4}{"seconds",9}{"expanded",10}{"generated",12}{"plan",7}");

        foreach (string problemPath in Directory
                     .EnumerateFiles(domainDirectory, "*.pddl")
                     .Where(path => !string.Equals(Path.GetFileName(path), "domain.pddl", StringComparison.OrdinalIgnoreCase))
                     .OrderBy(path => path, StringComparer.Ordinal))
        {
            string name = Path.GetFileNameWithoutExtension(problemPath);
            foreach (ApplicableActionGeneratorType generator in generators)
            {
                Problem problem;
                try
                {
                    problem = Problem.FromFile(domain, problemPath, generator);
                }
                catch (Exception exception) when (exception is NotSupportedException
                                                      or Mimir.Pddl.Analysis.PddlValidationException or PddlLoadException)
                {
                    Console.WriteLine($"{name,-34}{generator,-10}{"n/a",4}  {Shorten(exception.Message)}");
                    continue;
                }

                var goal = GoalCondition.FromProblem(problem);
                int invalid = 0;
                Measurement run = Solve(
                    new SerializedIteratedWidthPlanner(maximumWidth), problem, goal, seconds, _ => invalid++);
                Console.WriteLine(
                    $"{name,-34}{generator,-10}{(run.Solved ? "ok" : invalid > 0 ? "BAD" : "-"),4}{run.Seconds,9:F2}"
                    + $"{run.Expanded,10}{run.Generated,12}{run.PlanLength,7}");
                Console.Out.Flush();
            }
        }
    }

    /// <summary>
    /// One instance, one generator, one line of CSV. Running each measurement as its own process is what lets the
    /// whole-process wall clock be compared against an external planner on equal terms, grounding included.
    /// </summary>
    public static void One(
        string domainPath, string problemPath, int seconds, int maximumWidth, string generatorCode, string detector)
    {
        var total = Stopwatch.StartNew();
        ApplicableActionGeneratorType generator = ParseGenerators(generatorCode)[0];
        var domain = Domain.FromFile(domainPath);
        Problem problem = Problem.FromFile(domain, problemPath, generator);
        var goal = GoalCondition.FromProblem(problem);

        IDeadEndDetector? deadEndDetector = detector switch
        {
            "none" => null,
            "ff" => new FfDeadEndDetector(problem),
            "h2" => new H2DeadEndDetector(problem),
            _ => throw new ArgumentException($"Unknown detector '{detector}'.", nameof(detector)),
        };

        int invalid = 0;
        var planner = new SerializedIteratedWidthPlanner(maximumWidth);
        var widths = new List<int>();
        planner.RoundCompleted += widths.Add;
        Measurement run = Solve(planner, problem, goal, seconds, _ => invalid++, deadEndDetector);
        total.Stop();

        string status = run.Solved ? "ok" : invalid > 0 ? "invalid" : "unsolved";
        Console.WriteLine(string.Create(
            System.Globalization.CultureInfo.InvariantCulture,
            $"RESULT,{status},{total.Elapsed.TotalSeconds:F3},{run.Seconds:F3},"
            + $"{run.Expanded},{run.Generated},{run.PlanLength},"
            + $"{widths.Count},{goal.GoalLiterals.Count}"));
    }

    private static string Shorten(string message)
    {
        int stop = message.IndexOf(';');
        return stop < 0 ? message : message[..stop];
    }

    public static void Run(string benchmarkRoot, int seconds, int maximumWidth)
    {
        int instances = 0;
        int iwSolved = 0;
        int siwSolved = 0;
        int invalid = 0;
        double iwSeconds = 0;
        double siwSeconds = 0;
        var roundWidths = new SortedDictionary<int, int>();

        foreach ((string domainPath, string problemPath) in Discover(benchmarkRoot, 1))
        {
            Domain domain;
            Problem problem;
            try
            {
                domain = Domain.FromFile(domainPath);
                problem = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Factored);
            }
            catch (Exception exception) when (exception is NotSupportedException
                                                  or Mimir.Pddl.Analysis.PddlValidationException or PddlLoadException)
            {
                continue;
            }

            instances++;
            var goal = GoalCondition.FromProblem(problem);

            Measurement iw = Solve(new IteratedWidthPlanner(maximumWidth), problem, goal, seconds, null);
            var planner = new SerializedIteratedWidthPlanner(maximumWidth);
            var widths = new List<int>();
            planner.RoundCompleted += widths.Add;
            Measurement siw = Solve(planner, problem, goal, seconds, w => invalid += w);
            bool iwOk = iw.Solved;
            bool siwOk = siw.Solved;

            if (iwOk) iwSolved++;
            if (siwOk)
            {
                siwSolved++;
                foreach (int width in widths)
                    roundWidths[width] = roundWidths.GetValueOrDefault(width) + 1;
            }

            iwSeconds += iw.Seconds;
            siwSeconds += siw.Seconds;
            string name = Path.GetRelativePath(benchmarkRoot, problemPath);
            if (iwOk != siwOk)
                Console.WriteLine($"{name,-50}{(iwOk ? "IW only" : "SIW only")}");
        }

        Console.WriteLine();
        Console.WriteLine($"instances        : {instances}");
        Console.WriteLine($"IW({maximumWidth}) solved     : {iwSolved}");
        Console.WriteLine($"SIW({maximumWidth}) solved    : {siwSolved}");
        Console.WriteLine($"invalid plans    : {invalid}");
        Console.WriteLine($"IW seconds       : {iwSeconds:F1}");
        Console.WriteLine($"SIW seconds      : {siwSeconds:F1}");
        Console.WriteLine("rounds by width  : " + string.Join(
            ", ", roundWidths.Select(entry => $"k={entry.Key}: {entry.Value}")));
    }

    private static Measurement Solve(
        IPlanner planner,
        Problem problem,
        GoalCondition goal,
        int seconds,
        System.Action<int>? reportInvalid,
        IDeadEndDetector? deadEndDetector = null)
    {
        using var cancellation = new CancellationTokenSource(TimeSpan.FromSeconds(seconds));
        var stopwatch = Stopwatch.StartNew();
        PlanResult result = planner.Solve(
            problem, cancellationToken: cancellation.Token, deadEndDetector: deadEndDetector);
        stopwatch.Stop();

        var measured = new Measurement(
            false,
            stopwatch.Elapsed.TotalSeconds,
            result.Statistics.NodesExpanded,
            result.Statistics.NodesGenerated,
            result.PlanLength);
        if (!result.IsSuccess) return measured;

        // A reported plan that does not reach the goal would be worse than no plan at all.
        Mimir.Core.Grounding.State state = problem.InitialState;
        foreach (GroundAction action in result.Plan)
        {
            if (!action.IsApplicable(state.Expand())) { reportInvalid?.Invoke(1); return measured; }
            state = state.Expand().Apply(action);
        }

        if (!goal.IsSatisfied(state.Expand()))
        {
            reportInvalid?.Invoke(1);
            return measured;
        }

        return measured with { Solved = true };
    }

    private static IEnumerable<(string DomainPath, string ProblemPath)> Discover(string root, int instancesPerDomain)
    {
        foreach (string category in new[] { "strips", "adl" })
        {
            string categoryPath = Path.Combine(root, category);
            if (!Directory.Exists(categoryPath)) continue;

            foreach (string directory in Directory.EnumerateDirectories(categoryPath).OrderBy(p => p, StringComparer.Ordinal))
            {
                string domainPath = Path.Combine(directory, "domain.pddl");
                if (!File.Exists(domainPath)) continue;

                // The largest instances are the ones worth timing: the factored path exists for the regime where
                // the number of applicable ground actions explodes, and the smallest instance of a domain never
                // reaches it. File size stands in for instance size.
                string[] problems = Directory
                    .EnumerateFiles(directory, "*.pddl", SearchOption.TopDirectoryOnly)
                    .Where(p => !string.Equals(p, domainPath, StringComparison.OrdinalIgnoreCase))
                    .Where(p => !p.EndsWith("-domain.pddl", StringComparison.OrdinalIgnoreCase))
                    .OrderByDescending(p => new FileInfo(p).Length)
                    .ThenBy(p => p, StringComparer.Ordinal)
                    .Take(instancesPerDomain)
                    .OrderBy(p => p, StringComparer.Ordinal)
                    .ToArray();

                foreach (string problemPath in problems)
                {
                    string paired = Path.Combine(directory, $"{Path.GetFileNameWithoutExtension(problemPath)}-domain.pddl");
                    yield return (File.Exists(paired) ? paired : domainPath, problemPath);
                }
            }
        }
    }
}
