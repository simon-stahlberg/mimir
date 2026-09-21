from __future__ import annotations

import math
from dataclasses import dataclass
from enum import IntEnum
from functools import cached_property

from .advanced import numeric as _bindings
from .advanced._native import lib, raise_last_error
from .model import Domain, Problem, Object, Variable, Term, _Handle, _required_string, _wrap_term


class NumericOperator(IntEnum):
    ADD = 0
    SUBTRACT = 1
    MULTIPLY = 2
    DIVIDE = 3


class ComparisonOperator(IntEnum):
    EQUAL = 0
    LESS_THAN = 1
    LESS_THAN_OR_EQUAL = 2
    GREATER_THAN = 3
    GREATER_THAN_OR_EQUAL = 4


class NumericUpdateOperator(IntEnum):
    ASSIGN = 0
    INCREASE = 1
    DECREASE = 2


class NumericFunction(_Handle):
    @property
    def name(self) -> str:
        return _required_string(lib.mimir_function_get_name(self._handle), "NumericFunction.name")

    @cached_property
    def parameters(self) -> tuple[Variable, ...]:
        return tuple(Variable._from_handle(lib.mimir_function_get_parameter(self._handle, index), self._owner)
                     for index in range(lib.mimir_function_get_parameter_count(self._handle)))

    @property
    def arity(self) -> int:
        return len(self.parameters)

    def __str__(self) -> str:
        return self.name


class NumericExpression(_Handle):
    @property
    def is_ground(self) -> bool:
        if isinstance(self, (NumericConstant, GroundFunctionCall)):
            return True
        if isinstance(self, NumericBinaryExpression):
            return self.left.is_ground and self.right.is_ground
        return False

    def _binary(self, operation: NumericOperator, other: NumericExpression | float) -> NumericExpression:
        right = _expression(other)
        if self._owner is not None and right._owner is not None and self._owner != right._owner:
            raise ValueError("numeric expressions must have the same owner")
        handle = int(lib.mimir_numeric_binary(operation, self._handle, right._handle))
        return _wrap_numeric(handle, self._owner if self._owner is not None else right._owner)

    def __add__(self, other: NumericExpression | float) -> NumericExpression:
        return self._binary(NumericOperator.ADD, other)

    def __sub__(self, other: NumericExpression | float) -> NumericExpression:
        return self._binary(NumericOperator.SUBTRACT, other)

    def __mul__(self, other: NumericExpression | float) -> NumericExpression:
        return self._binary(NumericOperator.MULTIPLY, other)

    def __truediv__(self, other: NumericExpression | float) -> NumericExpression:
        return self._binary(NumericOperator.DIVIDE, other)

    def __radd__(self, other: float) -> NumericExpression:
        return _expression(other) + self

    def __rsub__(self, other: float) -> NumericExpression:
        return _expression(other) - self

    def __rmul__(self, other: float) -> NumericExpression:
        return _expression(other) * self

    def __rtruediv__(self, other: float) -> NumericExpression:
        return _expression(other) / self

    def __neg__(self) -> NumericExpression:
        return NumericConstant(0) - self

    def _compare(self, operation: ComparisonOperator, other: NumericExpression | float) -> NumericComparison:
        right = _expression(other)
        if self._owner is not None and right._owner is not None and self._owner != right._owner:
            raise ValueError("numeric expressions must have the same owner")
        comparison_type = GroundNumericComparison if self.is_ground and right.is_ground else NumericComparison
        return comparison_type(self, operation, right)

    def equal_to(self, other: NumericExpression | float) -> NumericComparison:
        return self._compare(ComparisonOperator.EQUAL, other)

    def less_than(self, other: NumericExpression | float) -> NumericComparison:
        return self._compare(ComparisonOperator.LESS_THAN, other)

    def less_than_or_equal(self, other: NumericExpression | float) -> NumericComparison:
        return self._compare(ComparisonOperator.LESS_THAN_OR_EQUAL, other)

    def greater_than(self, other: NumericExpression | float) -> NumericComparison:
        return self._compare(ComparisonOperator.GREATER_THAN, other)

    def greater_than_or_equal(self, other: NumericExpression | float) -> NumericComparison:
        return self._compare(ComparisonOperator.GREATER_THAN_OR_EQUAL, other)


class NumericConstant(NumericExpression):
    def __init__(self, value: float) -> None:
        if isinstance(value, bool) or not isinstance(value, (int, float)):
            raise TypeError("numeric constants require an int or float")
        if not math.isfinite(value):
            raise ValueError("numeric constants must be finite")
        handle = int(lib.mimir_numeric_constant(value))
        if handle == 0:
            raise_last_error("could not create numeric constant")
        self._initialize(handle, None)

    @property
    def value(self) -> float:
        return float(lib.mimir_numeric_constant_value(self._handle))

    def __str__(self) -> str:
        return str(self.value)


class FunctionCall(NumericExpression):
    @cached_property
    def function(self) -> NumericFunction:
        owner = self._owner.domain if isinstance(self._owner, Problem) else self._owner
        return NumericFunction._from_handle(lib.mimir_numeric_function(self._handle), owner)

    @cached_property
    def arguments(self) -> tuple[Term, ...]:
        return tuple(_wrap_term(lib.mimir_numeric_argument(self._handle, index), self._owner)
                     for index in range(lib.mimir_numeric_argument_count(self._handle)))

    def __str__(self) -> str:
        return f"({self.function.name}{''.join(' ' + str(argument) for argument in self.arguments)})"


class GroundFunctionCall(NumericExpression):
    @property
    def problem(self) -> Problem:
        if not isinstance(self._owner, Problem):
            raise RuntimeError("ground function call has no problem")
        return self._owner

    @cached_property
    def function(self) -> NumericFunction:
        return NumericFunction._from_handle(lib.mimir_numeric_function(self._handle), self.problem.domain)

    @cached_property
    def arguments(self) -> tuple[Object, ...]:
        return tuple(Object._from_handle(lib.mimir_numeric_argument(self._handle, index), self.problem)
                     for index in range(lib.mimir_numeric_argument_count(self._handle)))

    def __str__(self) -> str:
        return f"({self.function.name}{''.join(' ' + str(argument) for argument in self.arguments)})"


class NumericBinaryExpression(NumericExpression):
    @property
    def operator(self) -> NumericOperator:
        return NumericOperator(lib.mimir_numeric_operator(self._handle))

    @cached_property
    def left(self) -> NumericExpression:
        return _wrap_numeric(lib.mimir_numeric_operand(self._handle, 0), self._owner)

    @cached_property
    def right(self) -> NumericExpression:
        return _wrap_numeric(lib.mimir_numeric_operand(self._handle, 1), self._owner)

    def __str__(self) -> str:
        symbol = {NumericOperator.ADD: "+", NumericOperator.SUBTRACT: "-", NumericOperator.MULTIPLY: "*", NumericOperator.DIVIDE: "/"}[self.operator]
        return f"({symbol} {self.left} {self.right})"


@dataclass(frozen=True)
class NumericComparison:
    left: NumericExpression
    operator: ComparisonOperator
    right: NumericExpression

    def __post_init__(self) -> None:
        if not isinstance(self.left, NumericExpression) or not isinstance(self.right, NumericExpression):
            raise TypeError("comparison operands must be numeric expressions")
        if not isinstance(self.operator, ComparisonOperator):
            raise TypeError("operator must be a ComparisonOperator")

    def __bool__(self) -> bool:
        raise TypeError("evaluate comparisons with state.holds(comparison)")


@dataclass(frozen=True)
class GroundNumericComparison(NumericComparison):
    def __post_init__(self) -> None:
        super().__post_init__()
        if not self.left.is_ground or not self.right.is_ground:
            raise ValueError("ground comparisons cannot contain variables")


@dataclass(frozen=True)
class NumericUpdate:
    target: FunctionCall
    operator: NumericUpdateOperator
    expression: NumericExpression

    def __post_init__(self) -> None:
        if not isinstance(self.target, FunctionCall):
            raise TypeError("target must be a FunctionCall")
        if not isinstance(self.operator, NumericUpdateOperator) or not isinstance(self.expression, NumericExpression):
            raise TypeError("numeric updates require an operator and numeric expression")


@dataclass(frozen=True)
class GroundNumericUpdate:
    target: GroundFunctionCall
    operator: NumericUpdateOperator
    expression: NumericExpression

    def __post_init__(self) -> None:
        if not isinstance(self.target, GroundFunctionCall):
            raise TypeError("target must be a GroundFunctionCall")
        if not isinstance(self.operator, NumericUpdateOperator) or not isinstance(self.expression, NumericExpression):
            raise TypeError("numeric updates require an operator and numeric expression")
        if not self.expression.is_ground:
            raise ValueError("ground updates cannot contain variables")


def _expression(value: NumericExpression | float) -> NumericExpression:
    return value if isinstance(value, NumericExpression) else NumericConstant(value)


def _wrap_numeric(handle: int, owner: Domain | Problem | None) -> NumericExpression:
    if not handle:
        raise_last_error("could not create numeric expression")
    kind = int(lib.mimir_numeric_kind(handle))
    classes: dict[int, type[NumericExpression]] = {
        0: NumericConstant, 1: FunctionCall, 2: NumericBinaryExpression, 3: GroundFunctionCall,
    }
    return classes[kind]._from_handle(handle, None if kind == 0 else owner)
