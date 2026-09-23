"""Programmatic, parse-free construction of planning domains and problems."""

from __future__ import annotations

import ctypes
from collections.abc import Sequence
from numbers import Real
from typing import TypeVar, cast

from .advanced import free_handle, lib
from .advanced._native import _Finalizer, _NativeOwner, _create_finalizer
from .model import ActionGenerator, Domain, Problem, _validate_generator
from .numeric import ComparisonOperator, NumericOperator, NumericUpdateOperator


_BuilderHandleT = TypeVar("_BuilderHandleT", bound="_BuilderHandle")
_ChildBuilderHandleT = TypeVar(
    "_ChildBuilderHandleT", bound="_ChildBuilderHandle"
)


def _utf8(value: object, name: str) -> bytes:
    if not isinstance(value, str):
        raise TypeError(f"{name} must be str")
    if "\0" in value:
        raise ValueError(f"{name} cannot contain a null character")
    return value.encode("utf-8")


def _string_array(
    values: Sequence[str],
    name: str,
) -> tuple[ctypes.c_void_p, object | None]:
    encoded = tuple(_utf8(value, name) for value in values)
    if not encoded:
        return ctypes.c_void_p(), None
    array = (ctypes.c_char_p * len(encoded))(*encoded)
    return ctypes.cast(array, ctypes.c_void_p), (array, encoded)


def _handle_array(
    values: Sequence["_BuilderHandle"],
) -> tuple[ctypes.c_void_p, object | None]:
    if not values:
        return ctypes.c_void_p(), None
    array = (ctypes.c_int * len(values))(*(value._handle for value in values))
    return ctypes.cast(array, ctypes.c_void_p), array


def _parameter_arrays(
    parameters: Sequence[tuple[str, str]],
) -> tuple[ctypes.c_void_p, ctypes.c_void_p, tuple[object | None, object | None]]:
    names: list[str] = []
    types: list[str] = []
    for parameter in parameters:
        if not isinstance(parameter, tuple) or len(parameter) != 2:
            raise TypeError("parameters must be (name, type_name) tuples")
        name, type_name = parameter
        if not isinstance(name, str) or not isinstance(type_name, str):
            raise TypeError("parameter names and type names must be str")
        names.append(name)
        types.append(type_name)
    name_pointer, name_array = _string_array(names, "parameter name")
    type_pointer, type_array = _string_array(types, "parameter type name")
    return name_pointer, type_pointer, (name_array, type_array)


def _polarity(positive: object) -> int:
    if not isinstance(positive, bool):
        raise TypeError("positive must be bool")
    return 0 if positive else 1


def _number(value: object, name: str) -> float:
    if isinstance(value, bool) or not isinstance(value, Real):
        raise TypeError(f"{name} must be a real number")
    return float(value)


def _require_handle(handle: int, operation: str) -> int:
    if handle == 0:
        raise RuntimeError(f"native {operation} returned a null handle")
    return handle


def _require_success(result: int, operation: str) -> None:
    if int(result) != 1:
        raise RuntimeError(f"native {operation} did not report success")


class _BuilderHandle(_NativeOwner):
    __slots__ = ("_handle", "_finalizer", "__weakref__")

    _handle: int
    _finalizer: _Finalizer

    def __init__(self) -> None:
        raise TypeError(
            f"{type(self).__name__} values are created by their parent builder"
        )

    def _initialize(self, handle: int) -> None:
        handle = _require_handle(handle, type(self).__name__)
        finalizer = _create_finalizer(self, free_handle, handle)
        try:
            self._handle = handle
            self._finalizer = finalizer
        except BaseException:
            finalizer()
            raise

    @classmethod
    def _from_handle(
        cls: type[_BuilderHandleT],
        handle: int,
    ) -> _BuilderHandleT:
        try:
            value = object.__new__(cls)
        except BaseException:
            free_handle(handle)
            raise
        value._initialize(handle)
        return value


class _ChildBuilderHandle(_BuilderHandle):
    __slots__ = ("_parent",)

    _parent: _BuilderHandle

    @classmethod
    def _from_parent(
        cls: type[_ChildBuilderHandleT],
        handle: int,
        parent: _BuilderHandle,
    ) -> _ChildBuilderHandleT:
        value = cls._from_handle(handle)
        try:
            value._parent = parent
        except BaseException:
            value._finalizer()
            raise
        return value


class LogicalExpressionSpec(_BuilderHandle):
    """An immutable logical expression created through :class:`Logic`."""

    __slots__ = ()


class NumericExpressionSpec(_BuilderHandle):
    __slots__ = ()

    @staticmethod
    def _coerce(value: NumericExpressionSpec | float) -> NumericExpressionSpec:
        return value if isinstance(value, NumericExpressionSpec) else Numeric.constant(value)

    def __add__(self, other: NumericExpressionSpec | float) -> NumericExpressionSpec:
        return Numeric._binary(NumericOperator.ADD, self, self._coerce(other))

    def __sub__(self, other: NumericExpressionSpec | float) -> NumericExpressionSpec:
        return Numeric._binary(NumericOperator.SUBTRACT, self, self._coerce(other))

    def __mul__(self, other: NumericExpressionSpec | float) -> NumericExpressionSpec:
        return Numeric._binary(NumericOperator.MULTIPLY, self, self._coerce(other))

    def __truediv__(self, other: NumericExpressionSpec | float) -> NumericExpressionSpec:
        return Numeric._binary(NumericOperator.DIVIDE, self, self._coerce(other))

    def __radd__(self, other: float) -> NumericExpressionSpec:
        return self._coerce(other) + self

    def __rsub__(self, other: float) -> NumericExpressionSpec:
        return self._coerce(other) - self

    def __rmul__(self, other: float) -> NumericExpressionSpec:
        return self._coerce(other) * self

    def __rtruediv__(self, other: float) -> NumericExpressionSpec:
        return self._coerce(other) / self

    def __neg__(self) -> NumericExpressionSpec:
        return Numeric.constant(0) - self

    def _compare(self, operation: ComparisonOperator, other: NumericExpressionSpec | float) -> LogicalExpressionSpec:
        right = self._coerce(other)
        handle = int(lib.mimir_numeric_spec_compare(self._handle, operation, right._handle))
        return LogicalExpressionSpec._from_handle(_require_handle(handle, "Numeric comparison"))

    def equal_to(self, other: NumericExpressionSpec | float) -> LogicalExpressionSpec:
        return self._compare(ComparisonOperator.EQUAL, other)

    def less_than(self, other: NumericExpressionSpec | float) -> LogicalExpressionSpec:
        return self._compare(ComparisonOperator.LESS_THAN, other)

    def less_than_or_equal(self, other: NumericExpressionSpec | float) -> LogicalExpressionSpec:
        return self._compare(ComparisonOperator.LESS_THAN_OR_EQUAL, other)

    def greater_than(self, other: NumericExpressionSpec | float) -> LogicalExpressionSpec:
        return self._compare(ComparisonOperator.GREATER_THAN, other)

    def greater_than_or_equal(self, other: NumericExpressionSpec | float) -> LogicalExpressionSpec:
        return self._compare(ComparisonOperator.GREATER_THAN_OR_EQUAL, other)


class NumericFunctionSpec(NumericExpressionSpec):
    __slots__ = ()


class Logic:
    """Factories for derived-predicate logical expressions."""

    def __new__(cls) -> "Logic":
        raise TypeError("Logic is a factory namespace and cannot be instantiated")

    @staticmethod
    def true() -> LogicalExpressionSpec:
        return LogicalExpressionSpec._from_handle(
            _require_handle(int(lib.mimir_logic_true()), "Logic.true")
        )

    @staticmethod
    def false() -> LogicalExpressionSpec:
        return LogicalExpressionSpec._from_handle(
            _require_handle(int(lib.mimir_logic_false()), "Logic.false")
        )

    @staticmethod
    def atom(predicate_name: str, *arguments: str) -> LogicalExpressionSpec:
        pointer, _array = _string_array(arguments, "argument")
        handle = int(
            lib.mimir_logic_atom(
                _utf8(predicate_name, "predicate_name"), pointer, len(arguments)
            )
        )
        return LogicalExpressionSpec._from_handle(
            _require_handle(handle, "Logic.atom")
        )

    @staticmethod
    def equal(left: str, right: str) -> LogicalExpressionSpec:
        handle = int(
            lib.mimir_logic_equal(
                _utf8(left, "left"),
                _utf8(right, "right"),
            )
        )
        return LogicalExpressionSpec._from_handle(
            _require_handle(handle, "Logic.equal")
        )

    @staticmethod
    def not_(expression: LogicalExpressionSpec) -> LogicalExpressionSpec:
        if not isinstance(expression, LogicalExpressionSpec):
            raise TypeError("expression must be a LogicalExpressionSpec")
        handle = int(lib.mimir_logic_not(expression._handle))
        return LogicalExpressionSpec._from_handle(
            _require_handle(handle, "Logic.not_")
        )

    @staticmethod
    def and_(*expressions: LogicalExpressionSpec) -> LogicalExpressionSpec:
        return Logic._junction("and", expressions)

    @staticmethod
    def or_(*expressions: LogicalExpressionSpec) -> LogicalExpressionSpec:
        return Logic._junction("or", expressions)

    @staticmethod
    def imply(
        antecedent: LogicalExpressionSpec,
        consequent: LogicalExpressionSpec,
    ) -> LogicalExpressionSpec:
        if not isinstance(antecedent, LogicalExpressionSpec):
            raise TypeError("antecedent must be a LogicalExpressionSpec")
        if not isinstance(consequent, LogicalExpressionSpec):
            raise TypeError("consequent must be a LogicalExpressionSpec")
        handle = int(
            lib.mimir_logic_imply(antecedent._handle, consequent._handle)
        )
        return LogicalExpressionSpec._from_handle(
            _require_handle(handle, "Logic.imply")
        )

    @staticmethod
    def exists(
        parameters: Sequence[tuple[str, str]],
        body: LogicalExpressionSpec,
    ) -> LogicalExpressionSpec:
        return Logic._quantified("exists", parameters, body)

    @staticmethod
    def forall(
        parameters: Sequence[tuple[str, str]],
        body: LogicalExpressionSpec,
    ) -> LogicalExpressionSpec:
        return Logic._quantified("forall", parameters, body)

    @staticmethod
    def _junction(
        operation: str,
        expressions: Sequence[LogicalExpressionSpec],
    ) -> LogicalExpressionSpec:
        if any(
            not isinstance(expression, LogicalExpressionSpec)
            for expression in expressions
        ):
            raise TypeError("expressions must be LogicalExpressionSpec values")
        pointer, _array = _handle_array(expressions)
        if operation == "and":
            handle = int(lib.mimir_logic_and(pointer, len(expressions)))
        else:
            handle = int(lib.mimir_logic_or(pointer, len(expressions)))
        return LogicalExpressionSpec._from_handle(
            _require_handle(handle, f"Logic.{operation}_")
        )

    @staticmethod
    def _quantified(
        operation: str,
        parameters: Sequence[tuple[str, str]],
        body: LogicalExpressionSpec,
    ) -> LogicalExpressionSpec:
        if isinstance(parameters, (str, bytes)) or not isinstance(
            parameters, Sequence
        ):
            raise TypeError("parameters must be a sequence")
        if not isinstance(body, LogicalExpressionSpec):
            raise TypeError("body must be a LogicalExpressionSpec")
        name_pointer, type_pointer, _arrays = _parameter_arrays(parameters)
        if operation == "exists":
            handle = int(
                lib.mimir_logic_exists(
                    name_pointer,
                    type_pointer,
                    len(parameters),
                    body._handle,
                )
            )
        else:
            handle = int(
                lib.mimir_logic_forall(
                    name_pointer,
                    type_pointer,
                    len(parameters),
                    body._handle,
                )
            )
        return LogicalExpressionSpec._from_handle(
            _require_handle(handle, f"Logic.{operation}")
        )


def _add_expression(
    builder: _BuilderHandle,
    expression: LogicalExpressionSpec,
    arguments: tuple[str, ...],
    positive: bool,
) -> None:
    if arguments or not positive:
        raise ValueError("expression conditions cannot have extra arguments or a negative polarity")
    _require_success(lib.mimir_builder_add_expression(builder._handle, expression._handle), "add expression")


def _add_numeric_update(
    builder: _BuilderHandle,
    target: NumericFunctionSpec,
    operation: NumericUpdateOperator,
    expression: NumericExpressionSpec | float,
) -> None:
    if not isinstance(target, NumericFunctionSpec):
        raise TypeError("numeric update targets must be function references")
    value = NumericExpressionSpec._coerce(expression)
    _require_success(
        lib.mimir_builder_numeric_update(builder._handle, target._handle, operation, value._handle),
        "numeric update",
    )


class Numeric:
    """Factories for numeric expressions."""

    def __new__(cls) -> "Numeric":
        raise TypeError("Numeric is a factory namespace and cannot be instantiated")

    @staticmethod
    def constant(value: float) -> NumericExpressionSpec:
        handle = int(lib.mimir_numeric_spec_constant(_number(value, "value")))
        return NumericExpressionSpec._from_handle(
            _require_handle(handle, "Numeric.constant")
        )

    @staticmethod
    def function(function_name: str, *arguments: str) -> NumericFunctionSpec:
        pointer, _array = _string_array(arguments, "argument")
        handle = int(
            lib.mimir_numeric_spec_function(
                _utf8(function_name, "function_name"),
                pointer,
                len(arguments),
            )
        )
        return NumericFunctionSpec._from_handle(
            _require_handle(handle, "Numeric.function")
        )

    @staticmethod
    def _binary(
        operation: NumericOperator,
        left: NumericExpressionSpec,
        right: NumericExpressionSpec,
    ) -> NumericExpressionSpec:
        if not isinstance(left, NumericExpressionSpec) or not isinstance(
            right, NumericExpressionSpec
        ):
            raise TypeError("left and right must be NumericExpressionSpec values")
        handle = int(lib.mimir_numeric_spec_binary(operation, left._handle, right._handle))
        return NumericExpressionSpec._from_handle(
            _require_handle(handle, f"Numeric.{operation.name.lower()}")
        )


class DomainBuilder(_BuilderHandle):
    """Build a :class:`Domain` without parsing PDDL text."""

    __slots__ = ()

    def __init__(self, name: str) -> None:
        handle = int(lib.mimir_domain_builder_create(_utf8(name, "name")))
        self._initialize(_require_handle(handle, "DomainBuilder"))

    def requirements(self) -> "RequirementListBuilder":
        handle = int(lib.mimir_domain_builder_requirements(self._handle))
        return RequirementListBuilder._from_parent(
            _require_handle(handle, "DomainBuilder.requirements"), self
        )

    def types(self) -> "TypeListBuilder":
        handle = int(lib.mimir_domain_builder_types(self._handle))
        return TypeListBuilder._from_parent(
            _require_handle(handle, "DomainBuilder.types"), self
        )

    def constants(self) -> "ConstantListBuilder":
        handle = int(lib.mimir_domain_builder_constants(self._handle))
        return ConstantListBuilder._from_parent(
            _require_handle(handle, "DomainBuilder.constants"), self
        )

    def predicates(self) -> "PredicateListBuilder":
        handle = int(lib.mimir_domain_builder_predicates(self._handle))
        return PredicateListBuilder._from_parent(
            _require_handle(handle, "DomainBuilder.predicates"), self
        )

    def functions(self) -> "NumericFunctionListBuilder":
        handle = int(lib.mimir_domain_builder_functions(self._handle))
        return NumericFunctionListBuilder._from_parent(
            _require_handle(handle, "DomainBuilder.functions"), self
        )

    def actions(self) -> "ActionListBuilder":
        handle = int(lib.mimir_domain_builder_actions(self._handle))
        return ActionListBuilder._from_parent(
            _require_handle(handle, "DomainBuilder.actions"), self
        )

    def derived_predicates(self) -> "DerivedPredicateListBuilder":
        handle = int(lib.mimir_domain_builder_derived_predicates(self._handle))
        return DerivedPredicateListBuilder._from_parent(
            _require_handle(handle, "DomainBuilder.derived_predicates"), self
        )

    def build(self) -> Domain:
        handle = int(lib.mimir_domain_builder_build(self._handle))
        return Domain._from_handle(
            _require_handle(handle, "DomainBuilder.build")
        )


class RequirementListBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add(self, requirement: str) -> "RequirementListBuilder":
        result = lib.mimir_requirement_list_builder_add(
            self._handle, _utf8(requirement, "requirement")
        )
        _require_success(result, "RequirementListBuilder.add")
        return self

    def close(self) -> DomainBuilder:
        result = lib.mimir_requirement_list_builder_close(self._handle)
        _require_success(result, "RequirementListBuilder.close")
        return cast(DomainBuilder, self._parent)


class TypeListBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add(
        self,
        name: str,
        parent_type: str = "object",
    ) -> "TypeListBuilder":
        result = lib.mimir_type_list_builder_add(
            self._handle,
            _utf8(name, "name"),
            _utf8(parent_type, "parent_type"),
        )
        _require_success(result, "TypeListBuilder.add")
        return self

    def close(self) -> DomainBuilder:
        result = lib.mimir_type_list_builder_close(self._handle)
        _require_success(result, "TypeListBuilder.close")
        return cast(DomainBuilder, self._parent)


class ConstantListBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add(
        self,
        name: str,
        type_name: str = "object",
    ) -> "ConstantListBuilder":
        result = lib.mimir_constant_list_builder_add(
            self._handle,
            _utf8(name, "name"),
            _utf8(type_name, "type_name"),
        )
        _require_success(result, "ConstantListBuilder.add")
        return self

    def close(self) -> DomainBuilder:
        result = lib.mimir_constant_list_builder_close(self._handle)
        _require_success(result, "ConstantListBuilder.close")
        return cast(DomainBuilder, self._parent)


class PredicateListBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add(
        self,
        name: str,
        *parameters: tuple[str, str],
    ) -> "PredicateListBuilder":
        name_pointer, type_pointer, _arrays = _parameter_arrays(parameters)
        result = lib.mimir_predicate_list_builder_add(
            self._handle,
            _utf8(name, "name"),
            name_pointer,
            type_pointer,
            len(parameters),
        )
        _require_success(result, "PredicateListBuilder.add")
        return self

    def close(self) -> DomainBuilder:
        result = lib.mimir_predicate_list_builder_close(self._handle)
        _require_success(result, "PredicateListBuilder.close")
        return cast(DomainBuilder, self._parent)


class NumericFunctionListBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add(
        self,
        name: str,
        *parameters: tuple[str, str],
    ) -> "NumericFunctionListBuilder":
        name_pointer, type_pointer, _arrays = _parameter_arrays(parameters)
        result = lib.mimir_numeric_function_list_builder_add(
            self._handle,
            _utf8(name, "name"),
            name_pointer,
            type_pointer,
            len(parameters),
        )
        _require_success(result, "NumericFunctionListBuilder.add")
        return self

    def close(self) -> DomainBuilder:
        result = lib.mimir_numeric_function_list_builder_close(self._handle)
        _require_success(result, "NumericFunctionListBuilder.close")
        return cast(DomainBuilder, self._parent)


class ActionListBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add(self, name: str) -> "ActionSchemaBuilder":
        handle = int(
            lib.mimir_action_list_builder_add(
                self._handle, _utf8(name, "name")
            )
        )
        return ActionSchemaBuilder._from_parent(
            _require_handle(handle, "ActionListBuilder.add"), self
        )

    def close(self) -> DomainBuilder:
        result = lib.mimir_action_list_builder_close(self._handle)
        _require_success(result, "ActionListBuilder.close")
        return cast(DomainBuilder, self._parent)


class ActionSchemaBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add_parameter(
        self,
        name: str,
        type_name: str = "object",
    ) -> "ActionSchemaBuilder":
        result = lib.mimir_action_schema_builder_add_parameter(
            self._handle,
            _utf8(name, "name"),
            _utf8(type_name, "type_name"),
        )
        _require_success(result, "ActionSchemaBuilder.add_parameter")
        return self

    def add_precondition(
        self,
        predicate_name: str | LogicalExpressionSpec,
        *arguments: str,
        positive: bool = True,
    ) -> "ActionSchemaBuilder":
        if isinstance(predicate_name, LogicalExpressionSpec):
            _add_expression(self, predicate_name, arguments, positive)
            return self
        pointer, _array = _string_array(arguments, "argument")
        result = lib.mimir_action_schema_builder_add_precondition(
            self._handle,
            _utf8(predicate_name, "predicate_name"),
            _polarity(positive),
            pointer,
            len(arguments),
        )
        _require_success(result, "ActionSchemaBuilder.add_precondition")
        return self

    def add_effect(
        self,
        predicate_name: str,
        *arguments: str,
        positive: bool = True,
    ) -> "ActionSchemaBuilder":
        pointer, _array = _string_array(arguments, "argument")
        result = lib.mimir_action_schema_builder_add_effect(
            self._handle,
            _utf8(predicate_name, "predicate_name"),
            _polarity(positive),
            pointer,
            len(arguments),
        )
        _require_success(result, "ActionSchemaBuilder.add_effect")
        return self

    def add_conditional_effect(self) -> "ConditionalEffectBuilder":
        handle = int(
            lib.mimir_action_schema_builder_add_conditional_effect(self._handle)
        )
        return ConditionalEffectBuilder._from_parent(
            _require_handle(
                handle, "ActionSchemaBuilder.add_conditional_effect"
            ),
            self,
        )

    def with_cost(
        self,
        cost: float | NumericExpressionSpec,
    ) -> "ActionSchemaBuilder":
        if isinstance(cost, NumericExpressionSpec):
            result = lib.mimir_action_schema_builder_with_cost(
                self._handle, cost._handle
            )
        else:
            result = lib.mimir_action_schema_builder_with_constant_cost(
                self._handle, _number(cost, "cost")
            )
        _require_success(result, "ActionSchemaBuilder.with_cost")
        return self

    def assign(self, target: NumericFunctionSpec, expression: NumericExpressionSpec | float) -> ActionSchemaBuilder:
        _add_numeric_update(self, target, NumericUpdateOperator.ASSIGN, expression)
        return self

    def increase(self, target: NumericFunctionSpec, expression: NumericExpressionSpec | float) -> ActionSchemaBuilder:
        _add_numeric_update(self, target, NumericUpdateOperator.INCREASE, expression)
        return self

    def decrease(self, target: NumericFunctionSpec, expression: NumericExpressionSpec | float) -> ActionSchemaBuilder:
        _add_numeric_update(self, target, NumericUpdateOperator.DECREASE, expression)
        return self

    def scale_up(self, target: NumericFunctionSpec, expression: NumericExpressionSpec | float) -> ActionSchemaBuilder:
        _add_numeric_update(self, target, NumericUpdateOperator.SCALE_UP, expression)
        return self

    def scale_down(self, target: NumericFunctionSpec, expression: NumericExpressionSpec | float) -> ActionSchemaBuilder:
        _add_numeric_update(self, target, NumericUpdateOperator.SCALE_DOWN, expression)
        return self

    def close(self) -> ActionListBuilder:
        result = lib.mimir_action_schema_builder_close(self._handle)
        _require_success(result, "ActionSchemaBuilder.close")
        return cast(ActionListBuilder, self._parent)


class ConditionalEffectBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add_parameter(
        self,
        name: str,
        type_name: str = "object",
    ) -> "ConditionalEffectBuilder":
        result = lib.mimir_conditional_effect_builder_add_parameter(
            self._handle,
            _utf8(name, "name"),
            _utf8(type_name, "type_name"),
        )
        _require_success(result, "ConditionalEffectBuilder.add_parameter")
        return self

    def add_condition(
        self,
        predicate_name: str | LogicalExpressionSpec,
        *arguments: str,
        positive: bool = True,
    ) -> "ConditionalEffectBuilder":
        if isinstance(predicate_name, LogicalExpressionSpec):
            _add_expression(self, predicate_name, arguments, positive)
            return self
        pointer, _array = _string_array(arguments, "argument")
        result = lib.mimir_conditional_effect_builder_add_condition(
            self._handle,
            _utf8(predicate_name, "predicate_name"),
            _polarity(positive),
            pointer,
            len(arguments),
        )
        _require_success(result, "ConditionalEffectBuilder.add_condition")
        return self

    def add_effect(
        self,
        predicate_name: str,
        *arguments: str,
        positive: bool = True,
    ) -> "ConditionalEffectBuilder":
        pointer, _array = _string_array(arguments, "argument")
        result = lib.mimir_conditional_effect_builder_add_effect(
            self._handle,
            _utf8(predicate_name, "predicate_name"),
            _polarity(positive),
            pointer,
            len(arguments),
        )
        _require_success(result, "ConditionalEffectBuilder.add_effect")
        return self

    def assign(self, target: NumericFunctionSpec, expression: NumericExpressionSpec | float) -> ConditionalEffectBuilder:
        _add_numeric_update(self, target, NumericUpdateOperator.ASSIGN, expression)
        return self

    def increase(self, target: NumericFunctionSpec, expression: NumericExpressionSpec | float) -> ConditionalEffectBuilder:
        _add_numeric_update(self, target, NumericUpdateOperator.INCREASE, expression)
        return self

    def decrease(self, target: NumericFunctionSpec, expression: NumericExpressionSpec | float) -> ConditionalEffectBuilder:
        _add_numeric_update(self, target, NumericUpdateOperator.DECREASE, expression)
        return self

    def scale_up(self, target: NumericFunctionSpec, expression: NumericExpressionSpec | float) -> ConditionalEffectBuilder:
        _add_numeric_update(self, target, NumericUpdateOperator.SCALE_UP, expression)
        return self

    def scale_down(self, target: NumericFunctionSpec, expression: NumericExpressionSpec | float) -> ConditionalEffectBuilder:
        _add_numeric_update(self, target, NumericUpdateOperator.SCALE_DOWN, expression)
        return self

    def close(self) -> ActionSchemaBuilder:
        result = lib.mimir_conditional_effect_builder_close(self._handle)
        _require_success(result, "ConditionalEffectBuilder.close")
        return cast(ActionSchemaBuilder, self._parent)


class DerivedPredicateListBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def define(
        self,
        predicate_name: str,
        body: LogicalExpressionSpec,
    ) -> "DerivedPredicateListBuilder":
        if not isinstance(body, LogicalExpressionSpec):
            raise TypeError("body must be a LogicalExpressionSpec")
        result = lib.mimir_derived_predicate_list_builder_define(
            self._handle,
            _utf8(predicate_name, "predicate_name"),
            body._handle,
        )
        _require_success(result, "DerivedPredicateListBuilder.define")
        return self

    def close(self) -> DomainBuilder:
        result = lib.mimir_derived_predicate_list_builder_close(self._handle)
        _require_success(result, "DerivedPredicateListBuilder.close")
        return cast(DomainBuilder, self._parent)


class ProblemBuilder(_BuilderHandle):
    """Build a :class:`Problem` without parsing PDDL text."""

    __slots__ = ("_domain",)

    _domain: Domain

    def __init__(
        self,
        domain: Domain,
        name: str,
        *,
        generator: ActionGenerator = "lifted",
    ) -> None:
        if not isinstance(domain, Domain):
            raise TypeError("domain must be a Domain")
        generator_value = _validate_generator(generator)
        handle = int(
            lib.mimir_problem_builder_create(
                domain._handle,
                _utf8(name, "name"),
                generator_value.encode("utf-8"),
            )
        )
        self._initialize(_require_handle(handle, "ProblemBuilder"))
        self._domain = domain

    def objects(self) -> "ProblemObjectListBuilder":
        handle = int(lib.mimir_problem_builder_objects(self._handle))
        return ProblemObjectListBuilder._from_parent(
            _require_handle(handle, "ProblemBuilder.arguments"), self
        )

    def initial_state(self) -> "InitialStateBuilder":
        handle = int(lib.mimir_problem_builder_initial_state(self._handle))
        return InitialStateBuilder._from_parent(
            _require_handle(handle, "ProblemBuilder.initial_state"), self
        )

    def goal(self) -> "GoalBuilder":
        handle = int(lib.mimir_problem_builder_goal(self._handle))
        return GoalBuilder._from_parent(
            _require_handle(handle, "ProblemBuilder.goal"), self
        )

    def build(self) -> Problem:
        handle = int(lib.mimir_problem_builder_build(self._handle))
        return Problem._from_handle(
            _require_handle(handle, "ProblemBuilder.build"), self._domain
        )


class ProblemObjectListBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add(
        self,
        name: str,
        type_name: str = "object",
    ) -> "ProblemObjectListBuilder":
        result = lib.mimir_problem_object_list_builder_add(
            self._handle,
            _utf8(name, "name"),
            _utf8(type_name, "type_name"),
        )
        _require_success(result, "ProblemObjectListBuilder.add")
        return self

    def close(self) -> ProblemBuilder:
        result = lib.mimir_problem_object_list_builder_close(self._handle)
        _require_success(result, "ProblemObjectListBuilder.close")
        return cast(ProblemBuilder, self._parent)


class InitialStateBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add_fact(
        self,
        predicate_name: str,
        *arguments: str,
    ) -> "InitialStateBuilder":
        pointer, _array = _string_array(arguments, "argument")
        result = lib.mimir_initial_state_builder_add_fact(
            self._handle,
            _utf8(predicate_name, "predicate_name"),
            pointer,
            len(arguments),
        )
        _require_success(result, "InitialStateBuilder.add_fact")
        return self

    def set_value(self, target: NumericFunctionSpec, value: float) -> InitialStateBuilder:
        if not isinstance(target, NumericFunctionSpec):
            raise TypeError("initialization targets must be function references")
        result = lib.mimir_initial_state_set_value(self._handle, target._handle, _number(value, "value"))
        _require_success(result, "InitialStateBuilder.set_value")
        return self

    def close(self) -> ProblemBuilder:
        result = lib.mimir_initial_state_builder_close(self._handle)
        _require_success(result, "InitialStateBuilder.close")
        return cast(ProblemBuilder, self._parent)


class GoalBuilder(_ChildBuilderHandle):
    __slots__ = ()

    def add(
        self,
        predicate_name: str | LogicalExpressionSpec,
        *arguments: str,
        positive: bool = True,
    ) -> "GoalBuilder":
        if isinstance(predicate_name, LogicalExpressionSpec):
            _add_expression(self, predicate_name, arguments, positive)
            return self
        pointer, _array = _string_array(arguments, "argument")
        result = lib.mimir_goal_builder_add(
            self._handle,
            _utf8(predicate_name, "predicate_name"),
            _polarity(positive),
            pointer,
            len(arguments),
        )
        _require_success(result, "GoalBuilder.add")
        return self

    def close(self) -> ProblemBuilder:
        result = lib.mimir_goal_builder_close(self._handle)
        _require_success(result, "GoalBuilder.close")
        return cast(ProblemBuilder, self._parent)


__all__ = [
    "DomainBuilder",
    "RequirementListBuilder",
    "TypeListBuilder",
    "ConstantListBuilder",
    "PredicateListBuilder",
    "NumericFunctionListBuilder",
    "ActionListBuilder",
    "ActionSchemaBuilder",
    "ConditionalEffectBuilder",
    "DerivedPredicateListBuilder",
    "ProblemBuilder",
    "ProblemObjectListBuilder",
    "InitialStateBuilder",
    "GoalBuilder",
    "Logic",
    "LogicalExpressionSpec",
    "Numeric",
    "NumericExpressionSpec",
    "NumericFunctionSpec",
]
