"""High-level, Pythonic formalism classes for Mimir."""

from __future__ import annotations

import ctypes
import os
from collections.abc import Iterable, Iterator, Mapping, Sequence
from typing import cast
from enum import Enum
from functools import cached_property
from types import MappingProxyType
from typing import Literal as TypingLiteral, TypeAlias, TypeVar, overload

from .advanced import (
    free_handle,
    lib,
    relationship_value,
    take_string,
    value_equals,
    value_hash,
)
from .advanced._native import _Finalizer, _NativeOwner, _create_finalizer, raise_last_error
from ._base import Object, Term, Variable, _Handle, _required_string, _wrap_term
from .numeric import (
    NumericFunction, NumericExpression, FunctionCall, GroundFunctionCall, NumericComparison,
    GroundNumericComparison, NumericUpdate, GroundNumericUpdate, _wrap_numeric,
    _read_comparisons, _read_updates, _comparison_handles, NumericBinaryExpression, NumericConstant,
)


# -- Predicate type codes (must match Api.*.cs) -------------------------------

_PREDICATE_TYPE_STATIC = 1
_PREDICATE_TYPE_FLUENT = 2
_PREDICATE_TYPE_DERIVED = 3


class PredicateType(str, Enum):
    STATIC = "static"
    FLUENT = "fluent"
    DERIVED = "derived"


ActionGenerator: TypeAlias = TypingLiteral["grounded", "lifted"]


def _validate_generator(generator: object) -> ActionGenerator:
    if not isinstance(generator, str):
        raise TypeError("generator must be str")
    if generator == "grounded":
        return "grounded"
    if generator == "lifted":
        return "lifted"
    raise ValueError("generator must be 'grounded' or 'lifted'")



# =============================================================================
# Predicate
# =============================================================================


class Predicate(_Handle):
    """A PDDL predicate; can be static, fluent, or derived."""

    @property
    def name(self) -> str:
        return _required_string(lib.mimir_predicate_get_name(self._handle), "Predicate.name")

    @property
    def arity(self) -> int:
        return int(lib.mimir_predicate_get_arity(self._handle))

    @property
    def parameters(self) -> tuple[Variable, ...]:
        n = lib.mimir_predicate_get_parameter_count(self._handle)
        return tuple(
            Variable._from_handle(
                lib.mimir_predicate_get_parameter(self._handle, i),
                self._owner,
            )
            for i in range(n)
        )

    @property
    def predicate_type(self) -> PredicateType:
        return _predicate_type(self)

    @property
    def is_static(self) -> bool:
        return self.predicate_type is PredicateType.STATIC

    @property
    def is_fluent(self) -> bool:
        return self.predicate_type is PredicateType.FLUENT

    @property
    def is_derived(self) -> bool:
        return self.predicate_type is PredicateType.DERIVED

    def __str__(self) -> str:
        params = " ".join(str(p) for p in self.parameters)
        return f"({self.name} {params})" if params else f"({self.name})"

    def __repr__(self) -> str:
        return f"Predicate({self.name!r})"

# =============================================================================
# Atom (lifted) and GroundAtom (ground Fact)
# =============================================================================


class Atom(_Handle):
    """A lifted atom: a predicate applied to terms (variables and/or objects)."""

    @property
    def predicate(self) -> Predicate:
        owner = self._owner.domain if isinstance(self._owner, Problem) else self._owner
        return Predicate._from_handle(lib.mimir_atom_get_predicate(self._handle), owner)

    @property
    def arguments(self) -> tuple[Term, ...]:
        n = lib.mimir_atom_get_argument_count(self._handle)
        return tuple(_wrap_term(lib.mimir_atom_get_argument(self._handle, i), self._owner) for i in range(n))

    @property
    def variables(self) -> tuple[Variable, ...]:
        return tuple(term for term in self.arguments if isinstance(term, Variable))

    @property
    def arity(self) -> int:
        return int(lib.mimir_atom_get_argument_count(self._handle))

    @property
    def predicate_type(self) -> PredicateType:
        return _predicate_type(self)

    @property
    def is_static(self) -> bool:
        return self.predicate_type is PredicateType.STATIC

    @property
    def is_fluent(self) -> bool:
        return self.predicate_type is PredicateType.FLUENT

    @property
    def is_derived(self) -> bool:
        return self.predicate_type is PredicateType.DERIVED

    def __str__(self) -> str:
        return _required_string(lib.mimir_atom_to_string(self._handle), "Atom.__str__")

    def __repr__(self) -> str:
        return self.__str__()

class GroundAtom(_Handle):
    """A ground atom: a predicate applied to objects."""

    @cached_property
    def predicate(self) -> Predicate:
        owner = self._owner.domain if isinstance(self._owner, Problem) else self._owner
        return Predicate._from_handle(lib.mimir_ground_atom_get_predicate(self._handle), owner)

    @cached_property
    def arguments(self) -> tuple[Object, ...]:
        n = lib.mimir_ground_atom_get_argument_count(self._handle)
        return tuple(
            Object._from_handle(
                lib.mimir_ground_atom_get_argument(self._handle, i),
                self._owner,
            )
            for i in range(n)
        )

    @property
    def arity(self) -> int:
        return int(lib.mimir_ground_atom_get_argument_count(self._handle))

    @property
    def predicate_type(self) -> PredicateType:
        return _predicate_type(self)

    @property
    def is_static(self) -> bool:
        return self.predicate_type is PredicateType.STATIC

    @property
    def is_fluent(self) -> bool:
        return self.predicate_type is PredicateType.FLUENT

    @property
    def is_derived(self) -> bool:
        return self.predicate_type is PredicateType.DERIVED

    def __str__(self) -> str:
        return _required_string(lib.mimir_ground_atom_to_string(self._handle), "GroundAtom.__str__")

    def __repr__(self) -> str:
        return self.__str__()

# =============================================================================
# Literal (lifted) and GroundLiteral
# =============================================================================


class Literal(_Handle):
    """A lifted literal: an Atom together with positive/negative polarity."""

    @property
    def atom(self) -> Atom:
        return Atom._from_handle(lib.mimir_literal_get_atom(self._handle), self._owner)

    @property
    def is_positive(self) -> bool:
        # Native: 0 = positive (True), 1 = negative (False)
        return int(lib.mimir_literal_get_polarity(self._handle)) == 0

    @property
    def is_negative(self) -> bool:
        return not self.is_positive

    @property
    def predicate_type(self) -> PredicateType:
        return _predicate_type(self)

    @property
    def is_static(self) -> bool:
        return self.predicate_type is PredicateType.STATIC

    @property
    def is_fluent(self) -> bool:
        return self.predicate_type is PredicateType.FLUENT

    @property
    def is_derived(self) -> bool:
        return self.predicate_type is PredicateType.DERIVED

    def __str__(self) -> str:
        return str(self.atom) if self.is_positive else f"(not {self.atom})"

    def __repr__(self) -> str:
        return self.__str__()

class GroundLiteral(_Handle):
    """A ground literal: a GroundAtom with positive/negative polarity."""

    @property
    def atom(self) -> GroundAtom:
        return GroundAtom._from_handle(lib.mimir_ground_literal_get_atom(self._handle), self._owner)

    @property
    def is_positive(self) -> bool:
        return int(lib.mimir_ground_literal_get_polarity(self._handle)) == 0

    @property
    def is_negative(self) -> bool:
        return not self.is_positive

    @property
    def predicate_type(self) -> PredicateType:
        return _predicate_type(self)

    @property
    def is_static(self) -> bool:
        return self.predicate_type is PredicateType.STATIC

    @property
    def is_fluent(self) -> bool:
        return self.predicate_type is PredicateType.FLUENT

    @property
    def is_derived(self) -> bool:
        return self.predicate_type is PredicateType.DERIVED

    def __str__(self) -> str:
        return str(self.atom) if self.is_positive else f"(not {self.atom})"

    def __repr__(self) -> str:
        return self.__str__()

# =============================================================================
# Effects (lifted) and ConditionalEffect
# =============================================================================


class Effect:
    """A lifted effect represented as parameters plus effect literals."""

    __slots__ = ("_parameters", "_literals", "_owner", "_numeric_effects")

    _parameters: tuple[Variable, ...]
    _literals: tuple[Literal, ...]
    _owner: Domain | Problem | None

    _numeric_effects: tuple[NumericUpdate, ...]

    def __init__(self) -> None:
        raise TypeError("effects are created by actions")

    @classmethod
    def _from_parts(
        cls,
        parameters: Sequence[Variable],
        literals: Sequence[Literal],
        owner: Domain | Problem | None,
        numeric_effects: Sequence[NumericUpdate] = (),
    ) -> "Effect":
        values = [*parameters, *literals]
        if owner is None and values:
            owner = values[0]._owner
        if any(value._owner != owner for value in values):
            raise ValueError("effect values must have the same owner")
        value = object.__new__(cls)
        value._numeric_effects = tuple(numeric_effects)
        value._parameters = tuple(parameters)
        value._literals = tuple(literals)
        value._owner = owner
        return value

    @property
    def parameters(self) -> tuple[Variable, ...]:
        return self._parameters

    @property
    def literals(self) -> tuple[Literal, ...]:
        return self._literals

    def __iter__(self) -> Iterator[Literal]:
        return iter(self._literals)

    @property
    def numeric_effects(self) -> tuple[NumericUpdate, ...]:
        return self._numeric_effects

    def __str__(self) -> str:
        return "(and " + " ".join(str(l) for l in self._literals) + ")"

    def __repr__(self) -> str:
        return self.__str__()

    def __eq__(self, other: object) -> bool:
        return (
            isinstance(other, Effect)
            and self._owner == other._owner
            and self._parameters == other._parameters
            and self._literals == other._literals
            and self.numeric_effects == other.numeric_effects
        )

    def __hash__(self) -> int:
        return hash(("Effect", self._owner, tuple(self._parameters), tuple(self._literals), self.numeric_effects))


class _ConditionalEffectBase(_Handle):
    """The ``(forall ... (when condition ...))`` part shared by literal and numeric conditional effects."""

    @property
    def condition(self) -> "ConjunctiveCondition":
        if self._owner is None:
            raise RuntimeError("conditional effect has no owner")
        return ConjunctiveCondition._from_parts(
            None, self._owner, list(self.quantified_variables), self._conditions(), _read_comparisons(self._handle, self._owner)
        )

    @property
    def effect(self) -> Effect:
        raise NotImplementedError

    @property
    def quantified_variables(self) -> tuple[Variable, ...]:
        n = lib.mimir_conditional_effect_get_quantified_count(self._handle)
        return tuple(Variable._from_handle(lib.mimir_conditional_effect_get_quantified(self._handle, i), self._owner) for i in range(n))

    @property
    def fluent_literals(self) -> tuple[Literal, ...]:
        n = lib.mimir_conditional_effect_get_fluent_condition_count(self._handle)
        return tuple(Literal._from_handle(lib.mimir_conditional_effect_get_fluent_condition(self._handle, i), self._owner) for i in range(n))

    @property
    def static_literals(self) -> tuple[Literal, ...]:
        n = lib.mimir_conditional_effect_get_static_condition_count(self._handle)
        return tuple(Literal._from_handle(lib.mimir_conditional_effect_get_static_condition(self._handle, i), self._owner) for i in range(n))

    @property
    def derived_literals(self) -> tuple[Literal, ...]:
        n = lib.mimir_conditional_effect_get_derived_condition_count(self._handle)
        return tuple(Literal._from_handle(lib.mimir_conditional_effect_get_derived_condition(self._handle, i), self._owner) for i in range(n))

    def _conditions(self) -> list[Literal]:
        return list(self.static_literals + self.fluent_literals + self.derived_literals)

    def __str__(self) -> str:
        conds = self._conditions()
        effect = self.effect
        if not conds:
            return str(effect)
        cond_str = " ".join(str(c) for c in conds)
        return f"(when (and {cond_str}) {effect})"

    def __repr__(self) -> str:
        return self.__str__()


class ConditionalEffect(_ConditionalEffectBase):
    """A lifted ``(forall ... (when condition literal))`` clause."""

    @property
    def effect(self) -> Effect:
        if self._owner is None:
            raise RuntimeError("conditional effect has no owner")
        return Effect._from_parts(self.quantified_variables, (self._effect_literal,), self._owner)

    @property
    def _effect_literal(self) -> Literal:
        return Literal._from_handle(lib.mimir_conditional_effect_get_effect(self._handle), self._owner)


class ConditionalNumericEffect(_ConditionalEffectBase):
    """A lifted ``(forall ... (when condition numeric-update))`` clause."""

    @property
    def effect(self) -> Effect:
        if self._owner is None:
            raise RuntimeError("conditional effect has no owner")
        updates = cast(tuple[NumericUpdate, ...], _read_updates(self._handle, self._owner))
        return Effect._from_parts(self.quantified_variables, (), self._owner, updates)

# =============================================================================
# Ground effect and GroundConditionalEffect
# =============================================================================


class GroundEffect:
    """The unconditional add/delete lists of a grounded Action."""

    __slots__ = ("_action", "_problem", "_add_list", "_delete_list", "_numeric_effects")

    _action: GroundAction | None
    _problem: Problem
    _add_list: tuple[GroundAtom, ...] | None
    _delete_list: tuple[GroundAtom, ...] | None

    _numeric_effects: tuple[GroundNumericUpdate, ...]

    def __init__(self) -> None:
        raise TypeError("ground effects are created by grounded actions")

    @classmethod
    def _from_parts(
        cls,
        problem: "Problem",
        *,
        action: "GroundAction" | None = None,
        add_atoms: Sequence[GroundAtom] | None = None,
        delete_atoms: Sequence[GroundAtom] | None = None,
        numeric_effects: Sequence[GroundNumericUpdate] = (),
    ) -> "GroundEffect":
        supplied_atoms = [*(add_atoms or ()), *(delete_atoms or ())]
        if any(atom._owner != problem for atom in supplied_atoms):
            raise ValueError("ground effect atoms must belong to its problem")
        value = object.__new__(cls)
        value._numeric_effects = tuple(numeric_effects)
        value._action = action
        value._problem = problem
        value._add_list = tuple(add_atoms) if add_atoms is not None else None
        value._delete_list = tuple(delete_atoms) if delete_atoms is not None else None
        return value

    @property
    def problem(self) -> "Problem":
        return self._problem

    @property
    def add_atoms(self) -> tuple[GroundAtom, ...]:
        if self._add_list is not None:
            return self._add_list
        if self._action is None:
            return ()
        n = lib.mimir_ground_action_get_add_effect_count(self._action._handle)
        return tuple(
            GroundAtom._from_handle(lib.mimir_ground_action_get_add_effect(self._action._handle, i), self._problem)
            for i in range(n)
        )

    @property
    def delete_atoms(self) -> tuple[GroundAtom, ...]:
        if self._delete_list is not None:
            return self._delete_list
        if self._action is None:
            return ()
        n = lib.mimir_ground_action_get_delete_effect_count(self._action._handle)
        return tuple(
            GroundAtom._from_handle(lib.mimir_ground_action_get_delete_effect(self._action._handle, i), self._problem)
            for i in range(n)
        )

    @property
    def numeric_effects(self) -> tuple[GroundNumericUpdate, ...]:
        if self._action is not None:
            return cast(tuple[GroundNumericUpdate, ...], _read_updates(self._action._handle, self._problem))
        return self._numeric_effects

    @property
    def literals(self) -> tuple[GroundLiteral, ...]:
        return tuple(self._problem.ground_literal(atom) for atom in self.add_atoms) + tuple(
            self._problem.ground_literal(atom, positive=False) for atom in self.delete_atoms)

    def __str__(self) -> str:
        adds = " ".join(str(a) for a in self.add_atoms)
        dels = " ".join(f"(not {a})" for a in self.delete_atoms)
        return f"(and {adds} {dels})" if adds or dels else "(and)"

    def __repr__(self) -> str:
        return self.__str__()

    def __eq__(self, other: object) -> bool:
        return (
            isinstance(other, GroundEffect)
            and self._problem == other._problem
            and self.add_atoms == other.add_atoms
            and self.delete_atoms == other.delete_atoms
            and self.numeric_effects == other.numeric_effects
        )

    def __hash__(self) -> int:
        return hash((
            "GroundEffect", self._problem,
            self.add_atoms, self.delete_atoms, self.numeric_effects
        ))


_GroundConditionalEffectT = TypeVar("_GroundConditionalEffectT", bound="_GroundConditionalEffectBase")


class _GroundConditionalEffectBase(_Handle):
    """The condition part shared by grounded literal and numeric conditional effects."""

    def __init__(self) -> None:
        raise TypeError("ground conditional effects are created by grounded actions")

    _problem: Problem

    @classmethod
    def _from_handle(
        cls: type[_GroundConditionalEffectT],
        handle: int,
        owner: Domain | Problem | None = None,
    ) -> _GroundConditionalEffectT:
        value = cls._allocate_for_handle(handle)
        try:
            if not isinstance(owner, Problem):
                raise TypeError("ground conditional effects require a problem owner")
            value._problem = owner
        except BaseException:
            free_handle(handle)
            raise
        value._initialize(handle, owner)
        return value

    @property
    def problem(self) -> "Problem":
        return self._problem

    @property
    def effect(self) -> GroundEffect:
        raise NotImplementedError

    @property
    def condition(self) -> "GroundConjunctiveCondition":
        return GroundConjunctiveCondition._from_literals(self._conditions(), self._problem, _read_comparisons(self._handle, self._problem))

    # Bitboard-encoded condition types: (count_fn, get_fn, polarity).
    _CONDITION_KINDS = (
        ("mimir_ground_conditional_effect_get_positive_fluent_count",
         "mimir_ground_conditional_effect_get_positive_fluent", True),
        ("mimir_ground_conditional_effect_get_negative_fluent_count",
         "mimir_ground_conditional_effect_get_negative_fluent", False),
        ("mimir_ground_conditional_effect_get_positive_static_count",
         "mimir_ground_conditional_effect_get_positive_static", True),
        ("mimir_ground_conditional_effect_get_negative_static_count",
         "mimir_ground_conditional_effect_get_negative_static", False),
    )

    def _conditions(self) -> list[GroundLiteral]:
        result: list[GroundLiteral] = []
        for count_name, get_name, polarity in self._CONDITION_KINDS:
            count_fn = getattr(lib, count_name)
            get_fn = getattr(lib, get_name)
            for i in range(count_fn(self._handle)):
                atom = GroundAtom._from_handle(get_fn(self._handle, i), self._problem)
                result.append(self._problem.ground_literal(atom, positive=polarity))

        # Derived conditions are already proper Literal<Fact<Derived>>.
        n_d = lib.mimir_ground_conditional_effect_get_derived_count(self._handle)
        for i in range(n_d):
            result.append(GroundLiteral._from_handle(
                lib.mimir_ground_conditional_effect_get_derived(self._handle, i),
                self._problem,
            ))
        return result

    def is_satisfied(self, state: "State") -> bool:
        if not isinstance(state, State):
            raise TypeError("state must be a State")
        if state.problem != self._problem:
            raise ValueError("conditional effect and state belong to different problems")
        return relationship_value(
            lib.mimir_ground_conditional_effect_is_satisfied(self._handle, state._handle),
            "conditional effect and state belong to different problems",
        )

    def __str__(self) -> str:
        cond = self.condition
        effect = self.effect
        if len(cond) == 0:
            return str(effect)
        return f"(when {cond} {effect})"

    def __repr__(self) -> str:
        return self.__str__()


class GroundConditionalEffect(_GroundConditionalEffectBase):
    """A grounded conditional effect: condition ground literals plus one effect ground literal."""

    @property
    def _effect_literal(self) -> GroundLiteral:
        return GroundLiteral._from_handle(lib.mimir_ground_conditional_effect_get_effect(self._handle), self._problem)

    @property
    def effect(self) -> GroundEffect:
        literal = self._effect_literal
        atom = literal.atom
        if literal.is_positive:
            return GroundEffect._from_parts(self._problem, add_atoms=(atom,), delete_atoms=())
        return GroundEffect._from_parts(self._problem, add_atoms=(), delete_atoms=(atom,))


class GroundConditionalNumericEffect(_GroundConditionalEffectBase):
    """A grounded conditional effect: condition ground literals plus one ground numeric update."""

    @property
    def effect(self) -> GroundEffect:
        updates = cast(tuple[GroundNumericUpdate, ...], _read_updates(self._handle, self._problem))
        return GroundEffect._from_parts(self._problem, add_atoms=(), delete_atoms=(), numeric_effects=updates)

# =============================================================================
# Action (schema) and GroundAction
# =============================================================================


class Action(_Handle):
    """A lifted action schema."""

    @property
    def name(self) -> str:
        return _required_string(lib.mimir_action_get_name(self._handle), "Action.name")

    @property
    def arity(self) -> int:
        return int(lib.mimir_action_get_arity(self._handle))

    @property
    def parameters(self) -> tuple[Variable, ...]:
        n = lib.mimir_action_get_parameter_count(self._handle)
        return tuple(Variable._from_handle(lib.mimir_action_get_parameter(self._handle, i), self._owner) for i in range(n))

    @property
    def fluent_preconditions(self) -> tuple[Literal, ...]:
        n = lib.mimir_action_get_fluent_precondition_count(self._handle)
        return tuple(Literal._from_handle(lib.mimir_action_get_fluent_precondition(self._handle, i), self._owner) for i in range(n))

    @property
    def static_preconditions(self) -> tuple[Literal, ...]:
        n = lib.mimir_action_get_static_precondition_count(self._handle)
        return tuple(Literal._from_handle(lib.mimir_action_get_static_precondition(self._handle, i), self._owner) for i in range(n))

    @property
    def derived_preconditions(self) -> tuple[Literal, ...]:
        n = lib.mimir_action_get_derived_precondition_count(self._handle)
        return tuple(Literal._from_handle(lib.mimir_action_get_derived_precondition(self._handle, i), self._owner) for i in range(n))

    @property
    def precondition(self) -> "ConjunctiveCondition":
        if self._owner is None:
            raise RuntimeError("action has no owner")
        return ConjunctiveCondition._from_parts(
            None,
            self._owner,
            list(self.parameters),
            list(self.static_preconditions + self.fluent_preconditions + self.derived_preconditions),
            _read_comparisons(self._handle, self._owner),
        )

    @cached_property
    def effect(self) -> Effect:
        if self._owner is None:
            raise RuntimeError("action has no owner")
        n = lib.mimir_action_get_effect_literal_count(self._handle)
        literals = tuple(Literal._from_handle(lib.mimir_action_get_effect_literal(self._handle, i), self._owner) for i in range(n))
        return Effect._from_parts(self.parameters, literals, self._owner, cast(tuple[NumericUpdate, ...], _read_updates(self._handle, self._owner)))

    @cached_property
    def conditional_effects(self) -> tuple[ConditionalEffect, ...]:
        n = lib.mimir_action_get_conditional_effect_count(self._handle)
        return tuple(ConditionalEffect._from_handle(lib.mimir_action_get_conditional_effect(self._handle, i), self._owner) for i in range(n))

    @cached_property
    def conditional_numeric_effects(self) -> tuple[ConditionalNumericEffect, ...]:
        n = lib.mimir_action_get_conditional_numeric_effect_count(self._handle)
        return tuple(
            ConditionalNumericEffect._from_handle(lib.mimir_action_get_conditional_numeric_effect(self._handle, i), self._owner)
            for i in range(n)
        )

    @cached_property
    def cost_expression(self) -> NumericExpression:
        return _wrap_numeric(lib.mimir_action_cost_expression(self._handle), self._owner)

    def __str__(self) -> str:
        return _required_string(lib.mimir_action_to_string(self._handle), "Action.__str__")

    def __repr__(self) -> str:
        return f"Action({self.name!r})"

class GroundAction(_Handle):
    """A grounded action: an action schema with concrete object arguments."""

    def __init__(self) -> None:
        raise TypeError("use Problem.ground_action() or Problem.action()")

    _problem: Problem

    @classmethod
    def _from_handle(
        cls,
        handle: int,
        owner: Domain | Problem | None = None,
    ) -> "GroundAction":
        value = cls._allocate_for_handle(handle)
        try:
            if not isinstance(owner, Problem):
                raise TypeError("ground actions require a problem owner")
            value._problem = owner
        except BaseException:
            free_handle(handle)
            raise
        value._initialize(handle, owner)
        return value

    @property
    def cost(self) -> float:
        return float(lib.mimir_ground_action_get_cost(self._handle))

    @cached_property
    def cost_expression(self) -> NumericExpression:
        return _wrap_numeric(lib.mimir_ground_action_cost_expression(self._handle), self._problem)

    @cached_property
    def schema(self) -> Action:
        return Action._from_handle(lib.mimir_ground_action_get_schema(self._handle), self._problem.domain)

    @property
    def problem(self) -> "Problem":
        return self._problem

    @cached_property
    def arguments(self) -> tuple[Object, ...]:
        n = lib.mimir_ground_action_get_argument_count(self._handle)
        return tuple(Object._from_handle(lib.mimir_ground_action_get_argument(self._handle, i), self._problem) for i in range(n))

    @cached_property
    def precondition(self) -> "GroundConjunctiveCondition":
        return GroundConjunctiveCondition._from_ground_action(self, self._problem)

    @cached_property
    def effect(self) -> GroundEffect:
        return GroundEffect._from_parts(self._problem, action=self)

    @cached_property
    def conditional_effects(self) -> tuple[GroundConditionalEffect, ...]:
        n = lib.mimir_ground_action_get_conditional_effect_count(self._handle)
        return tuple(GroundConditionalEffect._from_handle(lib.mimir_ground_action_get_conditional_effect(self._handle, i), self._problem) for i in range(n))

    @cached_property
    def conditional_numeric_effects(self) -> tuple[GroundConditionalNumericEffect, ...]:
        n = lib.mimir_ground_action_get_conditional_numeric_effect_count(self._handle)
        return tuple(
            GroundConditionalNumericEffect._from_handle(
                lib.mimir_ground_action_get_conditional_numeric_effect(self._handle, i), self._problem
            )
            for i in range(n)
        )

    def is_applicable(self, state: "State") -> bool:
        if not isinstance(state, State):
            raise TypeError("state must be a State")
        if state.problem != self._problem:
            raise ValueError("action and state belong to different problems")
        return relationship_value(
            lib.mimir_ground_action_is_applicable(self._handle, state._handle),
            "action and state belong to different problems",
        )

    def apply(self, state: "State") -> "State":
        if not isinstance(state, State):
            raise TypeError("state must be a State")
        if state.problem != self._problem:
            raise ValueError("action and state belong to different problems")
        if not self.is_applicable(state):
            raise ValueError("action is not applicable in this state")
        return self._apply_unchecked(state)

    def _apply_unchecked(self, state: "State") -> "State":
        new_handle = lib.mimir_ground_action_apply(self._handle, state._handle)
        if new_handle == 0:
            raise ValueError("action cannot be applied to this state")
        return State._from_handle(new_handle, self._problem)

    def __str__(self) -> str:
        return _required_string(lib.mimir_ground_action_to_string(self._handle), "GroundAction.__str__")

    def __repr__(self) -> str:
        return self.__str__()

# =============================================================================
# Domain
# =============================================================================


class Domain(_Handle):
    """A PDDL domain loaded from a file."""

    def __init__(self) -> None:
        raise TypeError("use Domain.from_file() or Domain.from_pddl()")

    # Schema-level values (functions, predicates) are owned by the domain even when reached through a problem.
    @property
    def _schema_domain(self) -> Domain:
        return self

    @classmethod
    def from_file(cls, path: str | os.PathLike[str]) -> "Domain":
        path_value = os.fspath(path)
        if not os.path.isfile(path_value):
            raise FileNotFoundError(path_value)
        handle = lib.mimir_load_domain(os.fsencode(path_value))
        if handle == 0:
            raise_last_error(f"could not load domain {path_value!r}")
        return cls._from_handle(handle)

    @classmethod
    def from_pddl(cls, text: str) -> "Domain":
        if not isinstance(text, str):
            raise TypeError("text must be str")
        handle = lib.mimir_load_domain_from_string(text.encode("utf-8"))
        if handle == 0:
            raise_last_error("could not parse domain PDDL")
        return cls._from_handle(handle)

    @property
    def name(self) -> str:
        return _required_string(lib.mimir_domain_get_name(self._handle), "Domain.name")

    @cached_property
    def requirements(self) -> tuple[str, ...]:
        n = lib.mimir_domain_get_requirement_count(self._handle)
        return tuple(_required_string(lib.mimir_domain_get_requirement(self._handle, i), "Domain.requirement") for i in range(n))

    @cached_property
    def expanded_requirements(self) -> tuple[str, ...]:
        n = lib.mimir_domain_get_expanded_requirement_count(self._handle)
        return tuple(
            _required_string(
                lib.mimir_domain_get_expanded_requirement(self._handle, i),
                "Domain.expanded_requirement",
            )
            for i in range(n)
        )

    @cached_property
    def type_hierarchy(self) -> Mapping[str, str | None]:
        n = lib.mimir_domain_get_type_count(self._handle)
        hierarchy: dict[str, str | None] = {}
        for index in range(n):
            name = _required_string(
                lib.mimir_domain_get_type_name(self._handle, index),
                "Domain.type_hierarchy key",
            )
            parent = take_string(lib.mimir_domain_get_type_parent(self._handle, index))
            if index == 0:
                if name != "object" or parent is not None:
                    raise RuntimeError("native type hierarchy must start with the object root")
            elif parent is None:
                raise RuntimeError(f"native type {name!r} has no parent")
            hierarchy[name] = parent
        return MappingProxyType(hierarchy)

    @cached_property
    def uses_typing(self) -> bool:
        return relationship_value(
            lib.mimir_domain_uses_typing(self._handle),
            "cannot query typing support on an invalid domain",
        )

    @cached_property
    def uses_equality(self) -> bool:
        return relationship_value(
            lib.mimir_domain_uses_equality(self._handle),
            "cannot query equality support on an invalid domain",
        )

    @cached_property
    def uses_conditional_effects(self) -> bool:
        return relationship_value(
            lib.mimir_domain_uses_conditional_effects(self._handle),
            "cannot query conditional-effect support on an invalid domain",
        )

    @cached_property
    def functions(self) -> tuple[NumericFunction, ...]:
        return tuple(NumericFunction._from_handle(lib.mimir_domain_get_function(self._handle, index), self)
                     for index in range(lib.mimir_domain_get_function_count(self._handle)))

    def function(self, name: str) -> NumericFunction:
        if not isinstance(name, str):
            raise TypeError("name must be str")
        for function in self.functions:
            if function.name.lower() == name.lower():
                return function
        raise ValueError(f"unknown numeric function {name!r}")

    @cached_property
    def predicates(self) -> tuple[Predicate, ...]:
        return self.static_predicates + self.fluent_predicates + self.derived_predicates

    @cached_property
    def static_predicates(self) -> tuple[Predicate, ...]:
        n = lib.mimir_domain_get_static_predicate_count(self._handle)
        return tuple(Predicate._from_handle(lib.mimir_domain_get_static_predicate(self._handle, i), self) for i in range(n))

    @cached_property
    def fluent_predicates(self) -> tuple[Predicate, ...]:
        n = lib.mimir_domain_get_fluent_predicate_count(self._handle)
        return tuple(Predicate._from_handle(lib.mimir_domain_get_fluent_predicate(self._handle, i), self) for i in range(n))

    @cached_property
    def derived_predicates(self) -> tuple[Predicate, ...]:
        n = lib.mimir_domain_get_derived_predicate_count(self._handle)
        return tuple(Predicate._from_handle(lib.mimir_domain_get_derived_predicate(self._handle, i), self) for i in range(n))

    @cached_property
    def actions(self) -> tuple[Action, ...]:
        n = lib.mimir_domain_get_action_count(self._handle)
        return tuple(Action._from_handle(lib.mimir_domain_get_action(self._handle, i), self) for i in range(n))

    @cached_property
    def constants(self) -> tuple[Object, ...]:
        n = lib.mimir_domain_get_constant_count(self._handle)
        return tuple(Object._from_handle(lib.mimir_domain_get_constant(self._handle, i), self) for i in range(n))

    def predicate(self, name: str) -> Predicate:
        if not isinstance(name, str):
            raise TypeError("name must be str")
        for predicate in self.predicates:
            if predicate.name == name:
                return predicate
        raise KeyError(name)

    def action(self, name: str) -> Action:
        if not isinstance(name, str):
            raise TypeError("name must be str")
        for action in self.actions:
            if action.name == name:
                return action
        raise KeyError(name)

    def constant(self, name: str) -> Object:
        if not isinstance(name, str):
            raise TypeError("name must be str")
        for constant in self.constants:
            if constant.name == name:
                return constant
        raise KeyError(name)

    def has_predicate(self, name: str) -> bool:
        if not isinstance(name, str):
            raise TypeError("name must be str")
        return any(predicate.name == name for predicate in self.predicates)

    def has_action(self, name: str) -> bool:
        if not isinstance(name, str):
            raise TypeError("name must be str")
        return any(action.name == name for action in self.actions)

    def has_constant(self, name: str) -> bool:
        if not isinstance(name, str):
            raise TypeError("name must be str")
        return any(constant.name == name for constant in self.constants)

    def is_type_compatible(self, child: str, parent: str) -> bool:
        if not isinstance(child, str) or not isinstance(parent, str):
            raise TypeError("child and parent must be str")
        return relationship_value(
            lib.mimir_domain_is_compatible(
                self._handle, child.encode("utf-8"), parent.encode("utf-8")
            ),
            "cannot query type compatibility on an invalid domain",
        )

    def __repr__(self) -> str:
        return f"Domain({self.name!r})"


# =============================================================================
# Problem
# =============================================================================


class Problem(_Handle):
    """A PDDL problem instance."""

    def __init__(self) -> None:
        raise TypeError("use Problem.from_file(), Problem.from_pddl(), or Problem.from_files()")

    _domain: Domain

    @classmethod
    def _from_handle(
        cls,
        handle: int,
        owner: Domain | Problem | None = None,
    ) -> "Problem":
        value = cls._allocate_for_handle(handle)
        try:
            if not isinstance(owner, Domain):
                raise TypeError("problems require a domain owner")
            value._domain = owner
        except BaseException:
            free_handle(handle)
            raise
        value._initialize(handle, owner)
        return value

    @classmethod
    def from_file(
        cls,
        domain: Domain,
        path: str | os.PathLike[str],
        *,
        generator: ActionGenerator = "lifted",
    ) -> "Problem":
        if not isinstance(domain, Domain):
            raise TypeError("domain must be a Domain")
        path_value = os.fspath(path)
        if not os.path.isfile(path_value):
            raise FileNotFoundError(path_value)
        generator_value = _validate_generator(generator)
        handle = lib.mimir_load_problem(
            domain._handle,
            os.fsencode(path_value),
            generator_value.encode("utf-8"),
        )
        if handle == 0:
            raise_last_error(f"could not load problem {path_value!r}")
        return cls._from_handle(handle, domain)

    @classmethod
    def from_pddl(
        cls,
        domain: Domain,
        text: str,
        *,
        generator: ActionGenerator = "lifted",
    ) -> "Problem":
        if not isinstance(domain, Domain):
            raise TypeError("domain must be a Domain")
        if not isinstance(text, str):
            raise TypeError("text must be str")
        generator_value = _validate_generator(generator)
        handle = lib.mimir_load_problem_from_string(
            domain._handle,
            text.encode("utf-8"),
            generator_value.encode("utf-8"),
        )
        if handle == 0:
            raise_last_error("could not parse problem PDDL")
        return cls._from_handle(handle, domain)

    @classmethod
    def from_files(
        cls,
        domain_path: str | os.PathLike[str],
        problem_path: str | os.PathLike[str],
        *,
        generator: ActionGenerator = "lifted",
    ) -> "Problem":
        domain = Domain.from_file(domain_path)
        return cls.from_file(domain, problem_path, generator=generator)

    @property
    def name(self) -> str:
        return _required_string(lib.mimir_problem_get_name(self._handle), "Problem.name")

    @property
    def domain(self) -> Domain:
        return self._domain

    @property
    def _schema_domain(self) -> Domain:
        return self._domain

    @property
    def generator(self) -> ActionGenerator:
        value = _required_string(
            lib.mimir_problem_get_generator(self._handle),
            "Problem.generator",
        )
        return _validate_generator(value)

    @cached_property
    def requirements(self) -> tuple[str, ...]:
        n = lib.mimir_problem_get_requirement_count(self._handle)
        return tuple(
            _required_string(
                lib.mimir_problem_get_requirement(self._handle, i),
                "Problem.requirement",
            )
            for i in range(n)
        )

    @cached_property
    def declared_objects(self) -> tuple[Object, ...]:
        n = lib.mimir_problem_get_declared_object_count(self._handle)
        return tuple(
            Object._from_handle(
                lib.mimir_problem_get_declared_object(self._handle, i), self
            )
            for i in range(n)
        )

    @cached_property
    def all_objects(self) -> tuple[Object, ...]:
        n = lib.mimir_problem_get_all_object_count(self._handle)
        return tuple(
            Object._from_handle(lib.mimir_problem_get_all_object(self._handle, i), self)
            for i in range(n)
        )

    @property
    def initial_state(self) -> "State":
        handle = lib.mimir_problem_get_initial_state(self._handle)
        if handle == 0:
            raise RuntimeError("Failed to retrieve initial state")
        return State._from_handle(handle, self)

    @cached_property
    def initial_static_atoms(self) -> tuple[GroundAtom, ...]:
        n = lib.mimir_problem_get_static_initial_atom_count(self._handle)
        return tuple(GroundAtom._from_handle(lib.mimir_problem_get_static_initial_atom(self._handle, i), self) for i in range(n))

    @cached_property
    def initial_fluent_atoms(self) -> tuple[GroundAtom, ...]:
        n = lib.mimir_problem_get_fluent_initial_atom_count(self._handle)
        return tuple(GroundAtom._from_handle(lib.mimir_problem_get_fluent_initial_atom(self._handle, i), self) for i in range(n))

    @cached_property
    def initial_atoms(self) -> tuple[GroundAtom, ...]:
        return self.initial_static_atoms + self.initial_fluent_atoms

    @cached_property
    def goal(self) -> "GroundConjunctiveCondition":
        n = lib.mimir_problem_get_goal_literal_count(self._handle)
        literals = [GroundLiteral._from_handle(lib.mimir_problem_get_goal_literal(self._handle, i), self) for i in range(n)]
        return GroundConjunctiveCondition._from_literals(literals, self, _read_comparisons(self._handle, self))

    def has_object(self, name: str) -> bool:
        if not isinstance(name, str):
            raise TypeError("name must be str")
        h = lib.mimir_problem_get_object_by_name(self._handle, name.encode("utf-8"))
        if h == 0:
            return False
        free_handle(h)
        return True

    def object(self, name: str) -> Object:
        if not isinstance(name, str):
            raise TypeError("name must be str")
        h = lib.mimir_problem_get_object_by_name(self._handle, name.encode("utf-8"))
        if h == 0:
            raise KeyError(name)
        return Object._from_handle(h, self)

    def variable(self, name: str, type_name: str = "object") -> Variable:
        if not isinstance(name, str) or not isinstance(type_name, str):
            raise TypeError("name and type_name must be str")
        handle = lib.mimir_problem_new_variable(
            self._handle, name.encode("utf-8"), type_name.encode("utf-8")
        )
        if handle == 0:
            raise ValueError(f"failed to create variable {name!r}")
        return Variable._from_handle(handle, self)

    def lifted_atom(self, predicate: Predicate, *terms: "Term") -> Atom:
        if not isinstance(predicate, Predicate):
            raise TypeError("predicate must be a Predicate")
        if any(not isinstance(term, (Object, Variable)) for term in terms):
            raise TypeError("terms must be Object or Variable values")
        pointer, _array = _handle_array([term._handle for term in terms])
        handle = lib.mimir_problem_new_atom(
            self._handle, predicate._handle, pointer, len(terms)
        )
        if handle == 0:
            raise ValueError("predicate or terms do not belong to this problem")
        return Atom._from_handle(handle, self)

    def ground_atom(self, predicate: Predicate, *objects: Object) -> GroundAtom:
        if not isinstance(predicate, Predicate):
            raise TypeError("predicate must be a Predicate")
        if any(not isinstance(value, Object) for value in objects):
            raise TypeError("objects must be Object values")
        pointer, _array = _handle_array([value._handle for value in objects])
        handle = lib.mimir_problem_register_fact(
            self._handle, predicate._handle, pointer, len(objects)
        )
        if handle == 0:
            raise ValueError("predicate or objects do not belong to this problem")
        return GroundAtom._from_handle(handle, self)

    def literal(self, atom: Atom, *, positive: bool = True) -> Literal:
        if not isinstance(atom, Atom):
            raise TypeError("atom must be an Atom")
        if not isinstance(positive, bool):
            raise TypeError("positive must be bool")
        handle = lib.mimir_problem_new_literal(
            self._handle, atom._handle, 0 if positive else 1
        )
        if handle == 0:
            raise ValueError("atom does not belong to this problem")
        return Literal._from_handle(handle, self)

    def ground_literal(
        self,
        atom: GroundAtom,
        *,
        positive: bool = True,
    ) -> GroundLiteral:
        if not isinstance(atom, GroundAtom):
            raise TypeError("atom must be a GroundAtom")
        if not isinstance(positive, bool):
            raise TypeError("positive must be bool")
        handle = lib.mimir_problem_new_ground_literal(
            self._handle, atom._handle, 0 if positive else 1
        )
        if handle == 0:
            raise ValueError("ground atom does not belong to this problem")
        return GroundLiteral._from_handle(handle, self)

    def condition(
        self,
        *literals: Literal,
        variables: Iterable[Variable] = (),
        numeric_conditions: Iterable[NumericComparison] = (),
    ) -> "ConjunctiveCondition":
        comparison_values = tuple(numeric_conditions)
        variable_values = list(variables)
        literal_values = list(literals)
        if any(not isinstance(value, Variable) for value in variable_values):
            raise TypeError("variables must contain Variable values")
        if any(not isinstance(value, Literal) for value in literal_values):
            raise TypeError("literals must contain Literal values")
        if len(set(variable_values)) != len(variable_values):
            raise ValueError("variables must not contain duplicates")
        variable_pointer, _variable_array = _handle_array(
            [value._handle for value in variable_values]
        )
        literal_pointer, _literal_array = _handle_array(
            [value._handle for value in literal_values]
        )
        comparison_ptr, _comparison_array = _handle_array(_comparison_handles(comparison_values))
        handle = lib.mimir_problem_new_conjunctive_condition(
            self._handle,
            variable_pointer,
            len(variable_values),
            literal_pointer,
            len(literal_values), comparison_ptr, len(comparison_values),
        )
        return ConjunctiveCondition._from_parts(
            handle, self, variable_values, literal_values, comparison_values
        )

    def ground_condition(
        self,
        *items: GroundAtom | GroundLiteral,
        numeric_conditions: Iterable[NumericComparison] = (),
    ) -> "GroundConjunctiveCondition":
        comparison_values = tuple(numeric_conditions)
        if any(not isinstance(item, (GroundAtom, GroundLiteral)) for item in items):
            raise TypeError("items must be GroundAtom or GroundLiteral values")
        literals = [
            self.ground_literal(item) if isinstance(item, GroundAtom) else item
            for item in items
        ]
        return GroundConjunctiveCondition._from_literals(literals, self, comparison_values)

    def ground_action(self, action: Action, *objects: Object) -> GroundAction:
        if not isinstance(action, Action):
            raise TypeError("action must be an Action")
        if any(not isinstance(value, Object) for value in objects):
            raise TypeError("objects must be Object values")
        pointer, _array = _handle_array([value._handle for value in objects])
        handle = lib.mimir_problem_new_ground_action(
            self._handle, action._handle, pointer, len(objects)
        )
        if handle == 0:
            raise ValueError("action or objects do not belong to this problem")
        return GroundAction._from_handle(handle, self)

    def state(self, *fluent_atoms: GroundAtom, numeric_values: Mapping[GroundFunctionCall, float] | None = None) -> "State":
        atoms = tuple(fluent_atoms)
        if any(not isinstance(atom, GroundAtom) for atom in atoms):
            raise TypeError("fluent_atoms must contain GroundAtom values")
        if any(not atom.is_fluent for atom in atoms):
            raise ValueError("state() accepts fluent atoms only")
        ptr, _array = _handle_array([atom._handle for atom in atoms])
        if numeric_values is not None and not isinstance(numeric_values, Mapping):
            raise TypeError("numeric_values must be a mapping")
        entries = tuple(numeric_values.items()) if numeric_values is not None else ()
        for field, value in entries:
            if not isinstance(field, GroundFunctionCall):
                raise TypeError("numeric_values keys must be GroundFunctionCall values")
            if isinstance(value, bool) or not isinstance(value, (int, float)):
                raise TypeError("numeric_values must contain int or float values")
        numeric_ptr, _numeric_array = _handle_array([field._handle for field, value in entries])
        numbers = (ctypes.c_double * len(entries))(*(value for field, value in entries)) if entries else None
        handle = lib.mimir_problem_new_state(self._handle, ptr, len(atoms), numeric_ptr, numbers, len(entries))
        if handle == 0:
            raise_last_error("could not create state")
        return State._from_handle(handle, self)

    def lifted_function_call(self, function: NumericFunction, *arguments: Term) -> FunctionCall:
        if not isinstance(function, NumericFunction):
            raise TypeError("function must be a NumericFunction")
        if any(not isinstance(argument, (Object, Variable)) for argument in arguments):
            raise TypeError("arguments must be variables or objects")
        pointer, _array = _handle_array([argument._handle for argument in arguments])
        handle = int(lib.mimir_problem_lifted_function_call(self._handle, function._handle, pointer, len(arguments)))
        return FunctionCall._from_handle(handle, self)

    def function_call(self, function_name: str, *object_names: str) -> GroundFunctionCall:
        if not isinstance(function_name, str) or any(not isinstance(name, str) for name in object_names):
            raise TypeError("function and object names must be str")
        if "\0" in function_name or any("\0" in name for name in object_names):
            raise ValueError("names cannot contain null characters")
        encoded = tuple(name.encode("utf-8") for name in object_names)
        array = (ctypes.c_char_p * len(encoded))(*encoded)
        handle = int(lib.mimir_problem_function_call(self._handle, function_name.encode("utf-8"), array, len(encoded)))
        return GroundFunctionCall._from_handle(handle, self)

    def atom(self, predicate_name: str, *object_names: str) -> GroundAtom:
        if not isinstance(predicate_name, str) or any(not isinstance(name, str) for name in object_names):
            raise TypeError("predicate and object names must be str")
        predicate = self.domain.predicate(predicate_name)
        return self.ground_atom(predicate, *(self.object(name) for name in object_names))

    def action(self, action_name: str, *object_names: str) -> GroundAction:
        if not isinstance(action_name, str) or any(not isinstance(name, str) for name in object_names):
            raise TypeError("action and object names must be str")
        schema = self.domain.action(action_name)
        return self.ground_action(schema, *(self.object(name) for name in object_names))

    def __repr__(self) -> str:
        return f"Problem({self.name!r})"


def _handle_array(handles: Sequence[int]) -> tuple[ctypes.c_void_p, object | None]:
    """Convert a list of ints to a (ctypes pointer, backing array) tuple.

    The backing array must be retained by the caller until the native call
    returns; assign it to a variable (e.g. ``ptr, arr = _handle_array(...)``).
    """
    n = len(handles)
    if n == 0:
        return ctypes.c_void_p(0), None
    ArrType = ctypes.c_int * n
    arr = ArrType(*handles)
    return ctypes.cast(arr, ctypes.c_void_p), arr


# =============================================================================
# State
# =============================================================================


class State(_Handle):
    """A state with complete static, fluent, and derived atom views."""

    def __init__(self) -> None:
        raise TypeError("use Problem.state(), Problem.initial_state, or search results")

    _problem: Problem
    _applicable_actions: tuple[GroundAction, ...] | None
    _hash_code: int | None

    @classmethod
    def _from_handle(
        cls,
        handle: int,
        owner: Domain | Problem | None = None,
    ) -> "State":
        value = cls._allocate_for_handle(handle)
        try:
            if not isinstance(owner, Problem):
                raise TypeError("states require a problem owner")
            value._problem = owner
            value._applicable_actions = None
            value._hash_code = None
        except BaseException:
            free_handle(handle)
            raise
        value._initialize(handle, owner)
        return value

    @property
    def problem(self) -> Problem:
        return self._problem

    @cached_property
    def static_atoms(self) -> tuple[GroundAtom, ...]:
        return self._problem.initial_static_atoms

    @cached_property
    def fluent_atoms(self) -> tuple[GroundAtom, ...]:
        count = lib.mimir_state_get_fluent_atom_count(self._handle)
        return tuple(
            GroundAtom._from_handle(lib.mimir_state_get_fluent_atom(self._handle, index), self._problem)
            for index in range(count)
        )

    @cached_property
    def derived_atoms(self) -> tuple[GroundAtom, ...]:
        count = lib.mimir_state_get_derived_atom_count(self._handle)
        return tuple(
            GroundAtom._from_handle(
                lib.mimir_state_get_derived_atom(self._handle, index), self._problem
            )
            for index in range(count)
        )

    @cached_property
    def atoms(self) -> tuple[GroundAtom, ...]:
        return self.static_atoms + self.fluent_atoms + self.derived_atoms

    @overload
    def value(self, expression: GroundAtom) -> bool: ...

    @overload
    def value(self, expression: NumericExpression) -> float: ...

    def value(self, expression: GroundAtom | NumericExpression) -> bool | float:
        if isinstance(expression, GroundAtom):
            return self.contains(expression)
        if isinstance(expression, NumericExpression):
            if not expression.is_ground:
                raise ValueError("state.value requires a grounded expression")
            return float(lib.mimir_state_numeric_value(self._handle, expression._handle))
        raise TypeError("value requires a GroundAtom or grounded NumericExpression")

    def contains(self, ground_atom: GroundAtom) -> bool:
        if not isinstance(ground_atom, GroundAtom):
            raise TypeError("ground_atom must be a GroundAtom")
        if ground_atom._owner != self._problem:
            raise ValueError("ground atom and state belong to different problems")
        if ground_atom.is_fluent:
            result = lib.mimir_state_contains_fluent(self._handle, ground_atom._handle)
            return relationship_value(result, "ground atom and state belong to different problems")
        if ground_atom.is_static:
            result = lib.mimir_state_contains_static(self._handle, ground_atom._handle)
            return relationship_value(result, "ground atom and state belong to different problems")
        if ground_atom.is_derived:
            result = lib.mimir_state_contains_derived(self._handle, ground_atom._handle)
            return relationship_value(result, "ground atom and state belong to different problems")
        return False

    def contains_all(self, ground_atoms: Iterable[GroundAtom]) -> bool:
        return all(self.contains(a) for a in ground_atoms)

    def contains_none(self, ground_atoms: Iterable[GroundAtom]) -> bool:
        return not any(self.contains(a) for a in ground_atoms)

    def literal_holds(self, literal: GroundLiteral) -> bool:
        if not isinstance(literal, GroundLiteral):
            raise TypeError("literal must be a GroundLiteral")
        if literal._owner != self._problem:
            raise ValueError("literal and state belong to different problems")
        atom = literal.atom
        holds = self.contains(atom)
        return holds if literal.is_positive else not holds

    def literals_hold(self, literals: Iterable[GroundLiteral]) -> bool:
        for lit in literals:
            if not self.literal_holds(lit):
                return False
        return True

    def holds(
        self,
        value: GroundAtom | GroundLiteral | GroundConjunctiveCondition | NumericComparison,
    ) -> bool:
        if isinstance(value, NumericComparison):
            return bool(lib.mimir_state_holds_comparison(self._handle, value._handle))
        if isinstance(value, GroundAtom):
            return self.contains(value)
        if isinstance(value, GroundLiteral):
            return self.literal_holds(value)
        if isinstance(value, GroundConjunctiveCondition):
            return value.holds(self)
        raise TypeError("value must be a GroundAtom, GroundLiteral, GroundConjunctiveCondition, or NumericComparison")

    def applicable_actions(
        self,
    ) -> tuple[GroundAction, ...]:
        if self._applicable_actions is not None:
            return self._applicable_actions
        list_handle = lib.mimir_state_generate_applicable_actions(
            self._handle, self._problem._handle
        )
        if list_handle == 0:
            raise ValueError("state does not belong to this problem")
        try:
            actions = tuple(
                GroundAction._from_handle(
                    lib.mimir_action_list_get(list_handle, index), self._problem
                )
                for index in range(lib.mimir_action_list_count(list_handle))
            )
        finally:
            free_handle(list_handle)
        self._applicable_actions = actions
        return actions

    def successor_states(self) -> tuple[tuple[GroundAction, State], ...]:
        """Expand all applicable actions in native code and return ordered pairs.

        Action order matches ``applicable_actions()``, regardless of which
        method is called first. Equal successor states are retained for each
        action. Actions are cached; successor states are not.
        """
        cached_actions = self._applicable_actions
        if cached_actions == ():
            return ()
        action_ptr, action_array = _handle_array(
            [action._handle for action in cached_actions]
            if cached_actions is not None else []
        )
        list_handle = lib.mimir_state_generate_successors(
            self._handle,
            self._problem._handle,
            action_ptr,
            len(cached_actions) if cached_actions is not None else -1,
        )
        if list_handle == 0:
            raise_last_error("could not generate successor states")
        try:
            count = lib.mimir_transition_list_count(list_handle)
            action_handles = (ctypes.c_int * count)()
            state_handles = (ctypes.c_int * count)()
            try:
                copied = lib.mimir_transition_list_copy_handles(
                    list_handle,
                    action_handles if cached_actions is None else None,
                    state_handles,
                    count,
                )
                if copied != count:
                    raise RuntimeError("native successor count changed during transfer")
                transitions = []
                for index in range(count):
                    if cached_actions is None:
                        handle = action_handles[index]
                        action_handles[index] = 0
                        action = GroundAction._from_handle(handle, self._problem)
                    else:
                        action = cached_actions[index]
                    # _from_handle adopts the handle even when construction fails.
                    handle = state_handles[index]
                    state_handles[index] = 0
                    successor = State._from_handle(handle, self._problem)
                    transitions.append((action, successor))
                result = tuple(transitions)
                if cached_actions is None:
                    self._applicable_actions = tuple(action for action, _ in result)
                return result
            except BaseException:
                for handle in action_handles:
                    free_handle(handle)
                for handle in state_handles:
                    free_handle(handle)
                raise
        finally:
            free_handle(list_handle)

    def __hash__(self) -> int:
        if self._hash_code is None:
            self._hash_code = value_hash(self._handle)
        return self._hash_code

    def __iter__(self) -> Iterator[GroundAtom]:
        return iter(self.atoms)

    def __len__(self) -> int:
        return len(self.atoms)

    def __contains__(self, atom: object) -> bool:
        return isinstance(atom, GroundAtom) and self.contains(atom)

    def __str__(self) -> str:
        return _required_string(lib.mimir_state_to_string(self._handle), "State.__str__")

    def __repr__(self) -> str:
        return self.__str__()

# =============================================================================
# Conjunctive conditions
# =============================================================================


class ConjunctiveCondition(_Handle):
    """A lifted conjunctive condition."""

    _numeric_conditions: tuple[NumericComparison, ...]

    def __init__(self) -> None:
        raise TypeError("use Problem.condition()")

    _problem: Problem | None
    _parameters: list[Variable]
    _literals: list[Literal]

    @classmethod
    def _from_parts(
        cls,
        handle: int | None,
        owner: Domain | Problem,
        parameters: Sequence[Variable],
        literals: Sequence[Literal],
        numeric_conditions: Sequence[NumericComparison] = (),
    ) -> "ConjunctiveCondition":
        if handle is None:
            self = object.__new__(cls)
        else:
            self = cls._allocate_for_handle(handle)

        try:
            if handle is None:
                self._handle = 0
                self._owner = owner
                self._finalizer = None
            self._problem = owner if isinstance(owner, Problem) else None
            self._parameters = list(parameters)
            self._literals = list(literals)
            self._numeric_conditions = tuple(numeric_conditions)
            owned_values = [*self._parameters, *self._literals]
            if any(value._owner != owner for value in owned_values):
                raise ValueError("condition values must have the same owner")
        except BaseException:
            if handle is not None:
                free_handle(handle)
            raise

        if handle is not None:
            self._initialize(handle, owner)
        return self

    @property
    def parameters(self) -> tuple[Variable, ...]:
        return tuple(self._parameters)

    @property
    def numeric_conditions(self) -> tuple[NumericComparison, ...]:
        return self._numeric_conditions

    @property
    def literals(self) -> tuple[Literal, ...]:
        return tuple(self._literals)

    def bindings(
        self,
        state: State,
        *,
        limit: int | None = None,
    ) -> tuple[dict[Variable, Object], ...]:
        """Enumerate variable bindings under which this condition holds in ``state``."""
        if not isinstance(state, State):
            raise TypeError("state must be a State")
        if limit is None:
            native_limit = -1
        else:
            if isinstance(limit, bool) or not isinstance(limit, int):
                raise TypeError("limit must be int or None")
            if limit < 0:
                raise ValueError("limit must be nonnegative")
            if limit == 0:
                return ()
            native_limit = limit
        problem = state.problem
        if self._problem is not None and self._problem != problem:
            raise ValueError("condition and state belong to different problems")
        if self._problem is None and isinstance(self._owner, Domain):
            mapped_parameters = [
                problem.variable(variable.name, variable.type_name)
                for variable in self._parameters
            ]
            variable_map = dict(zip(self._parameters, mapped_parameters))
            mapped_literals: list[Literal] = []
            for literal in self._literals:
                atom = literal.atom
                terms: list[Term] = []
                for term in atom.arguments:
                    if isinstance(term, Variable):
                        if term not in variable_map:
                            raise ValueError("condition contains an undeclared variable")
                        terms.append(variable_map[term])
                    else:
                        terms.append(term)
                mapped_atom = problem.lifted_atom(atom.predicate, *terms)
                mapped_literals.append(problem.literal(mapped_atom, positive=literal.is_positive))
            def map_numeric(expression: NumericExpression) -> NumericExpression:
                if isinstance(expression, NumericConstant):
                    return expression
                if isinstance(expression, FunctionCall):
                    terms: list[Term] = [variable_map[term] if isinstance(term, Variable) else term
                                         for term in expression.arguments]
                    return problem.lifted_function_call(expression.function, *terms)
                if isinstance(expression, NumericBinaryExpression):
                    return map_numeric(expression.left)._binary(expression.operator, map_numeric(expression.right))
                raise ValueError("schema comparison contains an unsupported numeric expression")

            mapped_comparisons = [map_numeric(c.left)._compare(c.operator, map_numeric(c.right))
                                  for c in self.numeric_conditions]
            mapped = problem.condition(*mapped_literals, variables=mapped_parameters, numeric_conditions=mapped_comparisons)
            mapped_bindings = mapped.bindings(state, limit=limit)
            return tuple(
                {
                    original: binding[mapped_parameter]
                    for original, mapped_parameter in zip(self._parameters, mapped_parameters)
                }
                for binding in mapped_bindings
            )
        if self._handle == 0:
            backed = problem.condition(*self._literals, variables=self._parameters, numeric_conditions=self.numeric_conditions)
            return backed.bindings(state, limit=limit)
        list_handle = lib.mimir_conjunctive_condition_ground(
            self._handle, state._handle, native_limit
        )
        if list_handle == 0:
            raise ValueError("condition cannot be grounded against this state")
        try:
            count = lib.mimir_binding_list_count(list_handle)
            results: list[dict[Variable, Object]] = []
            for i in range(count):
                size = lib.mimir_binding_list_get_binding_size(list_handle, i)
                if size != len(self._parameters):
                    raise RuntimeError("native binding size does not match condition parameters")
                objects = [
                    Object._from_handle(lib.mimir_binding_list_get_object(list_handle, i, j), problem)
                    for j in range(size)
                ]
                results.append(dict(zip(self._parameters, objects)))
            return tuple(results)
        finally:
            free_handle(list_handle)

    def groundings(
        self,
        state: State,
        *,
        limit: int | None = None,
        omit_predicates: Iterable[Predicate] = (),
    ) -> tuple[GroundConjunctiveCondition, ...]:
        """Return satisfying ground instances of this condition in ``state``.

        Omitted predicates still constrain the enumerated bindings, but their
        literals are not included in the returned conditions.
        """
        if not isinstance(state, State):
            raise TypeError("state must be a State")
        omitted = tuple(omit_predicates)
        if any(not isinstance(predicate, Predicate) for predicate in omitted):
            raise TypeError("omit_predicates must contain Predicate values")
        if any(predicate._owner != state.problem.domain for predicate in omitted):
            raise ValueError(
                "omit_predicates contains a predicate from a different domain"
            )

        problem = state.problem
        results: list[GroundConjunctiveCondition] = []
        for binding in self.bindings(state, limit=limit):
            literals: list[GroundLiteral] = []
            for literal in self._literals:
                atom = literal.atom
                if atom.predicate in omitted:
                    continue
                objects = tuple(
                    binding[term] if isinstance(term, Variable) else term
                    for term in atom.arguments
                )
                ground_atom = problem.ground_atom(atom.predicate, *objects)
                literals.append(
                    problem.ground_literal(
                        ground_atom,
                        positive=literal.is_positive,
                    )
                )
            results.append(problem.ground_condition(*literals))
        return tuple(results)

    def __iter__(self) -> Iterator[Literal]:
        return iter(self._literals)

    def __len__(self) -> int:
        return len(self._literals)

    def __str__(self) -> str:
        parts = [str(literal) for literal in self._literals]
        return "(and " + " ".join(parts) + ")"

    def __repr__(self) -> str:
        return self.__str__()

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ConjunctiveCondition):
            return False
        if (self._handle == 0) != (other._handle == 0):
            return False
        if self._handle != 0:
            return value_equals(self._handle, other._handle)
        return (
            self._owner == other._owner
            and self._parameters == other._parameters
            and self._literals == other._literals
            and self.numeric_conditions == other.numeric_conditions
        )

    def __hash__(self) -> int:
        if self._handle != 0:
            return value_hash(self._handle)
        return hash((
            "ConjunctiveCondition", self._owner,
            tuple(self._parameters), tuple(self._literals), self.numeric_conditions,
        ))


class GroundConjunctiveCondition(_NativeOwner):
    """A grounded conjunctive condition: a list of ground literals attached to a problem."""

    __slots__ = ("_literals", "_problem", "_handle", "_finalizer", "_numeric_conditions", "__weakref__")

    _literals: list[GroundLiteral]
    _problem: Problem
    _handle: int
    _finalizer: _Finalizer

    _numeric_conditions: tuple[GroundNumericComparison, ...]

    def __init__(self) -> None:
        raise TypeError("use Problem.ground_condition()")

    @classmethod
    def _from_literals(
        cls, literals: Sequence[GroundLiteral], problem: Problem, numeric_conditions: Sequence[NumericComparison] = ()
    ) -> "GroundConjunctiveCondition":
        if any(literal._owner != problem for literal in literals):
            raise ValueError("ground literals must belong to this problem")
        literal_values = list(literals)
        ptr, _array = _handle_array([literal._handle for literal in literal_values])
        comparison_values = tuple(numeric_conditions)
        comparison_ptr, _comparison_array = _handle_array(_comparison_handles(comparison_values))
        grounded: list[GroundNumericComparison] = []
        for comparison in comparison_values:
            if not isinstance(comparison, GroundNumericComparison):
                raise ValueError("ground numeric conditions cannot contain variables")
            grounded.append(comparison)
        self = object.__new__(cls)
        handle = lib.mimir_goal_create(problem._handle, ptr, len(literal_values), comparison_ptr, len(grounded))
        finalizer = _create_finalizer(self, free_handle, handle)
        try:
            self._literals = literal_values
            self._numeric_conditions = tuple(grounded)
            self._problem = problem
            self._handle = handle
            self._finalizer = finalizer
        except BaseException:
            finalizer()
            raise
        return self

    @property
    def problem(self) -> Problem:
        return self._problem

    @property
    def numeric_conditions(self) -> tuple[GroundNumericComparison, ...]:
        return self._numeric_conditions

    @property
    def literals(self) -> tuple[GroundLiteral, ...]:
        return tuple(self._literals)

    # Bitboard-encoded precondition types: (count_fn_name, get_fn_name, polarity).
    _PRECONDITION_KINDS = (
        ("mimir_ground_action_get_positive_fluent_precond_count",
         "mimir_ground_action_get_positive_fluent_precond", True),
        ("mimir_ground_action_get_negative_fluent_precond_count",
         "mimir_ground_action_get_negative_fluent_precond", False),
        ("mimir_ground_action_get_positive_static_precond_count",
         "mimir_ground_action_get_positive_static_precond", True),
        ("mimir_ground_action_get_negative_static_precond_count",
         "mimir_ground_action_get_negative_static_precond", False),
    )

    @staticmethod
    def _from_ground_action(action: GroundAction, problem: Problem) -> "GroundConjunctiveCondition":
        result: list[GroundLiteral] = []
        h = action._handle
        for count_name, get_name, polarity in GroundConjunctiveCondition._PRECONDITION_KINDS:
            count_fn = getattr(lib, count_name)
            get_fn = getattr(lib, get_name)
            for i in range(count_fn(h)):
                fact_handle = get_fn(h, i)
                if fact_handle:
                    result.append(problem.ground_literal(GroundAtom._from_handle(fact_handle, problem), positive=polarity))

        # Derived preconditions are already Literal<Fact<Derived>>.
        n_d = lib.mimir_ground_action_get_derived_precondition_count(h)
        for i in range(n_d):
            lit_h = lib.mimir_ground_action_get_derived_precondition(h, i)
            if lit_h:
                result.append(GroundLiteral._from_handle(lit_h, problem))

        return GroundConjunctiveCondition._from_literals(result, problem, _read_comparisons(action._handle, problem))

    def lift(self, add_inequalities: bool = False) -> ConjunctiveCondition:
        if not isinstance(add_inequalities, bool):
            raise TypeError("add_inequalities must be bool")
        problem = self._problem
        variable_map: dict[Object, Variable] = {}
        lifted_literals: list[Literal] = []

        for literal in self:
            predicate = literal.atom.predicate
            polarity = literal.is_positive
            lifted_terms: list[Variable] = []

            for obj in literal.atom.arguments:
                if obj not in variable_map:
                    variable_id = len(variable_map)
                    variable_map[obj] = problem.variable(f"?x{variable_id}", obj.type_name)
                lifted_terms.append(variable_map[obj])

            lifted_atom = problem.lifted_atom(predicate, *lifted_terms)
            lifted_literals.append(problem.literal(lifted_atom, positive=polarity))

        def lift_numeric(expression: NumericExpression) -> NumericExpression:
            if isinstance(expression, NumericConstant):
                return expression
            if isinstance(expression, GroundFunctionCall):
                for obj in expression.arguments:
                    if obj not in variable_map:
                        variable_map[obj] = problem.variable(f"?x{len(variable_map)}", obj.type_name)
                return problem.lifted_function_call(expression.function, *(variable_map[obj] for obj in expression.arguments))
            if isinstance(expression, NumericBinaryExpression):
                return lift_numeric(expression.left)._binary(expression.operator, lift_numeric(expression.right))
            raise ValueError("expected a grounded numeric expression")

        numeric_conditions = tuple(lift_numeric(c.left)._compare(c.operator, lift_numeric(c.right)) for c in self.numeric_conditions)
        variables = list(variable_map.values())
        if add_inequalities and problem.domain.uses_equality:
            equals = problem.domain.predicate("=")
            variables = list(variable_map.values())
            for i in range(len(variables)):
                for j in range(i + 1, len(variables)):
                    atom = problem.lifted_atom(equals, variables[i], variables[j])
                    lifted_literals.append(problem.literal(atom, positive=False))

        return problem.condition(*lifted_literals, variables=variables, numeric_conditions=numeric_conditions)

    def holds(self, state: State) -> bool:
        if not isinstance(state, State):
            raise TypeError("state must be a State")
        if self._problem != state.problem:
            raise ValueError("condition and state belong to different problems")
        return bool(lib.mimir_goal_is_satisfied(self._handle, state._handle))

    def __iter__(self) -> Iterator[GroundLiteral]:
        return iter(self._literals)

    def __len__(self) -> int:
        return len(self._literals)

    def __str__(self) -> str:
        return "(and " + " ".join([*(str(l) for l in self._literals), *(str(c) for c in self._numeric_conditions)]) + ")"

    def __repr__(self) -> str:
        return self.__str__()

    def __eq__(self, other: object) -> bool:
        return (
            isinstance(other, GroundConjunctiveCondition)
            and self._problem == other._problem
            and self._literals == other._literals
            and self.numeric_conditions == other.numeric_conditions
        )

    def __hash__(self) -> int:
        return hash(("GroundConjunctiveCondition", self._problem, tuple(self._literals), self.numeric_conditions))


def _predicate_type(
    value: Predicate | Atom | GroundAtom | Literal | GroundLiteral,
) -> PredicateType:
    if isinstance(value, Predicate):
        code = lib.mimir_predicate_get_predicate_type(value._handle)
    elif isinstance(value, Atom):
        code = lib.mimir_atom_get_predicate_type(value._handle)
    elif isinstance(value, GroundAtom):
        code = lib.mimir_ground_atom_get_predicate_type(value._handle)
    elif isinstance(value, Literal):
        code = lib.mimir_literal_get_predicate_type(value._handle)
    elif isinstance(value, GroundLiteral):
        code = lib.mimir_ground_literal_get_predicate_type(value._handle)
    else:
        raise TypeError("value has no predicate type")
    if code == _PREDICATE_TYPE_STATIC:
        return PredicateType.STATIC
    if code == _PREDICATE_TYPE_FLUENT:
        return PredicateType.FLUENT
    if code == _PREDICATE_TYPE_DERIVED:
        return PredicateType.DERIVED
    raise RuntimeError("native value has an unknown predicate type")


__all__ = [
    "Variable", "Object", "Term",
    "Predicate",
    "Atom", "GroundAtom",
    "Literal", "GroundLiteral",
    "Effect", "ConditionalEffect", "ConditionalNumericEffect", "GroundEffect",
    "GroundConditionalEffect", "GroundConditionalNumericEffect",
    "Action", "GroundAction",
    "Domain", "Problem", "State",
    "ConjunctiveCondition", "GroundConjunctiveCondition", "PredicateType", "ActionGenerator",
]
