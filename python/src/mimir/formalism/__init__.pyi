
class Base:
    def get_identifier(self) -> int: ...

class Object(Base):
    pass

class Variable:
    pass

class Type:
    pass

class Parameter:
    pass

class Predicate:
    pass

class TermObject:
    def get_object(self) -> Object: ...

class TermVariable:
    def get_variable(self) -> Variable: ...

class Term:
    def get_object(self) -> TermObject: ...
    def get_variable(self) -> TermVariable: ...

class Requirements:
    pass

class Atom:
    pass

class Literal:
    pass

class GroundAtom:
    pass

class GroundLiteral:
    pass

class NumericFluent:
    pass

class Condition:
    pass

class Effect:
    pass

class FunctionExpression:
    pass

class FunctionSkeleton:
    pass

class Function:
    pass

class OptimizationMetric:
    pass

class PDDLFactories:
    pass

class Action:
    pass

class Domain:
    pass

class Problem:
    pass

class PDDLParser:
    def __init__(self, domain_filepath: str, problem_filepath: str) -> None: ...
    def get_domain(self) -> Domain: ...
    def get_problem(self) -> Problem: ...
    def get_factories(self) -> PDDLFactories: ...

