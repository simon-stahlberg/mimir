using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Interop;

public static partial class Exports
{
    // -- Domain builder --

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_builder_create")]
    public static int DomainBuilderCreate(IntPtr namePtr)
        => CreateHandle(() => new DomainBuilder(ReadRequiredUtf8(namePtr, "name")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_builder_requirements")]
    public static int DomainBuilderRequirements(int handle)
        => CreateChild<DomainBuilder>(handle, builder => builder.Requirements());

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_builder_types")]
    public static int DomainBuilderTypes(int handle)
        => CreateChild<DomainBuilder>(handle, builder => builder.Types());

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_builder_constants")]
    public static int DomainBuilderConstants(int handle)
        => CreateChild<DomainBuilder>(handle, builder => builder.Constants());

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_builder_predicates")]
    public static int DomainBuilderPredicates(int handle)
        => CreateChild<DomainBuilder>(handle, builder => builder.Predicates());

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_builder_functions")]
    public static int DomainBuilderFunctions(int handle)
        => CreateChild<DomainBuilder>(handle, builder => builder.Functions());

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_builder_actions")]
    public static int DomainBuilderActions(int handle)
        => CreateChild<DomainBuilder>(handle, builder => builder.Actions());

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_builder_derived_predicates")]
    public static int DomainBuilderDerivedPredicates(int handle)
        => CreateChild<DomainBuilder>(handle, builder => builder.DerivedPredicates());

    [UnmanagedCallersOnly(EntryPoint = "mimir_domain_builder_build")]
    public static int DomainBuilderBuild(int handle)
        => CreateChild<DomainBuilder>(handle, builder => builder.Build());

    // -- Domain sections --

    [UnmanagedCallersOnly(EntryPoint = "mimir_requirement_list_builder_add")]
    public static byte RequirementListBuilderAdd(int handle, IntPtr requirementPtr)
        => Mutate<RequirementListBuilder>(handle, builder =>
            builder.Add(ReadRequiredUtf8(requirementPtr, "requirement")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_requirement_list_builder_close")]
    public static byte RequirementListBuilderClose(int handle)
        => Mutate<RequirementListBuilder>(handle, builder => builder.Close());

    [UnmanagedCallersOnly(EntryPoint = "mimir_type_list_builder_add")]
    public static byte TypeListBuilderAdd(int handle, IntPtr namePtr, IntPtr parentTypePtr)
        => Mutate<TypeListBuilder>(handle, builder => builder.Add(
            ReadRequiredUtf8(namePtr, "name"),
            ReadRequiredUtf8(parentTypePtr, "parent_type")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_type_list_builder_close")]
    public static byte TypeListBuilderClose(int handle)
        => Mutate<TypeListBuilder>(handle, builder => builder.Close());

    [UnmanagedCallersOnly(EntryPoint = "mimir_constant_list_builder_add")]
    public static byte ConstantListBuilderAdd(int handle, IntPtr namePtr, IntPtr typePtr)
        => Mutate<ConstantListBuilder>(handle, builder => builder.Add(
            ReadRequiredUtf8(namePtr, "name"),
            ReadRequiredUtf8(typePtr, "type")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_constant_list_builder_close")]
    public static byte ConstantListBuilderClose(int handle)
        => Mutate<ConstantListBuilder>(handle, builder => builder.Close());

    [UnmanagedCallersOnly(EntryPoint = "mimir_predicate_list_builder_add")]
    public static byte PredicateListBuilderAdd(
        int handle,
        IntPtr namePtr,
        IntPtr parameterNamesPtr,
        IntPtr parameterTypesPtr,
        int parameterCount)
        => Mutate<PredicateListBuilder>(handle, builder => builder.Add(
            ReadRequiredUtf8(namePtr, "name"),
            ReadTypedParameters(
                parameterNamesPtr,
                parameterTypesPtr,
                parameterCount,
                "parameters")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_predicate_list_builder_close")]
    public static byte PredicateListBuilderClose(int handle)
        => Mutate<PredicateListBuilder>(handle, builder => builder.Close());

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_function_list_builder_add")]
    public static byte NumericFunctionListBuilderAdd(
        int handle,
        IntPtr namePtr,
        IntPtr parameterNamesPtr,
        IntPtr parameterTypesPtr,
        int parameterCount)
        => Mutate<NumericFunctionListBuilder>(handle, builder => builder.Add(
            ReadRequiredUtf8(namePtr, "name"),
            ReadTypedParameters(
                parameterNamesPtr,
                parameterTypesPtr,
                parameterCount,
                "parameters")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_function_list_builder_close")]
    public static byte NumericFunctionListBuilderClose(int handle)
        => Mutate<NumericFunctionListBuilder>(handle, builder => builder.Close());

    [UnmanagedCallersOnly(EntryPoint = "mimir_derived_predicate_list_builder_define")]
    public static byte DerivedPredicateListBuilderDefine(
        int handle,
        IntPtr predicateNamePtr,
        int bodyHandle)
        => Mutate<DerivedPredicateListBuilder>(handle, builder => builder.Define(
            ReadRequiredUtf8(predicateNamePtr, "predicate_name"),
            RequireHandle<LogicalExpressionSpec>(bodyHandle)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_derived_predicate_list_builder_close")]
    public static byte DerivedPredicateListBuilderClose(int handle)
        => Mutate<DerivedPredicateListBuilder>(handle, builder => builder.Close());

    // -- Actions and conditional effects --

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_list_builder_add")]
    public static int ActionListBuilderAdd(int handle, IntPtr namePtr)
        => CreateChild<ActionListBuilder>(handle, builder =>
            builder.Add(ReadRequiredUtf8(namePtr, "name")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_list_builder_close")]
    public static byte ActionListBuilderClose(int handle)
        => Mutate<ActionListBuilder>(handle, builder => builder.Close());

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_schema_builder_add_parameter")]
    public static byte ActionSchemaBuilderAddParameter(
        int handle,
        IntPtr namePtr,
        IntPtr typePtr)
        => Mutate<ActionSchemaBuilder>(handle, builder => builder.AddParameter(
            ReadRequiredUtf8(namePtr, "name"),
            ReadRequiredUtf8(typePtr, "type")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_schema_builder_add_precondition")]
    public static byte ActionSchemaBuilderAddPrecondition(
        int handle,
        IntPtr predicateNamePtr,
        byte polarity,
        IntPtr argumentsPtr,
        int argumentCount)
        => Mutate<ActionSchemaBuilder>(handle, builder => builder.AddPrecondition(
            ReadRequiredUtf8(predicateNamePtr, "predicate_name"),
            ParsePolarity(polarity),
            ReadUtf8Array(argumentsPtr, argumentCount, "arguments")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_schema_builder_add_effect")]
    public static byte ActionSchemaBuilderAddEffect(
        int handle,
        IntPtr predicateNamePtr,
        byte polarity,
        IntPtr argumentsPtr,
        int argumentCount)
        => Mutate<ActionSchemaBuilder>(handle, builder => builder.AddEffect(
            ReadRequiredUtf8(predicateNamePtr, "predicate_name"),
            ParsePolarity(polarity),
            ReadUtf8Array(argumentsPtr, argumentCount, "arguments")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_schema_builder_add_conditional_effect")]
    public static int ActionSchemaBuilderAddConditionalEffect(int handle)
        => CreateChild<ActionSchemaBuilder>(handle, builder => builder.AddConditionalEffect());

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_schema_builder_with_constant_cost")]
    public static byte ActionSchemaBuilderWithConstantCost(int handle, double cost)
        => Mutate<ActionSchemaBuilder>(handle, builder => builder.WithCost(cost));

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_schema_builder_with_cost")]
    public static byte ActionSchemaBuilderWithCost(int handle, int costHandle)
        => Mutate<ActionSchemaBuilder>(handle, builder =>
            builder.WithCost(RequireHandle<NumericExpressionSpec>(costHandle)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_schema_builder_close")]
    public static byte ActionSchemaBuilderClose(int handle)
        => Mutate<ActionSchemaBuilder>(handle, builder => builder.Close());

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_builder_add_parameter")]
    public static byte ConditionalEffectBuilderAddParameter(
        int handle,
        IntPtr namePtr,
        IntPtr typePtr)
        => Mutate<ConditionalEffectBuilder>(handle, builder => builder.AddParameter(
            ReadRequiredUtf8(namePtr, "name"),
            ReadRequiredUtf8(typePtr, "type")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_builder_add_condition")]
    public static byte ConditionalEffectBuilderAddCondition(
        int handle,
        IntPtr predicateNamePtr,
        byte polarity,
        IntPtr argumentsPtr,
        int argumentCount)
        => Mutate<ConditionalEffectBuilder>(handle, builder => builder.AddCondition(
            ReadRequiredUtf8(predicateNamePtr, "predicate_name"),
            ParsePolarity(polarity),
            ReadUtf8Array(argumentsPtr, argumentCount, "arguments")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_builder_add_effect")]
    public static byte ConditionalEffectBuilderAddEffect(
        int handle,
        IntPtr predicateNamePtr,
        byte polarity,
        IntPtr argumentsPtr,
        int argumentCount)
        => Mutate<ConditionalEffectBuilder>(handle, builder => builder.AddEffect(
            ReadRequiredUtf8(predicateNamePtr, "predicate_name"),
            ParsePolarity(polarity),
            ReadUtf8Array(argumentsPtr, argumentCount, "arguments")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_builder_close")]
    public static byte ConditionalEffectBuilderClose(int handle)
        => Mutate<ConditionalEffectBuilder>(handle, builder => builder.Close());

    // -- Logical expression specifications --

    [UnmanagedCallersOnly(EntryPoint = "mimir_logic_true")]
    public static int LogicTrue()
        => CreateHandle(() => Logic.True());

    [UnmanagedCallersOnly(EntryPoint = "mimir_logic_false")]
    public static int LogicFalse()
        => CreateHandle(() => Logic.False());

    [UnmanagedCallersOnly(EntryPoint = "mimir_logic_atom")]
    public static int LogicAtom(IntPtr predicateNamePtr, IntPtr argumentsPtr, int argumentCount)
        => CreateHandle(() => Logic.Atom(
            ReadRequiredUtf8(predicateNamePtr, "predicate_name"),
            ReadUtf8Array(argumentsPtr, argumentCount, "arguments")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_logic_equal")]
    public static int LogicEqual(IntPtr leftPtr, IntPtr rightPtr)
        => CreateHandle(() => Logic.Equal(
            ReadRequiredUtf8(leftPtr, "left"),
            ReadRequiredUtf8(rightPtr, "right")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_logic_not")]
    public static int LogicNot(int expressionHandle)
        => CreateHandle(() => Logic.Not(RequireHandle<LogicalExpressionSpec>(expressionHandle)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_logic_and")]
    public static int LogicAnd(IntPtr expressionHandlesPtr, int expressionCount)
        => CreateHandle(() => Logic.And(
            ReadHandleArray<LogicalExpressionSpec>(
                expressionHandlesPtr,
                expressionCount,
                "expressions")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_logic_or")]
    public static int LogicOr(IntPtr expressionHandlesPtr, int expressionCount)
        => CreateHandle(() => Logic.Or(
            ReadHandleArray<LogicalExpressionSpec>(
                expressionHandlesPtr,
                expressionCount,
                "expressions")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_logic_imply")]
    public static int LogicImply(int antecedentHandle, int consequentHandle)
        => CreateHandle(() => Logic.Imply(
            RequireHandle<LogicalExpressionSpec>(antecedentHandle),
            RequireHandle<LogicalExpressionSpec>(consequentHandle)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_logic_exists")]
    public static int LogicExists(
        IntPtr parameterNamesPtr,
        IntPtr parameterTypesPtr,
        int parameterCount,
        int bodyHandle)
        => CreateHandle(() => Logic.Exists(
            ReadTypedParameters(
                parameterNamesPtr,
                parameterTypesPtr,
                parameterCount,
                "parameters"),
            RequireHandle<LogicalExpressionSpec>(bodyHandle)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_logic_forall")]
    public static int LogicForall(
        IntPtr parameterNamesPtr,
        IntPtr parameterTypesPtr,
        int parameterCount,
        int bodyHandle)
        => CreateHandle(() => Logic.Forall(
            ReadTypedParameters(
                parameterNamesPtr,
                parameterTypesPtr,
                parameterCount,
                "parameters"),
            RequireHandle<LogicalExpressionSpec>(bodyHandle)));

    // -- Numeric expression specifications --

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_spec_constant")]
    public static int NumericSpecConstant(double value)
        => CreateHandle(() => Numeric.Constant(value));

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_spec_function")]
    public static int NumericSpecFunction(
        IntPtr functionNamePtr,
        IntPtr argumentsPtr,
        int argumentCount)
        => CreateHandle(() => Numeric.Function(
            ReadRequiredUtf8(functionNamePtr, "function_name"),
            ReadUtf8Array(argumentsPtr, argumentCount, "arguments")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_spec_binary")]
    public static int NumericSpecBinary(int operation, int leftHandle, int rightHandle)
        => CreateHandle(() => Numeric.Binary(
            (NumericOperator)operation,
            RequireHandle<NumericExpressionSpec>(leftHandle),
            RequireHandle<NumericExpressionSpec>(rightHandle)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_numeric_spec_compare")]
    public static int NumericSpecCompare(int leftHandle, int operation, int rightHandle)
        => CreateHandle(() => RequireHandle<NumericExpressionSpec>(leftHandle).Compare(
            (ComparisonOperator)operation,
            RequireHandle<NumericExpressionSpec>(rightHandle)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_schema_builder_add_precondition_expression")]
    public static byte ActionSchemaBuilderAddPreconditionExpression(int handle, int expression)
        => Mutate<ActionSchemaBuilder>(handle, builder => builder.AddPrecondition(RequireHandle<LogicalExpressionSpec>(expression)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_builder_add_condition_expression")]
    public static byte ConditionalEffectBuilderAddConditionExpression(int handle, int expression)
        => Mutate<ConditionalEffectBuilder>(handle, builder => builder.AddCondition(RequireHandle<LogicalExpressionSpec>(expression)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_goal_builder_add_expression")]
    public static byte GoalBuilderAddExpression(int handle, int expression)
        => Mutate<GoalBuilder>(handle, builder => builder.Add(RequireHandle<LogicalExpressionSpec>(expression)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_action_schema_builder_numeric_update")]
    public static byte ActionSchemaBuilderNumericUpdate(int handle, int target, int operation, int expression)
        => Mutate<ActionSchemaBuilder>(handle, builder => builder.NumericUpdate(
            RequireHandle<NumericFunctionSpec>(target), (NumericUpdateOperator)operation, RequireHandle<NumericExpressionSpec>(expression)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_conditional_effect_builder_numeric_update")]
    public static byte ConditionalEffectBuilderNumericUpdate(int handle, int target, int operation, int expression)
        => Mutate<ConditionalEffectBuilder>(handle, builder => builder.NumericUpdate(
            RequireHandle<NumericFunctionSpec>(target), (NumericUpdateOperator)operation, RequireHandle<NumericExpressionSpec>(expression)));

    [UnmanagedCallersOnly(EntryPoint = "mimir_initial_state_builder_set_value")]
    public static byte InitialStateSetValue(int builder, int target, double value)
        => Mutate<InitialStateBuilder>(builder, initial => initial.SetValue(RequireHandle<NumericFunctionSpec>(target), value));

    // -- Problem builder --

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_builder_create")]
    public static int ProblemBuilderCreate(
        int domainHandle,
        IntPtr namePtr,
        IntPtr generatorPtr)
        => CreateHandle(() => new ProblemBuilder(
            RequireHandle<Domain>(domainHandle),
            ReadRequiredUtf8(namePtr, "name"),
            ParseGeneratorType(ReadRequiredUtf8(generatorPtr, "generator"))));

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_builder_objects")]
    public static int ProblemBuilderObjects(int handle)
        => CreateChild<ProblemBuilder>(handle, builder => builder.Objects());

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_builder_initial_state")]
    public static int ProblemBuilderInitialState(int handle)
        => CreateChild<ProblemBuilder>(handle, builder => builder.InitialState());

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_builder_goal")]
    public static int ProblemBuilderGoal(int handle)
        => CreateChild<ProblemBuilder>(handle, builder => builder.Goal());

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_builder_build")]
    public static int ProblemBuilderBuild(int handle)
        => CreateChild<ProblemBuilder>(handle, builder => builder.Build());

    // -- Problem sections --

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_object_list_builder_add")]
    public static byte ProblemObjectListBuilderAdd(
        int handle,
        IntPtr namePtr,
        IntPtr typePtr)
        => Mutate<ProblemObjectListBuilder>(handle, builder => builder.Add(
            ReadRequiredUtf8(namePtr, "name"),
            ReadRequiredUtf8(typePtr, "type")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_problem_object_list_builder_close")]
    public static byte ProblemObjectListBuilderClose(int handle)
        => Mutate<ProblemObjectListBuilder>(handle, builder => builder.Close());

    [UnmanagedCallersOnly(EntryPoint = "mimir_initial_state_builder_add_fact")]
    public static byte InitialStateBuilderAddFact(
        int handle,
        IntPtr predicateNamePtr,
        IntPtr argumentsPtr,
        int argumentCount)
        => Mutate<InitialStateBuilder>(handle, builder => builder.AddFact(
            ReadRequiredUtf8(predicateNamePtr, "predicate_name"),
            ReadUtf8Array(argumentsPtr, argumentCount, "arguments")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_initial_state_builder_close")]
    public static byte InitialStateBuilderClose(int handle)
        => Mutate<InitialStateBuilder>(handle, builder => builder.Close());

    [UnmanagedCallersOnly(EntryPoint = "mimir_goal_builder_add")]
    public static byte GoalBuilderAdd(
        int handle,
        IntPtr predicateNamePtr,
        byte polarity,
        IntPtr argumentsPtr,
        int argumentCount)
        => Mutate<GoalBuilder>(handle, builder => builder.Add(
            ReadRequiredUtf8(predicateNamePtr, "predicate_name"),
            ParsePolarity(polarity),
            ReadUtf8Array(argumentsPtr, argumentCount, "arguments")));

    [UnmanagedCallersOnly(EntryPoint = "mimir_goal_builder_close")]
    public static byte GoalBuilderClose(int handle)
        => Mutate<GoalBuilder>(handle, builder => builder.Close());

    private static int CreateChild<T>(int handle, Func<T, object> create)
        where T : class
        => ReadValue<T, int>(handle, 0, value => ObjectRegistry.Store(create(value)));

    private static byte Mutate<T>(int handle, System.Action<T> mutate)
        where T : class
        => ReadValue<T, byte>(handle, 0, value =>
        {
            mutate(value);
            return 1;
        });

    private static string ReadRequiredUtf8(IntPtr pointer, string parameterName)
        => ReadUtf8(pointer) ?? throw new ArgumentNullException(parameterName);

    private static unsafe string[] ReadUtf8Array(
        IntPtr pointer,
        int count,
        string parameterName)
    {
        if (count < 0) throw new ArgumentOutOfRangeException(parameterName, count, "Count cannot be negative.");
        if (count == 0) return Array.Empty<string>();
        if (pointer == IntPtr.Zero) throw new ArgumentNullException(parameterName);

        var result = new string[count];
        var values = (IntPtr*)pointer;
        for (int index = 0; index < count; index++)
        {
            result[index] = ReadUtf8(values[index])
                ?? throw new ArgumentException(
                    $"{parameterName} cannot contain null values.",
                    parameterName);
        }

        return result;
    }

    private static (string Name, string Type)[] ReadTypedParameters(
        IntPtr namesPtr,
        IntPtr typesPtr,
        int count,
        string parameterName)
    {
        string[] names = ReadUtf8Array(namesPtr, count, parameterName);
        string[] types = ReadUtf8Array(typesPtr, count, parameterName);
        var result = new (string Name, string Type)[count];
        for (int index = 0; index < count; index++)
            result[index] = (names[index], types[index]);
        return result;
    }

    private static unsafe T[] ReadHandleArray<T>(
        IntPtr pointer,
        int count,
        string parameterName)
        where T : class
    {
        if (count < 0) throw new ArgumentOutOfRangeException(parameterName, count, "Count cannot be negative.");
        if (count == 0) return Array.Empty<T>();
        if (pointer == IntPtr.Zero) throw new ArgumentNullException(parameterName);

        var result = new T[count];
        var handles = (int*)pointer;
        for (int index = 0; index < count; index++)
            result[index] = RequireHandle<T>(handles[index]);
        return result;
    }

    private static Polarity ParsePolarity(byte polarity)
        => polarity switch
        {
            0 => Polarity.Positive,
            1 => Polarity.Negative,
            _ => throw new ArgumentOutOfRangeException(
                nameof(polarity),
                polarity,
                "Polarity must be 0 (positive) or 1 (negative)."),
        };
}
