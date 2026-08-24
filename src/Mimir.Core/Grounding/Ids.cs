namespace Mimir.Core.Grounding;

internal readonly record struct FactIndex(int Value);
internal readonly record struct FluentIndex(int Value);
internal readonly record struct StaticIndex(int Value);
internal readonly record struct DerivedIndex(int Value);
internal readonly record struct DerivedEvaluationSlot(int Value);
