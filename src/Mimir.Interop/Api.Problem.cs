using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using StaticPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Static>;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;
using DerivedPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Derived>;

namespace Mimir.Interop;

public static partial class Exports
{
    internal static GroundedApplicableActionGenerator GetGroundedGenerator(Problem problem, State state)
    {
        ArgumentNullException.ThrowIfNull(problem);
        ArgumentNullException.ThrowIfNull(state);
        return problem.GetApplicableActionGenerator(state) as GroundedApplicableActionGenerator
            ?? throw new ArgumentException(
                "A grounded RPG heuristic requires a problem configured with the grounded applicable-action generator.",
                nameof(problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_load_problem")]
    public static int LoadProblem(int domainHandle, IntPtr problemPathPtr, IntPtr generatorPtr)
    {
        var domain = ObjectRegistry.Get<Domain>(domainHandle);
        if (domain == null) return 0;
        string? problemPath = ReadUtf8(problemPathPtr);
        string? generator = ReadUtf8(generatorPtr);
        if (problemPath == null || generator == null) return 0;
        return CreateHandle(() => Problem.FromFile(domain, problemPath, ParseGeneratorType(generator)));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_load_problem_from_string")]
    public static int LoadProblemFromString(int domainHandle, IntPtr problemTextPtr, IntPtr generatorPtr)
    {
        var domain = ObjectRegistry.Get<Domain>(domainHandle);
        if (domain == null) return 0;
        string? problemText = ReadUtf8(problemTextPtr);
        string? generator = ReadUtf8(generatorPtr);
        if (problemText == null || generator == null) return 0;
        return CreateHandle(() => Problem.FromText(domain, problemText, ParseGeneratorType(generator)));
    }

    private static ApplicableActionGeneratorType ParseGeneratorType(string generator)
        => generator.ToLowerInvariant() switch
        {
            "grounded" => ApplicableActionGeneratorType.Grounded,
            "lifted" => ApplicableActionGeneratorType.Lifted,
            _ => throw new ArgumentException($"Unknown applicable-action generator '{generator}'.", nameof(generator)),
        };

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_name")]
    public static IntPtr ProblemGetName(int handle)
    {
        var p = ObjectRegistry.Get<Problem>(handle);
        return AllocUtf8(p?.Name);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_domain")]
    public static int ProblemGetDomain(int handle)
    {
        var p = ObjectRegistry.Get<Problem>(handle);
        if (p == null) return 0;
        return ObjectRegistry.Store(p.Domain);
    }

    // -- Objects --

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_object_count")]
    public static int ProblemGetObjectCount(int handle)
        => ReadValue(handle, -1, (Problem problem) => problem.Objects.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_object")]
    public static int ProblemGetObject(int handle, int index)
    {
        var p = ObjectRegistry.Get<Problem>(handle);
        if (p == null || index < 0 || index >= p.Objects.Count) return 0;
        return ObjectRegistry.Store(p.Objects[index]);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_object_by_name")]
    public static int ProblemGetObjectByName(int handle, IntPtr namePtr)
    {
        var p = ObjectRegistry.Get<Problem>(handle);
        if (p == null) return 0;
        string? name = ReadUtf8(namePtr);
        if (name == null) return 0;
        return p.ObjectLookup.TryGetValue(name, out var c) ? ObjectRegistry.Store(c) : 0;
    }

    // -- Initial state --

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_initial_state")]
    public static int ProblemGetInitialState(int handle)
    {
        var p = ObjectRegistry.Get<Problem>(handle);
        if (p == null) return 0;
        return ExpandAndStoreState(p.InitialState);
    }

    // -- Initial atoms (static, fluent, derived) --

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_static_initial_atom_count")]
    public static int ProblemGetStaticInitialAtomCount(int handle)
        => ReadValue(handle, -1, (Problem problem) => BitboardOps.PopCount(problem.Context.StaticBitboardWords));

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_static_initial_atom")]
    public static int ProblemGetStaticInitialAtom(int handle, int index)
    {
        var p = ObjectRegistry.Get<Problem>(handle);
        if (p == null) return 0;
        var ctx = p.Context;
        int localId = BitboardOps.NthSetBitIndex(ctx.StaticBitboardWords, index);
        var staticIndex = new StaticIndex(localId);
        if (!ctx.IsValid(staticIndex)) return 0;
        return ObjectRegistry.Store(ctx.GetFact(staticIndex));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_fluent_initial_atom_count")]
    public static int ProblemGetFluentInitialAtomCount(int handle)
        => ReadValue(handle, -1, (Problem problem) => problem._initialFluentFacts.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_fluent_initial_atom")]
    public static int ProblemGetFluentInitialAtom(int handle, int index)
    {
        var p = ObjectRegistry.Get<Problem>(handle);
        if (p == null) return 0;
        var list = p._initialFluentFacts;
        if (index < 0 || index >= list.Count) return 0;
        return ObjectRegistry.Store(list[index]);
    }

    // -- Goal (returns the goal as a list of ground literals) --

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_goal_literal_count")]
    public static int ProblemGetGoalLiteralCount(int handle)
        => ReadValue(handle, -1, (Problem problem) => problem.Goal.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_goal_literal")]
    public static int ProblemGetGoalLiteral(int handle, int index)
    {
        var p = ObjectRegistry.Get<Problem>(handle);
        if (p == null || index < 0 || index >= p.Goal.Count) return 0;
        return ObjectRegistry.Store(p.Goal[index]);
    }

    // -- Looking up a fact for (predicate, args) --

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_register_fact")]
    public static int ProblemRegisterFact(int problemHandle, int predicateHandle, IntPtr objectHandlesPtr, int objectCount)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        var predicateObj = ObjectRegistry.GetRaw(predicateHandle);
        if (predicateObj == null) return 0;
        var args = ReadHandleArray<Constant>(objectHandlesPtr, objectCount);
        if (args == null) return 0;

        return predicateObj switch
        {
            FluentPredicate pf => CreateHandle(() => problem.Context.RegisterFact(pf, args)),
            StaticPredicate ps => CreateHandle(() => problem.Context.RegisterFact(ps, args)),
            DerivedPredicate pd => CreateHandle(() => problem.Context.RegisterFact(pd, args)),
            _ => 0,
        };
    }

    internal static List<T>? ReadHandleArray<T>(IntPtr handlesPtr, int count) where T : class
    {
        if (count < 0) return null;
        if (count == 0) return new List<T>();
        if (handlesPtr == IntPtr.Zero) return null;
        var result = new List<T>(count);
        unsafe
        {
            int* p = (int*)handlesPtr;
            for (int i = 0; i < count; i++)
            {
                var item = ObjectRegistry.Get<T>(p[i]);
                if (item == null) return null;
                result.Add(item);
            }
        }
        return result;
    }

    internal static List<ITerm>? ReadTermArray(IntPtr handlesPtr, int count)
    {
        if (count < 0) return null;
        if (count == 0) return new List<ITerm>();
        if (handlesPtr == IntPtr.Zero) return null;
        var result = new List<ITerm>(count);
        unsafe
        {
            int* p = (int*)handlesPtr;
            for (int i = 0; i < count; i++)
            {
                var raw = ObjectRegistry.GetRaw(p[i]);
                if (raw is not ITerm term) return null;
                result.Add(term);
            }
        }
        return result;
    }

    // -------- Construction helpers --------

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_new_variable")]
    public static int ProblemNewVariable(int problemHandle, IntPtr namePtr, IntPtr typePtr)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        string? name = ReadUtf8(namePtr);
        string? type = ReadUtf8(typePtr);
        if (name == null || type == null) return 0;
        return CreateHandle(() => problem.NewVariable(name, type));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_new_atom")]
    public static int ProblemNewAtom(int problemHandle, int predicateHandle, IntPtr termHandlesPtr, int termCount)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        var predRaw = ObjectRegistry.GetRaw(predicateHandle);
        var terms = ReadTermArray(termHandlesPtr, termCount);
        if (predRaw == null || terms == null) return 0;
        return predRaw switch
        {
            FluentPredicate pf => CreateHandle(() => problem.NewAtom(pf, terms)),
            StaticPredicate ps => CreateHandle(() => problem.NewAtom(ps, terms)),
            DerivedPredicate pd => CreateHandle(() => problem.NewAtom(pd, terms)),
            _ => 0
        };
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_new_literal")]
    public static int ProblemNewLiteral(int problemHandle, int atomHandle, byte polarity)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null || polarity > 1) return 0;
        var atomRaw = ObjectRegistry.GetRaw(atomHandle);
        bool isPos = polarity == 0;
        return atomRaw switch
        {
            Atom<Fluent> af => CreateHandle(() => problem.NewLiteral(af, isPos)),
            Atom<Static> astatic => CreateHandle(() => problem.NewLiteral(astatic, isPos)),
            Atom<Derived> ad => CreateHandle(() => problem.NewLiteral(ad, isPos)),
            _ => 0
        };
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_new_ground_literal")]
    public static int ProblemNewGroundLiteral(int problemHandle, int factHandle, byte polarity)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null || polarity > 1) return 0;
        var factRaw = ObjectRegistry.GetRaw(factHandle);
        bool isPos = polarity == 0;
        return factRaw switch
        {
            Fact<Fluent> ff => CreateHandle(() => problem.NewGroundLiteral(ff, isPos)),
            Fact<Static> fs => CreateHandle(() => problem.NewGroundLiteral(fs, isPos)),
            Fact<Derived> fd => CreateHandle(() => problem.NewGroundLiteral(fd, isPos)),
            _ => 0
        };
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_new_conjunctive_condition")]
    public static int ProblemNewConjunctiveCondition(
        int problemHandle,
        IntPtr varHandlesPtr, int varCount,
        IntPtr litHandlesPtr, int litCount)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        var variables = ReadHandleArray<Variable>(varHandlesPtr, varCount);
        if (variables == null || litCount < 0) return 0;
        if (litCount > 0 && litHandlesPtr == IntPtr.Zero) return 0;
        var literals = new List<Literal>(litCount);
        unsafe
        {
            int* p = (int*)litHandlesPtr;
            for (int i = 0; i < litCount; i++)
            {
                if (ObjectRegistry.GetRaw(p[i]) is not Literal literal) return 0;
                literals.Add(literal);
            }
        }
        return CreateHandle(() =>
            problem.NewConjunctiveCondition(variables, literals));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_get_generator")]
    public static IntPtr ProblemGetGenerator(int handle)
    {
        var problem = ObjectRegistry.Get<Problem>(handle);
        return AllocUtf8(problem?.GeneratorType switch
        {
            ApplicableActionGeneratorType.Grounded => "grounded",
            ApplicableActionGeneratorType.Lifted => "lifted",
            _ => null,
        });
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_new_ground_action")]
    public static int ProblemNewGroundAction(
        int problemHandle,
        int actionHandle,
        IntPtr objectHandlesPtr,
        int objectCount)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        var schema = ObjectRegistry.Get<ActionSchema>(actionHandle);
        if (problem == null || schema == null) return 0;
        if (!problem.Domain.Actions.Any(action => ReferenceEquals(action, schema))) return 0;

        var objects = ReadHandleArray<Constant>(objectHandlesPtr, objectCount);
        if (objects == null || objects.Count != schema.Parameters.Count) return 0;
        for (int i = 0; i < objects.Count; i++)
        {
            Constant argument = objects[i];
            if (!problem.Objects.Any(candidate => ReferenceEquals(candidate, argument))) return 0;
            if (!problem.Domain.IsCompatible(argument.Type, schema.Parameters[i].Type)) return 0;
        }

        return CreateHandle(() => ActionBuilder.BuildAction(schema, objects, problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_new_state")]
    public static int ProblemNewState(
        int problemHandle,
        IntPtr factHandlesPtr,
        int factCount)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null) return 0;
        var facts = ReadHandleArray<Fact<Fluent>>(factHandlesPtr, factCount);
        if (facts == null) return 0;
        return CreateHandle(() => StateFactory.Default.Create(problem.Context, facts).Expand());
    }
}
