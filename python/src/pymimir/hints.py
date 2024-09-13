# type hints:
from typing import Union

# import the necessary classes:
from pymimir import (
    StaticAtom,
    FluentAtom,
    DerivedAtom,
    StaticGroundAtom,
    FluentGroundAtom,
    DerivedGroundAtom,
    StaticLiteral,
    FluentLiteral,
    DerivedLiteral,
    StaticGroundLiteral,
    FluentGroundLiteral,
    DerivedGroundLiteral,
    StaticPredicate,
    FluentPredicate,
    DerivedPredicate,
)

# Define the type hint for the Atom class:
Atom = Union[
    StaticAtom,
    FluentAtom,
    DerivedAtom,
]
GroundAtom = Union[
    StaticGroundAtom,
    FluentGroundAtom,
    DerivedGroundAtom,
]
Literal = Union[
    StaticLiteral,
    FluentLiteral,
    DerivedLiteral,
]
GroundLiteral = Union[
    StaticGroundLiteral,
    FluentGroundLiteral,
    DerivedGroundLiteral,
]
Predicate = Union[
    StaticPredicate,
    FluentPredicate,
    DerivedPredicate,
]

__all__ = [
    "Atom",
    "GroundAtom",
    "Literal",
    "GroundLiteral",
    "Predicate",
]
