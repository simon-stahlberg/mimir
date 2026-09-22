from __future__ import annotations

from enum import IntEnum
from functools import cached_property
from typing import TYPE_CHECKING, cast

from ._base import Object, Term, Variable, _Handle, _required_string, _wrap_term
from .advanced._native import lib

if TYPE_CHECKING:
    from .model import Domain, Problem


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
    SCALE_UP = 3
    SCALE_DOWN = 4


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
        return bool(lib.mimir_numeric_is_ground(self._handle))

    def _binary(self, operation: NumericOperator, other: NumericExpression | float) -> NumericExpression:
        right = _expression(other)
        handle = int(lib.mimir_numeric_binary(operation, self._handle, right._handle))
        return _wrap_numeric(handle, _shared_owner(self, right))

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
        handle = int(lib.mimir_numeric_comparison_create(self._handle, operation, right._handle))
        return _wrap_comparison(handle, _shared_owner(self, right))

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
        self._initialize(int(lib.mimir_numeric_constant(value)), None)

    @property
    def value(self) -> float:
        return float(lib.mimir_numeric_constant_value(self._handle))

    def __str__(self) -> str:
        return str(self.value)


def _call_string(function: NumericFunction, arguments: tuple[Term, ...] | tuple[Object, ...]) -> str:
    return f"({function.name}{''.join(' ' + str(argument) for argument in arguments)})"


class FunctionCall(NumericExpression):
    @cached_property
    def function(self) -> NumericFunction:
        if self._owner is None:
            raise RuntimeError("function call has no owner")
        return NumericFunction._from_handle(lib.mimir_numeric_function(self._handle), self._owner._schema_domain)

    @cached_property
    def arguments(self) -> tuple[Term, ...]:
        return tuple(_wrap_term(lib.mimir_numeric_argument(self._handle, index), self._owner)
                     for index in range(lib.mimir_numeric_argument_count(self._handle)))

    def __str__(self) -> str:
        return _call_string(self.function, self.arguments)


class GroundFunctionCall(NumericExpression):
    @property
    def problem(self) -> Problem:
        if self._owner is None:
            raise RuntimeError("ground function call has no problem")
        # Ground function calls are only created through a Problem.
        return cast("Problem", self._owner)

    @cached_property
    def function(self) -> NumericFunction:
        return NumericFunction._from_handle(lib.mimir_numeric_function(self._handle), self.problem.domain)

    @cached_property
    def arguments(self) -> tuple[Object, ...]:
        return tuple(Object._from_handle(lib.mimir_numeric_argument(self._handle, index), self.problem)
                     for index in range(lib.mimir_numeric_argument_count(self._handle)))

    def __str__(self) -> str:
        return _call_string(self.function, self.arguments)


_OPERATOR_SYMBOLS = {
    NumericOperator.ADD: "+", NumericOperator.SUBTRACT: "-", NumericOperator.MULTIPLY: "*", NumericOperator.DIVIDE: "/",
}
_COMPARISON_SYMBOLS = {
    ComparisonOperator.EQUAL: "=", ComparisonOperator.LESS_THAN: "<", ComparisonOperator.LESS_THAN_OR_EQUAL: "<=",
    ComparisonOperator.GREATER_THAN: ">", ComparisonOperator.GREATER_THAN_OR_EQUAL: ">=",
}


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
        return f"({_OPERATOR_SYMBOLS[self.operator]} {self.left} {self.right})"


class NumericComparison(_Handle):
    """A comparison between two numeric expressions; create it with ``expression.less_than(...)`` etc."""

    @cached_property
    def left(self) -> NumericExpression:
        return _wrap_numeric(lib.mimir_numeric_comparison_operand(self._handle, 0), self._owner)

    @cached_property
    def right(self) -> NumericExpression:
        return _wrap_numeric(lib.mimir_numeric_comparison_operand(self._handle, 1), self._owner)

    @property
    def operator(self) -> ComparisonOperator:
        return ComparisonOperator(lib.mimir_numeric_comparison_operator(self._handle))

    def __bool__(self) -> bool:
        raise TypeError("evaluate comparisons with state.holds(comparison)")

    def __str__(self) -> str:
        return f"({_COMPARISON_SYMBOLS[self.operator]} {self.left} {self.right})"


class GroundNumericComparison(NumericComparison):
    """A comparison whose operands contain no variables."""


class _NumericUpdateBase(_Handle):
    @property
    def operator(self) -> NumericUpdateOperator:
        return NumericUpdateOperator(lib.mimir_numeric_update_operator(self._handle))

    @cached_property
    def expression(self) -> NumericExpression:
        return _wrap_numeric(lib.mimir_numeric_update_operand(self._handle, 1), self._owner)

    def __str__(self) -> str:
        return f"({self.operator.name.lower().replace('_', '-')} {self._target_expression()} {self.expression})"

    def _target_expression(self) -> NumericExpression:
        return _wrap_numeric(lib.mimir_numeric_update_operand(self._handle, 0), self._owner)


class NumericUpdate(_NumericUpdateBase):
    @cached_property
    def target(self) -> FunctionCall:
        target = self._target_expression()
        if not isinstance(target, FunctionCall):
            raise RuntimeError("numeric update target is not a function call")
        return target


class GroundNumericUpdate(_NumericUpdateBase):
    @cached_property
    def target(self) -> GroundFunctionCall:
        target = self._target_expression()
        if not isinstance(target, GroundFunctionCall):
            raise RuntimeError("ground numeric update target is not a ground function call")
        return target


def _expression(value: NumericExpression | float) -> NumericExpression:
    return value if isinstance(value, NumericExpression) else NumericConstant(value)


def _shared_owner(left: _Handle, right: _Handle) -> Domain | Problem | None:
    if left._owner is not None and right._owner is not None and left._owner != right._owner:
        raise ValueError("numeric expressions must have the same owner")
    return left._owner if left._owner is not None else right._owner


_EXPRESSION_KINDS: dict[int, type[NumericExpression]] = {
    0: NumericConstant, 1: FunctionCall, 2: NumericBinaryExpression, 3: GroundFunctionCall,
}


def _wrap_numeric(handle: int, owner: Domain | Problem | None) -> NumericExpression:
    kind = int(lib.mimir_numeric_kind(handle))
    return _EXPRESSION_KINDS[kind]._from_handle(handle, None if kind == 0 else owner)


def _wrap_comparison(handle: int, owner: Domain | Problem | None) -> NumericComparison:
    cls = GroundNumericComparison if lib.mimir_numeric_is_ground(handle) else NumericComparison
    return cls._from_handle(handle, owner)


def _read_comparisons(handle: int, owner: Domain | Problem) -> tuple[NumericComparison, ...]:
    return tuple(_wrap_comparison(lib.mimir_numeric_comparison_get(handle, index), owner)
                 for index in range(lib.mimir_numeric_comparison_count(handle)))


def _read_updates(handle: int, owner: Domain | Problem) -> tuple[NumericUpdate | GroundNumericUpdate, ...]:
    updates: list[NumericUpdate | GroundNumericUpdate] = []
    for index in range(lib.mimir_numeric_update_count(handle)):
        item = lib.mimir_numeric_update_get(handle, index)
        if lib.mimir_numeric_is_ground(item):
            updates.append(GroundNumericUpdate._from_handle(item, owner))
        else:
            updates.append(NumericUpdate._from_handle(item, owner))
    return tuple(updates)


def _comparison_handles(comparisons: tuple[NumericComparison, ...]) -> list[int]:
    if any(not isinstance(comparison, NumericComparison) for comparison in comparisons):
        raise TypeError("comparisons must contain NumericComparison values")
    return [comparison._handle for comparison in comparisons]
