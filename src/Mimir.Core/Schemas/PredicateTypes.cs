namespace Mimir.Core.Schemas;

public interface IPredicateType;
public struct Static : IPredicateType;
public struct Fluent : IPredicateType;
public struct Derived : IPredicateType;
