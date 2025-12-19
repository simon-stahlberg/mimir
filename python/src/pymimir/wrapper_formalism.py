from pathlib import Path
from typing import overload, Union, Iterator, Literal as Lit

from pymimir.advanced.formalism import Action as AdvancedAction
from pymimir.advanced.formalism import AuxiliaryFunction as AdvancedAuxiliaryFunction
from pymimir.advanced.formalism import AuxiliaryFunctionExpressionFunction as AdvancedAuxiliaryFunctionExpressionFunction
from pymimir.advanced.formalism import AuxiliaryFunctionSkeleton as AdvancedAuxiliaryFunctionSkeleton
from pymimir.advanced.formalism import AuxiliaryFunctionSkeletonList as AdvancedAuxiliaryFunctionSkeletonList
from pymimir.advanced.formalism import AuxiliaryGroundFunction as AdvancedAuxiliaryGroundFunction
from pymimir.advanced.formalism import AuxiliaryGroundFunctionExpressionFunction as AdvancedAuxiliaryGroundFunctionExpressionFunction
from pymimir.advanced.formalism import AuxiliaryGroundFunctionValue as AdvancedAuxiliaryGroundFunctionValue
from pymimir.advanced.formalism import AuxiliaryGroundNumericEffect as AdvancedAuxiliaryGroundNumericEffect
from pymimir.advanced.formalism import AuxiliaryNumericEffect as AdvancedAuxiliaryNumericEffect
from pymimir.advanced.formalism import ConditionalEffect as AdvancedConditionalEffect
from pymimir.advanced.formalism import ConjunctiveCondition as AdvancedConjunctiveCondition
from pymimir.advanced.formalism import ConjunctiveEffect as AdvancedConjunctiveEffect
from pymimir.advanced.formalism import DerivedAtom as AdvancedDerivedAtom
from pymimir.advanced.formalism import DerivedGroundAtom as AdvancedDerivedGroundAtom
from pymimir.advanced.formalism import DerivedGroundAtomList as AdvancedDerivedGroundAtomList
from pymimir.advanced.formalism import DerivedGroundLiteral as AdvancedDerivedGroundLiteral
from pymimir.advanced.formalism import DerivedGroundLiteralList as AdvancedDerivedGroundLiteralList
from pymimir.advanced.formalism import DerivedLiteral as AdvancedDerivedLiteral
from pymimir.advanced.formalism import DerivedLiteralList as AdvancedDerivedLiteralList
from pymimir.advanced.formalism import DerivedPredicate as AdvancedDerivedPredicate
from pymimir.advanced.formalism import FluentAtom as AdvancedFluentAtom
from pymimir.advanced.formalism import FluentFunction as AdvancedFluentFunction
from pymimir.advanced.formalism import FluentFunctionExpressionFunction as AdvancedFluentFunctionExpressionFunction
from pymimir.advanced.formalism import FluentFunctionSkeleton as AdvancedFluentFunctionSkeleton
from pymimir.advanced.formalism import FluentFunctionSkeletonList as AdvancedFluentFunctionSkeletonList
from pymimir.advanced.formalism import FluentGroundAtom as AdvancedFluentGroundAtom
from pymimir.advanced.formalism import FluentGroundAtomList as AdvancedFluentGroundAtomList
from pymimir.advanced.formalism import FluentGroundFunction as AdvancedFluentGroundFunction
from pymimir.advanced.formalism import FluentGroundFunctionExpressionFunction as AdvancedFluentGroundFunctionExpressionFunction
from pymimir.advanced.formalism import FluentGroundFunctionValue as AdvancedFluentGroundFunctionValue
from pymimir.advanced.formalism import FluentGroundLiteral as AdvancedFluentGroundLiteral
from pymimir.advanced.formalism import FluentGroundLiteralList as AdvancedFluentGroundLiteralList
from pymimir.advanced.formalism import FluentGroundNumericEffect as AdvancedFluentGroundNumericEffect
from pymimir.advanced.formalism import FluentLiteral as AdvancedFluentLiteral
from pymimir.advanced.formalism import FluentLiteralList as AdvancedFluentLiteralList
from pymimir.advanced.formalism import FluentNumericEffect as AdvancedFluentNumericEffect
from pymimir.advanced.formalism import FluentPredicate as AdvancedFluentPredicate
from pymimir.advanced.formalism import FunctionExpression as AdvancedFunctionExpressionBase
from pymimir.advanced.formalism import FunctionExpressionBinaryOperator as AdvancedFunctionExpressionBinaryOperator
from pymimir.advanced.formalism import FunctionExpressionMinus as AdvancedFunctionExpressionMinus
from pymimir.advanced.formalism import FunctionExpressionMultiOperator as AdvancedFunctionExpressionMultiOperator
from pymimir.advanced.formalism import FunctionExpressionNumber as AdvancedFunctionExpressionNumber
from pymimir.advanced.formalism import GroundAction as AdvancedGroundAction
from pymimir.advanced.formalism import GroundConditionalEffect as AdvancedGroundConditionalEffect
from pymimir.advanced.formalism import GroundConjunctiveCondition as AdvancedGroundConjunctiveCondition
from pymimir.advanced.formalism import GroundConjunctiveEffect as AdvancedGroundConjunctiveEffect
from pymimir.advanced.formalism import GroundFunctionExpression as AdvancedGroundFunctionExpressionBase
from pymimir.advanced.formalism import GroundFunctionExpressionBinaryOperator as AdvancedGroundFunctionExpressionBinaryOperator
from pymimir.advanced.formalism import GroundFunctionExpressionMinus as AdvancedGroundFunctionExpressionMinus
from pymimir.advanced.formalism import GroundFunctionExpressionMultiOperator as AdvancedGroundFunctionExpressionMultiOperator
from pymimir.advanced.formalism import GroundFunctionExpressionNumber as AdvancedGroundFunctionExpressionNumber
from pymimir.advanced.formalism import GroundNumericConstraint as AdvancedGroundNumericConstraint
from pymimir.advanced.formalism import GroundNumericConstraintList as AdvancedGroundNumericConstraintList
from pymimir.advanced.formalism import NumericConstraint as AdvancedNumericConstraint
from pymimir.advanced.formalism import NumericConstraintList as AdvancedNumericConstraintList
from pymimir.advanced.formalism import Object as AdvancedObject
from pymimir.advanced.formalism import ObjectList as AdvancedObjectList
from pymimir.advanced.formalism import Parameter as AdvancedParameter
from pymimir.advanced.formalism import ParameterList as AdvancedParameterList
from pymimir.advanced.formalism import Parser as AdvancedParser
from pymimir.advanced.formalism import ParserOptions as AdvancedParserOptions
from pymimir.advanced.formalism import Problem as AdvancedProblem
from pymimir.advanced.formalism import StaticAtom as AdvancedStaticAtom
from pymimir.advanced.formalism import StaticFunction as AdvancedStaticFunction
from pymimir.advanced.formalism import StaticFunctionExpressionFunction as AdvancedStaticFunctionExpressionFunction
from pymimir.advanced.formalism import StaticFunctionSkeleton as AdvancedStaticFunctionSkeleton
from pymimir.advanced.formalism import StaticFunctionSkeletonList as AdvancedStaticFunctionSkeletonList
from pymimir.advanced.formalism import StaticGroundAtom as AdvancedStaticGroundAtom
from pymimir.advanced.formalism import StaticGroundAtomList as AdvancedStaticGroundAtomList
from pymimir.advanced.formalism import StaticGroundFunction as AdvancedStaticGroundFunction
from pymimir.advanced.formalism import StaticGroundFunctionExpressionFunction as AdvancedStaticGroundFunctionExpressionFunction
from pymimir.advanced.formalism import StaticGroundFunctionValue as AdvancedStaticGroundFunctionValue
from pymimir.advanced.formalism import StaticGroundLiteral as AdvancedStaticGroundLiteral
from pymimir.advanced.formalism import StaticGroundLiteralList as AdvancedStaticGroundLiteralList
from pymimir.advanced.formalism import StaticLiteral as AdvancedStaticLiteral
from pymimir.advanced.formalism import StaticLiteralList as AdvancedStaticLiteralList
from pymimir.advanced.formalism import StaticPredicate as AdvancedStaticPredicate
from pymimir.advanced.formalism import Term as AdvancedTerm
from pymimir.advanced.formalism import TermList as AdvancedTermList
from pymimir.advanced.formalism import Type as AdvancedType
from pymimir.advanced.formalism import TypeList as AdvancedTypeList
from pymimir.advanced.formalism import Variable as AdvancedVariable
from pymimir.advanced.formalism import VariableList as AdvancedVariableList
from pymimir.advanced.search import ConjunctiveConditionSatisficingBindingGenerator
from pymimir.advanced.search import GroundedOptions, LiftedOptions, LiftedKPKCOptions, LiftedExhaustiveOptions, SymmetryPruning, SearchContext, SearchContextOptions
from pymimir.advanced.search import State as AdvancedState


# -------------------------------
# Type aliases for advanced types
# -------------------------------

AdvancedAtom = Union[AdvancedStaticAtom, AdvancedFluentAtom, AdvancedDerivedAtom]
AdvancedFunction = Union[AdvancedStaticFunction, AdvancedFluentFunction, AdvancedAuxiliaryFunction]
AdvancedFunctionExpression = Union[AdvancedFunctionExpressionBinaryOperator, AdvancedFunctionExpressionMultiOperator, AdvancedFunctionExpressionMinus, AdvancedFunctionExpressionNumber, AdvancedStaticFunctionExpressionFunction, AdvancedFluentFunctionExpressionFunction, AdvancedAuxiliaryFunctionExpressionFunction]
AdvancedFunctionSkeleton = Union[AdvancedStaticFunctionSkeleton, AdvancedFluentFunctionSkeleton, AdvancedAuxiliaryFunctionSkeleton]
AdvancedFunctionSkeletonList = Union[AdvancedStaticFunctionSkeletonList, AdvancedFluentFunctionSkeletonList, AdvancedAuxiliaryFunctionSkeletonList]
AdvancedGroundAtom = Union[AdvancedStaticGroundAtom, AdvancedFluentGroundAtom, AdvancedDerivedGroundAtom]
AdvancedGroundAtomList = Union[AdvancedStaticGroundAtomList, AdvancedFluentGroundAtomList, AdvancedDerivedGroundAtomList]
AdvancedGroundFunction = Union[AdvancedStaticGroundFunction, AdvancedFluentGroundFunction, AdvancedAuxiliaryGroundFunction]
AdvancedGroundFunctionExpression = Union[AdvancedGroundFunctionExpressionBinaryOperator, AdvancedGroundFunctionExpressionMultiOperator, AdvancedGroundFunctionExpressionMinus, AdvancedGroundFunctionExpressionNumber, AdvancedStaticGroundFunctionExpressionFunction, AdvancedFluentGroundFunctionExpressionFunction, AdvancedAuxiliaryGroundFunctionExpressionFunction]
AdvancedGroundLiteral = Union[AdvancedStaticGroundLiteral, AdvancedFluentGroundLiteral, AdvancedDerivedGroundLiteral]
AdvancedGroundLiteralList = Union[AdvancedStaticGroundLiteralList, AdvancedFluentGroundLiteralList, AdvancedDerivedGroundLiteralList]
AdvancedLiteral = Union[AdvancedStaticLiteral, AdvancedFluentLiteral, AdvancedDerivedLiteral]
AdvancedLiteralList = Union[AdvancedStaticLiteralList, AdvancedFluentLiteralList, AdvancedDerivedLiteralList]
AdvancedPredicate = Union[AdvancedStaticPredicate, AdvancedFluentPredicate, AdvancedDerivedPredicate]


# ------------
# Type aliases
# ------------

Term = Union['Object', 'Variable']


# ----------------
# Helper functions
# ----------------

def _split_ground_literal_list(literals: 'list[GroundLiteral]') -> 'tuple[AdvancedStaticGroundLiteralList, AdvancedFluentGroundLiteralList, AdvancedDerivedGroundLiteralList]':
    """
    Split the ground literal list into static, fluent, and derived ground literals.

    :param literals: List of ground literals to split.
    :type literals: list[GroundLiteral]
    :return: Tuple containing lists of static, fluent, and derived ground literals.
    :rtype: tuple[AdvancedStaticGroundLiteralList, AdvancedFluentGroundLiteralList, AdvancedDerivedGroundLiteralList]
    """
    advanced_static_ground_literals = AdvancedStaticGroundLiteralList([x._advanced_ground_literal for x in literals if isinstance(x._advanced_ground_literal, AdvancedStaticGroundLiteral)])
    advanced_fluent_ground_literals = AdvancedFluentGroundLiteralList([x._advanced_ground_literal for x in literals if isinstance(x._advanced_ground_literal, AdvancedFluentGroundLiteral)])
    advanced_derived_ground_literals = AdvancedDerivedGroundLiteralList([x._advanced_ground_literal for x in literals if isinstance(x._advanced_ground_literal, AdvancedDerivedGroundLiteral)])
    assert len(advanced_static_ground_literals) + len(advanced_fluent_ground_literals) + len(advanced_derived_ground_literals) == len(literals), "Invalid ground literal list."
    return advanced_static_ground_literals, advanced_fluent_ground_literals, advanced_derived_ground_literals

def _split_literal_list(literals: 'list[Literal]') -> 'tuple[AdvancedStaticLiteralList, AdvancedFluentLiteralList, AdvancedDerivedLiteralList]':
    """
    Split the literal list into static, fluent, and derived literals.

    :param literals: List of literals to split.
    :type literals: list[Literal]
    :return: Tuple containing lists of static, fluent, and derived literals.
    :rtype: tuple[AdvancedStaticLiteralList, AdvancedFluentLiteralList, AdvancedDerivedLiteralList]
    """
    advanced_static_literals = AdvancedStaticLiteralList([x._advanced_literal for x in literals if isinstance(x._advanced_literal, AdvancedStaticLiteral)])
    advanced_fluent_literals = AdvancedFluentLiteralList([x._advanced_literal for x in literals if isinstance(x._advanced_literal, AdvancedFluentLiteral)])
    advanced_derived_literals = AdvancedDerivedLiteralList([x._advanced_literal for x in literals if isinstance(x._advanced_literal, AdvancedDerivedLiteral)])
    assert len(advanced_static_literals) + len(advanced_fluent_literals) + len(advanced_derived_literals) == len(literals), "Invalid literal list."
    return advanced_static_literals, advanced_fluent_literals, advanced_derived_literals


# -------
# Classes
# -------

class Variable:
    """Class representing variables."""

    def __init__(self, advanced_variable: 'Union[AdvancedVariable, AdvancedParameter]') -> None:
        """
        Internal constructor for the Variable class; to create a variable, use the new() method.

        :param advanced_variable: An instance of an advanced variable.
        :type advanced_variable: AdvancedVariable
        """
        assert isinstance(advanced_variable, AdvancedParameter) or isinstance(advanced_variable, AdvancedVariable), "Invalid variable type."
        if isinstance(advanced_variable, AdvancedParameter):
            self._advanced_variable = advanced_variable.get_variable()
        else:
            self._advanced_variable = advanced_variable

    @staticmethod
    def new(name: 'str', parameter_index: 'int', problem: 'Problem') -> 'Variable':
        """
        Create a new variable with the given name.

        :param name: The name of the variable.
        :type name: str
        :param parameter_index: The index of the variable in the parameter list.
        :type parameter_index: int
        :param problem: The problem instance to which the variable belongs.
        :type problem: Problem
        :return: A new Variable instance.
        :rtype: Variable
        """
        return problem.new_variable(name, parameter_index)

    def get_index(self) -> 'int':
        """
        Get the index of the variable.

        :return: The index of the variable.
        :rtype: int
        """
        return self._advanced_variable.get_index()

    def get_name(self) -> 'str':
        """
        Get the name of the variable.

        :return: The name of the variable.
        :rtype: str
        """
        return self._advanced_variable.get_name()

    def __str__(self) -> 'str':
        """
        Get the string representation of the variable.

        :return: The string representation of the variable.
        :rtype: str
        """
        return str(self._advanced_variable)

    def __repr__(self) -> 'str':
        """
        Get the string representation of the variable.

        :return: The string representation of the variable.
        :rtype: str
        """
        return str(self._advanced_variable)

    def __hash__(self) -> 'int':
        """
        Get the hash of the variable.

        :return: The hash of the variable.
        :rtype: int
        """
        return hash(self._advanced_variable)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two variables are equal.

        :param other: The other variable to compare with.
        :type other: object
        :return: True if the variables are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, Variable):
            return False
        return self._advanced_variable == other._advanced_variable


class Object:
    """Class representing objects in the PDDL domain."""

    def __init__(self, advanced_object: 'AdvancedObject') -> None:
        """
        Internal constructor for the Object class; to create an object, use the new() method.

        :param advanced_object: An instance of an advanced object.
        :type advanced_object: AdvancedObject
        """
        assert isinstance(advanced_object, AdvancedObject), "Invalid object type."
        self._advanced_object = advanced_object

    def get_index(self) -> 'int':
        """
        Get the index of the object.

        :return: The index of the object.
        :rtype: int
        """
        return self._advanced_object.get_index()

    def get_name(self) -> 'str':
        """
        Get the name of the object.

        :return: The name of the object.
        :rtype: str
        """
        return self._advanced_object.get_name()

    def __str__(self) -> 'str':
        """
        Get the string representation of the object.

        :return: The string representation of the object.
        :rtype: str
        """
        return str(self._advanced_object)

    def __repr__(self) -> 'str':
        """
        Get the string representation of the object.

        :return: The string representation of the object.
        :rtype: str
        """
        return str(self._advanced_object)

    def __hash__(self) -> 'int':
        """
        Get the hash of the object.

        :return: The hash of the object.
        :rtype: int
        """
        return hash(self._advanced_object)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two objects are equal.

        :param other: The other object to compare with.
        :type other: object
        :return: True if the objects are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, Object):
            return False
        return self._advanced_object == other._advanced_object


class Predicate:
    """Class representing predicates in the PDDL domain."""

    def __init__(self, advanced_predicate: 'AdvancedPredicate') -> None:
        """
        Internal constructor for the Predicate class; to create a predicate, use the new() method.

        :param advanced_predicate: An instance of an advanced predicate.
        :type advanced_predicate: AdvancedPredicate
        """
        assert isinstance(advanced_predicate, AdvancedStaticPredicate) \
            or isinstance(advanced_predicate, AdvancedFluentPredicate) \
            or isinstance(advanced_predicate, AdvancedDerivedPredicate), "Invalid predicate type."
        self._advanced_predicate = advanced_predicate

    def get_index(self) -> 'int':
        """
        Get the index of the predicate.

        :return: The index of the predicate.
        :rtype: int
        """
        return self._advanced_predicate.get_index()

    def get_name(self) -> 'str':
        """
        Get the name of the predicate.

        :return: The name of the predicate.
        :rtype: str
        """
        return self._advanced_predicate.get_name()

    def get_arity(self) -> 'int':
        """
        Get the arity of the predicate.

        :return: The arity of the predicate.
        :rtype: int
        """
        return self._advanced_predicate.get_arity()

    def get_parameters(self) -> 'list[Variable]':
        """
        Get the parameters of the predicate.

        :return: A list of variables representing the parameters of the predicate.
        :rtype: list[Variable]
        """
        return [Variable(x) for x in self._advanced_predicate.get_parameters()]

    def is_static(self) -> 'bool':
        """
        Get whether the predicate is static.

        :return: True if the predicate is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_predicate, AdvancedStaticPredicate)

    def is_fluent(self) -> 'bool':
        """
        Get whether the predicate is fluent.

        :return: True if the predicate is fluent, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_predicate, AdvancedFluentPredicate)

    def is_derived(self) -> 'bool':
        """
        Get whether the predicate is derived.

        :return: True if the predicate is derived, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_predicate, AdvancedDerivedPredicate)

    def __str__(self):
        """
        Get the string representation of the predicate.

        :return: The string representation of the predicate.
        :rtype: str
        """
        return str(self._advanced_predicate)

    def __repr__(self):
        """
        Get the string representation of the predicate.

        :return: The string representation of the predicate.
        :rtype: str
        """
        return str(self._advanced_predicate)

    def __hash__(self) -> 'int':
        """
        Get the hash of the predicate.

        :return: The hash of the predicate.
        :rtype: int
        """
        return hash(self._advanced_predicate)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two predicates are equal.

        :param other: The other predicate to compare with.
        :type other: object
        :return: True if the predicates are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, Predicate):
            return False
        if type(self._advanced_predicate) != type(other._advanced_predicate):
            return False
        return self._advanced_predicate == other._advanced_predicate  # type: ignore


class GroundAtom:
    """Class representing ground atoms in the PDDL domain."""

    def __init__(self, advanced_ground_atom: 'AdvancedGroundAtom') -> None:
        """
        Internal constructor for the GroundAtom class; to create a ground atom, use the new() method.

        :param advanced_ground_atom: An instance of an advanced ground atom.
        :type advanced_ground_atom: AdvancedGroundAtom
        """
        assert isinstance(advanced_ground_atom, AdvancedStaticGroundAtom) \
            or isinstance(advanced_ground_atom, AdvancedFluentGroundAtom) \
            or isinstance(advanced_ground_atom, AdvancedDerivedGroundAtom), "Invalid ground atom type."
        self._advanced_ground_atom = advanced_ground_atom

    @staticmethod
    def new(predicate: 'Predicate', objects: 'list[Object]', problem: 'Problem') -> 'GroundAtom':
        """
        Create a new ground atom with the given predicate and objects.

        :param predicate: The predicate of the ground atom.
        :type predicate: Predicate
        :param objects: The objects of the ground atom.
        :type objects: list[Object]
        :param problem: The problem instance to which the ground atom belongs.
        :type problem: Problem
        :return: A new GroundAtom instance.
        :rtype: GroundAtom
        """
        return problem.new_ground_atom(predicate, objects)

    def get_index(self) -> 'int':
        """
        Get the index of the ground atom.

        :return: The index of the ground atom.
        :rtype: int
        """
        return self._advanced_ground_atom.get_index()

    def get_predicate(self) -> 'Predicate':
        """
        Get the predicate of the ground atom.

        :return: The predicate of the ground atom.
        :rtype: Predicate
        """
        return Predicate(self._advanced_ground_atom.get_predicate())

    def get_terms(self) -> 'list[Object]':
        """
        Get the objects of the ground atom.

        :return: A list of objects representing the terms of the ground atom.
        :rtype: list[Object]
        """
        return [Object(x) for x in self._advanced_ground_atom.get_objects()]

    def get_arity(self) -> 'int':
        """
        Get the arity of the ground atom.

        :return: The arity of the ground atom.
        :rtype: int
        """
        return self._advanced_ground_atom.get_arity()

    def is_static(self) -> 'bool':
        """
        Get whether the ground atom is static.

        :return: True if the ground atom is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_ground_atom, AdvancedStaticGroundAtom)

    def is_fluent(self) -> 'bool':
        """
        Get whether the ground atom is fluent.

        :return: True if the ground atom is fluent, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_ground_atom, AdvancedFluentGroundAtom)

    def is_derived(self) -> 'bool':
        """
        Get whether the ground atom is derived.

        :return: True if the ground atom is derived, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_ground_atom, AdvancedDerivedGroundAtom)

    def __str__(self):
        """
        Get the string representation of the ground atom.

        :return: The string representation of the ground atom.
        :rtype: str
        """
        return str(self._advanced_ground_atom)

    def __repr__(self):
        """
        Get the string representation of the ground atom.

        :return: The string representation of the ground atom.
        :rtype: str
        """
        return str(self._advanced_ground_atom)

    def __hash__(self) -> 'int':
        """
        Get the hash of the ground atom.

        :return: The hash of the ground atom.
        :rtype: int
        """
        return hash(self._advanced_ground_atom)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two ground atoms are equal.

        :param other: The other ground atom to compare with.
        :type other: object
        :return: True if the ground atoms are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, GroundAtom):
            return False
        if type(self._advanced_ground_atom) != type(other._advanced_ground_atom):
            return False
        return self._advanced_ground_atom == other._advanced_ground_atom  # type: ignore


class Atom:
    """Class representing atoms in the PDDL domain."""

    def __init__(self, advanced_atom: 'AdvancedAtom') -> None:
        """
        Internal constructor for the Atom class; to create an atom, use the new() method.

        :param advanced_atom: An instance of an advanced atom.
        :type advanced_atom: AdvancedAtom
        """
        assert isinstance(advanced_atom, AdvancedStaticAtom) \
            or isinstance(advanced_atom, AdvancedFluentAtom) \
            or isinstance(advanced_atom, AdvancedDerivedAtom), "Invalid atom type."
        self._advanced_atom = advanced_atom

    @staticmethod
    def new(predicate: 'Predicate', terms: 'list[Term]', problem: 'Problem') -> 'Atom':
        """
        Create a new atom with the given predicate and terms.

        :param predicate: The predicate of the atom.
        :type predicate: Predicate
        :param terms: The terms of the atom, which can be a mix of Objects and Variables.
        :type terms: list[Term]
        :param problem: The problem instance to which the atom belongs.
        :type problem: Problem
        :return: A new Atom instance.
        :rtype: Atom
        """
        return problem.new_atom(predicate, terms)

    def is_static(self) -> 'bool':
        """
        Get whether the atom is static.

        :return: True if the atom is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_atom, AdvancedStaticAtom)

    def is_fluent(self) -> 'bool':
        """
        Get whether the atom is static.

        :return: True if the atom is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_atom, AdvancedFluentAtom)

    def is_derived(self) -> 'bool':
        """
        Get whether the atom is static.

        :return: True if the atom is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_atom, AdvancedDerivedAtom)

    def get_index(self) -> 'int':
        """
        Get the index of the atom.

        :return: The index of the atom.
        :rtype: int
        """
        return self._advanced_atom.get_index()

    def get_predicate(self) -> 'Predicate':
        """
        Get the predicate of the atom.

        :return: The predicate of the atom.
        :rtype: Predicate
        """
        return Predicate(self._advanced_atom.get_predicate())

    def get_terms(self) -> 'list[Union[Object, Variable]]':
        """
        Get the terms of the atom.

        :return: A list of terms representing the terms of the atom, which can be a mix of Objects and Variables.
        :rtype: list[Union[Object, Variable]]
        """
        return [Object(x.get()) if isinstance(x.get(), AdvancedObject) else Variable(x.get()) for x in self._advanced_atom.get_terms()]  # type: ignore

    def get_variables(self) -> 'list[Variable]':
        """
        Get the variables of the atom.

        :return: A list of variables representing the variables of the atom.
        :rtype: list[Variable]
        """
        return [Variable(x) for x in self._advanced_atom.get_variables()]

    def get_arity(self) -> 'int':
        """
        Get the arity of the atom.

        :return: The arity of the atom.
        :rtype: int
        """
        return len(self.get_terms())

    def __str__(self):
        """
        Get the string representation of the atom.

        :return: The string representation of the atom.
        :rtype: str
        """
        return str(self._advanced_atom)

    def __repr__(self):
        """
        Get the string representation of the atom.

        :return: The string representation of the atom.
        :rtype: str
        """
        return str(self._advanced_atom)

    def __hash__(self) -> 'int':
        """
        Get the hash of the atom.

        :return: The hash of the atom.
        :rtype: int
        """
        return hash(self._advanced_atom)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two atoms are equal.

        :param other: The other atom to compare with.
        :type other: object
        :return: True if the atoms are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, Atom):
            return False
        if type(self._advanced_atom) != type(other._advanced_atom):
            return False
        return self._advanced_atom == other._advanced_atom  # type: ignore


class GroundLiteral:
    """GroundLiteral class for the PDDL domain."""

    def __init__(self, advanced_ground_literal: 'AdvancedGroundLiteral') -> None:
        """
        Internal constructor for the GroundLiteral class; to create a ground literal, use the new() method.

        :param advanced_ground_literal: An instance of an advanced ground literal.
        :type advanced_ground_literal: AdvancedGroundLiteral
        """
        assert isinstance(advanced_ground_literal, AdvancedStaticGroundLiteral) \
            or isinstance(advanced_ground_literal, AdvancedFluentGroundLiteral) \
            or isinstance(advanced_ground_literal, AdvancedDerivedGroundLiteral), "Invalid ground literal type."
        self._advanced_ground_literal = advanced_ground_literal

    @staticmethod
    def new(ground_atom: 'GroundAtom', polarity: 'bool', problem: 'Problem') -> 'GroundLiteral':
        """
        Create a new ground literal with the given ground atom and polarity.

        :param ground_atom: The ground atom of the ground literal.
        :type ground_atom: GroundAtom
        :param polarity: The polarity of the ground literal (True for positive, False for negative).
        :type polarity: bool
        :param problem: The problem instance to which the ground literal belongs.
        :type problem: Problem
        :return: A new GroundLiteral instance.
        :rtype: GroundLiteral
        """
        return problem.new_ground_literal(ground_atom, polarity)

    def is_static(self) -> 'bool':
        """
        Get whether the ground literal is static.

        :return: True if the ground literal is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_ground_literal, AdvancedStaticGroundLiteral)

    def is_fluent(self) -> 'bool':
        """
        Get whether the ground literal is static.

        :return: True if the ground literal is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_ground_literal, AdvancedFluentGroundLiteral)

    def is_derived(self) -> 'bool':
        """
        Get whether the ground literal is static.

        :return: True if the ground literal is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_ground_literal, AdvancedDerivedGroundLiteral)

    def get_index(self) -> 'int':
        """
        Get the index of the ground literal.

        :return: The index of the ground literal.
        :rtype: int
        """
        return self._advanced_ground_literal.get_index()

    def get_atom(self) -> 'GroundAtom':
        """
        Get the atom of the ground literal.

        :return: The atom of the ground literal.
        :rtype: GroundAtom
        """
        return GroundAtom(self._advanced_ground_literal.get_atom())

    def get_polarity(self) -> bool:
        """
        Get the polarity of the ground literal.

        :return: True if the ground literal is positive, False if it is negative.
        :rtype: bool
        """
        return self._advanced_ground_literal.get_polarity()

    def __str__(self):
        """
        Get the string representation of the ground literal.

        :return: The string representation of the ground literal.
        :rtype: str
        """
        return str(self._advanced_ground_literal)

    def __repr__(self):
        """
        Get the string representation of the ground literal.

        :return: The string representation of the ground literal.
        :rtype: str
        """
        return str(self._advanced_ground_literal)

    def __hash__(self) -> 'int':
        """
        Get the hash of the ground literal.

        :return: The hash of the ground literal.
        :rtype: int
        """
        return hash(self._advanced_ground_literal)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two ground literals are equal.

        :param other: The other ground literal to compare with.
        :type other: object
        :return: True if the ground literals are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, GroundLiteral):
            return False
        if type(self._advanced_ground_literal) != type(other._advanced_ground_literal):
            return False
        return self._advanced_ground_literal == other._advanced_ground_literal  # type: ignore


class Literal:
    """Literal class for the PDDL domain."""

    def __init__(self, advanced_literal: 'AdvancedLiteral') -> None:
        """
        Internal constructor for the Literal class; to create a literal, use the new() method.

        :param advanced_literal: An instance of an advanced literal.
        :type advanced_literal: AdvancedLiteral
        """
        assert isinstance(advanced_literal, AdvancedStaticLiteral) \
            or isinstance(advanced_literal, AdvancedFluentLiteral) \
            or isinstance(advanced_literal, AdvancedDerivedLiteral), "Invalid literal type."
        self._advanced_literal = advanced_literal

    @staticmethod
    def new(atom: 'Atom', polarity: 'bool', problem: 'Problem') -> 'Literal':
        """
        Create a new literal with the given atom and polarity.

        :param atom: The atom of the literal.
        :type atom: Atom
        :param polarity: The polarity of the literal (True for positive, False for negative).
        :type polarity: bool
        :param problem: The problem instance to which the literal belongs.
        :type problem: Problem
        :return: A new Literal instance.
        :rtype: Literal
        """
        return problem.new_literal(atom, polarity)

    def is_static(self) -> 'bool':
        """
        Get whether the literal is static.

        :return: True if the literal is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_literal, AdvancedStaticLiteral)

    def is_fluent(self) -> 'bool':
        """
        Get whether the literal is static.

        :return: True if the literal is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_literal, AdvancedFluentLiteral)

    def is_derived(self) -> 'bool':
        """
        Get whether the literal is static.

        :return: True if the literal is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_literal, AdvancedDerivedLiteral)

    def get_index(self) -> 'int':
        """
        Get the index of the literal.

        :return: The index of the literal.
        :rtype: int
        """
        return self._advanced_literal.get_index()

    def get_atom(self) -> 'Atom':
        """
        Get the atom of the literal.

        :return: The atom of the literal.
        :rtype: Atom
        """
        return Atom(self._advanced_literal.get_atom())

    def get_polarity(self) -> bool:
        """
        Get the polarity of the literal.

        :return: True if the literal is positive, False if it is negative.
        :rtype: bool
        """
        return self._advanced_literal.get_polarity()

    def __str__(self) -> 'str':
        """
        Get the string representation of the literal.

        :return: The string representation of the literal.
        :rtype: str
        """
        return str(self._advanced_literal)

    def __repr__(self) -> 'str':
        """
        Get the string representation of the literal.

        :return: The string representation of the literal.
        :rtype: str
        """
        return str(self._advanced_literal)

    def __hash__(self) -> 'int':
        """
        Get the hash of the literal.

        :return: The hash of the literal.
        :rtype: int
        """
        return hash(self._advanced_literal)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two literals are equal.

        :param other: The other literal to compare with.
        :type other: object
        :return: True if the literals are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, Literal):
            return False
        if type(self._advanced_literal) != type(other._advanced_literal):
            return False
        return self._advanced_literal == other._advanced_literal  # type: ignore


class GroundEffect:
    """Class representing ground effects in the PDDL domain."""

    def __init__(self, advanced_effect: 'AdvancedGroundConjunctiveEffect', problem: 'Problem') -> None:
        """
        Internal constructor for the GroundEffect class; to create a ground effect, use the new() method.

        :param advanced_effect: An instance of an advanced ground conjunctive effect.
        :type advanced_effect: AdvancedGroundConjunctiveEffect
        :param problem: The problem instance to which the ground effect belongs.
        :type problem: Problem
        """
        assert isinstance(advanced_effect, AdvancedGroundConjunctiveEffect), "Invalid conjunctive effect type."
        assert isinstance(problem, Problem), "Invalid problem type."
        self._advanced_ground_effect = advanced_effect
        self._problem = problem
        self._operation_names = {
            'ASSIGN': 'assign',
            'INCREASE': 'increase',
            'DECREASE': 'decrease',
            'SCALE_UP': 'scale_up',
            'SCALE_DOWN': 'scale_down'
        }

    def get_index(self) -> 'int':
        """
        Get the index of the ground effect list.

        :return: The index of the ground effect list.
        :rtype: int
        """
        return self._advanced_ground_effect.get_index()

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance to which the ground effect belongs.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def get_add_list(self) -> 'list[GroundAtom]':
        """
        Get the add list of the ground effect.

        :return: A list of ground atoms that are added by the ground effect.
        :rtype: list[GroundAtom]
        """
        repositories = self._problem._advanced_problem.get_repositories()
        advanced_ground_atoms = repositories.get_fluent_ground_atoms_from_indices(list(self._advanced_ground_effect.get_positive_effects()))
        return [GroundAtom(x) for x in advanced_ground_atoms]

    def get_delete_list(self) -> 'list[GroundAtom]':
        """
        Get the delete list of the ground effect.

        :return: A list of ground atoms that are deleted by the ground effect.
        :rtype: list[GroundAtom]
        """
        repositories = self._problem._advanced_problem.get_repositories()
        advanced_ground_atoms = repositories.get_fluent_ground_atoms_from_indices(list(self._advanced_ground_effect.get_negative_effects()))
        return [GroundAtom(x) for x in advanced_ground_atoms]

    def get_functions(self) -> 'list[tuple[str, GroundFunctionTerm, GroundFunctionExpression]]':
        """
        Get the operation performed on the numeric functions by the conditional effect.

        :return: A list of tuples containing operation name, ground function term, and ground function expression.
        :rtype: list[tuple[str, GroundFunctionTerm, GroundFunctionExpression]]
        """
        def to_tuple(effect: 'Union[AdvancedFluentGroundNumericEffect, AdvancedAuxiliaryGroundNumericEffect]') -> 'tuple[str, GroundFunctionTerm, GroundFunctionExpression]':
            return (self._operation_names[effect.get_assign_operator().name], GroundFunctionTerm(effect.get_function()), GroundFunctionExpression(effect.get_function_expression()))
        fluent_effects = self._advanced_ground_effect.get_fluent_numeric_effects()
        auxiliary_effect = self._advanced_ground_effect.get_auxiliary_numeric_effect()
        function_effects = [to_tuple(effect) for effect in fluent_effects]
        if auxiliary_effect is not None:
            function_effects.append(to_tuple(auxiliary_effect))
        return function_effects

    def __str__(self):
        """
        Get the string representation of the ground effect list.

        :return: The string representation of the ground effect list.
        :rtype: str
        """
        return self._advanced_ground_effect.to_string(self._problem._advanced_problem)

    def __repr__(self):
        """
        Get the string representation of the ground effect list.

        :return: The string representation of the ground effect list.
        :rtype: str
        """
        return self._advanced_ground_effect.to_string(self._problem._advanced_problem)

    def __hash__(self) -> 'int':
        """
        Get the hash of the ground effect list.

        :return: The hash of the ground effect list.
        :rtype: int
        """
        return hash(self._advanced_ground_effect)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two ground effect lists are equal.

        :param other: The other ground effect list to compare with.
        :type other: object
        :return: True if the ground effect lists are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, GroundEffect):
            return False
        return self._advanced_ground_effect == other._advanced_ground_effect


class GroundConditionalEffect:
    """GroundConditionalEffect class for the PDDL domain."""

    def __init__(self, advanced_conditional_effect: 'AdvancedGroundConditionalEffect', problem: 'Problem') -> None:
        """
        Internal constructor for the GroundConditionalEffect class; to create a ground conditional effect, use the new() method.

        :param advanced_conditional_effect: An instance of an advanced ground conditional effect.
        :type advanced_conditional_effect: AdvancedGroundConditionalEffect
        :param problem: The problem instance to which the ground conditional effect belongs.
        :type problem: Problem
        """
        assert isinstance(advanced_conditional_effect, AdvancedGroundConditionalEffect), "Invalid conditional effect type."
        assert isinstance(problem, Problem), "Invalid problem type."
        self._advanced_conditional_effect = advanced_conditional_effect
        self._problem = problem

    def get_index(self) -> 'int':
        """
        Get the index of the ground conditional effect.

        :return: The index of the ground conditional effect.
        :rtype: int
        """
        return self._advanced_conditional_effect.get_index()

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance to which the ground conditional effect belongs.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def get_condition(self) -> 'GroundConjunctiveCondition':
        """
        Get the condition of the ground conditional effect.

        :return: The condition of the ground conditional effect.
        :rtype: GroundConjunctiveCondition
        """
        return GroundConjunctiveCondition(self._advanced_conditional_effect.get_conjunctive_condition(), self._problem)

    def get_effect(self) -> 'GroundEffect':
        """
        Get the effect list of the ground conditional effect.

        :return: The effect list of the ground conditional effect.
        :rtype: GroundEffect
        """
        return GroundEffect(self._advanced_conditional_effect.get_conjunctive_effect(), self._problem)

    def __str__(self):
        """
        Get the string representation of the ground conditional effect.

        :return: The string representation of the ground conditional effect.
        :rtype: str
        """
        return self._advanced_conditional_effect.to_string(self._problem._advanced_problem)

    def __repr__(self):
        """
        Get the string representation of the ground conditional effect.

        :return: The string representation of the ground conditional effect.
        :rtype: str
        """
        return self._advanced_conditional_effect.to_string(self._problem._advanced_problem)

    def __hash__(self) -> 'int':
        """
        Get the hash of the ground conditional effect.

        :return: The hash of the ground conditional effect.
        :rtype: int
        """
        return hash(self._advanced_conditional_effect)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two ground conditional effects are equal.

        :param other: The other ground conditional effect to compare with.
        :type other: object
        :return: True if the ground conditional effects are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, GroundConditionalEffect):
            return False
        return self._advanced_conditional_effect == other._advanced_conditional_effect


class Effect:
    """Class representing effects in the PDDL domain."""

    def __init__(self, advanced_effect: 'AdvancedConjunctiveEffect') -> None:
        """
        Internal constructor for the Effect class; to create an effect, use the new() method.

        :param advanced_effect: An instance of an advanced conjunctive effect.
        :type advanced_effect: AdvancedConjunctiveEffect
        """
        assert isinstance(advanced_effect, AdvancedConjunctiveEffect), "Invalid conjunctive effect type."
        self._advanced_conjunctive_effect = advanced_effect
        self._operation_names = {
            'ASSIGN': 'assign',
            'INCREASE': 'increase',
            'DECREASE': 'decrease',
            'SCALE_UP': 'scale_up',
            'SCALE_DOWN': 'scale_down'
        }

    def get_index(self) -> 'int':
        """
        Get the index of the effect list.

        :return: The index of the effect list.
        :rtype: int
        """
        return self._advanced_conjunctive_effect.get_index()

    def get_parameters(self) -> 'list[Variable]':
        """
        Get the parameters of the effect list.

        :return: A list of variables representing the parameters of the effect list.
        :rtype: list[Variable]
        """
        return [Variable(x) for x in self._advanced_conjunctive_effect.get_parameters()]

    def get_literals(self) -> 'list[Literal]':
        """
        Get the literals of the effect list.

        :return: A list of literals representing the effect list.
        :rtype: list[Literal]
        """
        return [Literal(x) for x in self._advanced_conjunctive_effect.get_literals()]

    def get_functions(self) -> 'list[tuple[str, FunctionTerm, FunctionExpression]]':
        """
        Get the operation performed on the numeric functions by the conditional effect.

        :return: A list of tuples containing operation name, function term, and function expression.
        :rtype: list[tuple[str, FunctionTerm, FunctionExpression]]
        """
        def to_tuple(effect: 'Union[AdvancedFluentNumericEffect, AdvancedAuxiliaryNumericEffect]') -> 'tuple[str, FunctionTerm, FunctionExpression]':
            return (self._operation_names[effect.get_assign_operator().name], FunctionTerm(effect.get_function()), FunctionExpression(effect.get_function_expression()))
        fluent_effects = self._advanced_conjunctive_effect.get_fluent_numeric_effects()
        auxiliary_effect = self._advanced_conjunctive_effect.get_auxiliary_numeric_effect()
        function_effects = [to_tuple(effect) for effect in fluent_effects]
        if auxiliary_effect is not None:
            function_effects.append(to_tuple(auxiliary_effect))
        return function_effects

    def __iter__(self) -> 'Iterator[Literal]':
        """
        Get an iterator over the literals of the effect list.

        :return: An iterator over the literals of the effect list.
        :rtype: Iterator[Literal]
        """
        return iter(self.get_literals())

    def __str__(self):
        """
        Get the string representation of the effect list.

        :return: The string representation of the effect list.
        :rtype: str
        """
        return str(self._advanced_conjunctive_effect)

    def __repr__(self):
        """
        Get the string representation of the effect list.

        :return: The string representation of the effect list.
        :rtype: str
        """
        return str(self._advanced_conjunctive_effect)

    def __hash__(self) -> 'int':
        """
        Get the hash of the effect list.

        :return: The hash of the effect list.
        :rtype: int
        """
        return hash(self._advanced_conjunctive_effect)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two effect lists are equal.

        :param other: The other effect list to compare with.
        :type other: object
        :return: True if the effect lists are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, Effect):
            return False
        return self._advanced_conjunctive_effect == other._advanced_conjunctive_effect


class ConditionalEffect:
    """Class representing conditional effects in the PDDL domain."""

    def __init__(self, advanced_conditional_effect: 'AdvancedConditionalEffect') -> None:
        """
        Internal constructor for the ConditionalEffect class; to create a conditional effect, use the new() method.

        :param advanced_conditional_effect: An instance of an advanced conditional effect.
        :type advanced_conditional_effect: AdvancedConditionalEffect
        """
        assert isinstance(advanced_conditional_effect, AdvancedConditionalEffect), "Invalid conditional effect type."
        self._advanced_conditional_effect = advanced_conditional_effect

    def get_index(self) -> 'int':
        """
        Get the index of the conditional effect.

        :return: The index of the conditional effect.
        :rtype: int
        """
        return self._advanced_conditional_effect.get_index()

    def get_condition(self) -> 'ConjunctiveCondition':
        """
        Get the static conditions of the conditional effect.

        :return: The conjunctive condition of the conditional effect.
        :rtype: ConjunctiveCondition
        """
        return ConjunctiveCondition(self._advanced_conditional_effect.get_conjunctive_condition())

    def get_effect(self) -> 'Effect':
        """
        Get the effect list of the conditional effect.

        :return: The effect list of the conditional effect.
        :rtype: Effect
        """
        return Effect(self._advanced_conditional_effect.get_conjunctive_effect())

    def __str__(self):
        """
        Get the string representation of the conditional effect.

        :return: The string representation of the conditional effect.
        :rtype: str
        """
        return str(self._advanced_conditional_effect)

    def __repr__(self):
        """
        Get the string representation of the conditional effect.

        :return: The string representation of the conditional effect.
        :rtype: str
        """
        return str(self._advanced_conditional_effect)

    def __hash__(self) -> 'int':
        """
        Get the hash of the conditional effect.

        :return: The hash of the conditional effect.
        :rtype: int
        """
        return hash(self._advanced_conditional_effect)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two conditional effects are equal.

        :param other: The other conditional effect to compare with.
        :type other: object
        :return: True if the conditional effects are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, ConditionalEffect):
            return False
        return self._advanced_conditional_effect == other._advanced_conditional_effect


class Action:
    """Class representing action schemas in the domain."""

    def __init__(self, advanced_action: 'AdvancedAction') -> None:
        """
        Internal constructor for the Action class; to create an action, use the new() method.

        :param advanced_action: An instance of an advanced action.
        :type advanced_action: AdvancedAction
        """
        assert isinstance(advanced_action, AdvancedAction), "Invalid action type."
        self._advanced_action = advanced_action

    def get_index(self) -> 'int':
        """
        Get the index of the action.

        :return: The index of the action.
        :rtype: int
        """
        return self._advanced_action.get_index()

    def get_arity(self) -> 'int':
        """
        Get the arity of the action.

        :return: The arity of the action.
        :rtype: int
        """
        return self._advanced_action.get_arity()

    def get_name(self) -> 'str':
        """
        Get the name of the action.

        :return: The name of the action.
        :rtype: str
        """
        return self._advanced_action.get_name()

    def get_parameters(self) -> 'list[Variable]':
        """
        Get the parameters of the action.

        :return: A list of variables representing the parameters of the action.
        :rtype: list[Variable]
        """
        return [Variable(x) for x in self._advanced_action.get_parameters()]

    def get_precondition(self) -> 'ConjunctiveCondition':
        """
        Get the conjunctive condition of the action.

        :return: The conjunctive condition of the action.
        :rtype: ConjunctiveCondition
        """
        return ConjunctiveCondition(self._advanced_action.get_conjunctive_condition())

    def get_conditional_effect(self) -> 'list[ConditionalEffect]':
        """
        Get the conditional effect of the action.

        :return: A list of conditional effects of the action.
        :rtype: list[ConditionalEffect]
        """
        return [ConditionalEffect(x) for x in self._advanced_action.get_conditional_effects()]

    def __str__(self):
        """
        Get the string representation of the action.

        :return: The string representation of the action.
        :rtype: str
        """
        return str(self._advanced_action)

    def __repr__(self):
        """
        Get the string representation of the action.

        :return: The string representation of the action.
        :rtype: str
        """
        return str(self._advanced_action)

    def __hash__(self) -> 'int':
        """
        Get the hash of the action.

        :return: The hash of the action.
        :rtype: int
        """
        return hash(self._advanced_action)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two actions are equal.

        :param other: The other action to compare with.
        :type other: object
        :return: True if the actions are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, Action):
            return False
        return self._advanced_action == other._advanced_action


class GroundAction:
    """Class representing ground actions in the problem."""

    def __init__(self, advanced_ground_action: 'AdvancedGroundAction', problem: 'Problem') -> None:
        """
        Internal constructor for the GroundAction class; to create a ground action, use the new() method.

        :param advanced_ground_action: An instance of an advanced ground action.
        :type advanced_ground_action: AdvancedGroundAction
        :param problem: The problem instance to which the ground action belongs.
        :type problem: Problem
        """
        assert isinstance(advanced_ground_action, AdvancedGroundAction), "Invalid ground action type."
        assert isinstance(problem, Problem), "Invalid problem type."
        self._advanced_ground_action = advanced_ground_action
        self._problem = problem

    @staticmethod
    def new(action: 'Action', objects: 'list[Object]', problem: 'Problem') -> 'GroundAction':
        """
        Create a new ground action with the given action schema and objects.

        :param action: The action schema of the ground action.
        :type action: Action
        :param objects: The objects of the ground action.
        :type objects: list[Object]
        :param problem: The problem instance to which the ground action belongs.
        :type problem: Problem
        :return: A new GroundAction instance.
        :rtype: GroundAction
        """
        return problem.new_ground_action(action, objects)

    def get_index(self) -> 'int':
        """
        Get the index of the ground action.

        :return: The index of the ground action.
        :rtype: int
        """
        return self._advanced_ground_action.get_index()

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance to which the ground action belongs.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def get_action(self) -> 'Action':
        """
        Get the name of the ground action.

        :return: The action schema of the ground action.
        :rtype: Action
        """
        return Action(self._advanced_ground_action.get_action())

    def get_objects(self) -> 'list[Object]':
        """
        Get the objects of the ground action.

        :return: A list of objects representing the objects of the ground action.
        :rtype: list[Object]
        """
        return [Object(x) for x in self._advanced_ground_action.get_objects()]

    def get_precondition(self) -> 'GroundConjunctiveCondition':
        """
        Get the conjunctive condition of the ground action.

        :return: The conjunctive condition of the ground action.
        :rtype: GroundConjunctiveCondition
        """
        return GroundConjunctiveCondition(self._advanced_ground_action.get_conjunctive_condition(), self._problem)

    def get_conditional_effect(self) -> 'list[GroundConditionalEffect]':
        """
        Get the conditional effect of the ground action.

        :return: A list of conditional effects of the ground action.
        :rtype: list[GroundConditionalEffect]
        """
        return [GroundConditionalEffect(x, self._problem) for x in self._advanced_ground_action.get_conditional_effects()]

    def is_applicable(self, state: 'State') -> 'bool':
        """
        Check if the ground action is applicable in the given state.

        :param state: The state to check applicability against.
        :type state: State
        :return: True if the ground action is applicable, False otherwise.
        :rtype: bool
        """
        assert isinstance(state, State), "Invalid state type."
        assert state._problem == self._problem, "State and action belong to different problems."
        return self.get_precondition().holds(state)

    @overload
    def apply(self, state: 'State') -> 'State': ...
    @overload
    def apply(self, state: 'State', return_cost: Lit[False]) -> 'State': ...
    @overload
    def apply(self, state: 'State', return_cost: Lit[True]) -> 'tuple[State, float]': ...
    @overload
    def apply(self, state: 'State', return_cost: 'bool') -> 'Union[State, tuple[State, float]]': ...

    def apply(self, state: 'State', return_cost: 'bool' = False) -> 'Union[State, tuple[State, float]]':
        """
        Apply the ground action to the given state and optionally return the cost.

        :param state: The state to which the ground action will be applied.
        :type state: State
        :param return_cost: Whether to return the cost of the applied action in the state.
        :type return_cost: bool
        :return: The resulting state after applying the ground action and the cost (if return_cost=True).
        :rtype: State or tuple[State, float]
        """
        assert isinstance(state, State), "Invalid state type."
        assert state._problem == self._problem, "State and action belong to different problems."
        sr = self._problem._search_context.get_state_repository()
        advanced_successor_state, cost = sr.get_or_create_successor_state(state._advanced_state, self._advanced_ground_action, 0.0)
        if return_cost:
            return State(advanced_successor_state, self._problem), cost
        else:
            return State(advanced_successor_state, self._problem)

    def __str__(self):
        """
        Get the string representation of the ground action.

        :return: The string representation of the ground action.
        :rtype: str
        """
        return self._advanced_ground_action.to_string_for_plan(self._problem._advanced_problem)

    def __repr__(self):
        """
        Get the string representation of the ground action.

        :return: The string representation of the ground action.
        :rtype: str
        """
        return self._advanced_ground_action.to_string_for_plan(self._problem._advanced_problem)

    def __hash__(self) -> 'int':
        """
        Get the hash of the ground action.

        :return: The hash of the ground action.
        :rtype: int
        """
        return hash(self._advanced_ground_action)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two ground actions are equal.

        :param other: The other ground action to compare with.
        :type other: object
        :return: True if the ground actions are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, GroundAction):
            return False
        return self._advanced_ground_action == other._advanced_ground_action


class Domain:
    """Class representing the PDDL domain."""

    def __init__(self, domain_path: 'Union[Path, str]') -> None:
        """
        Initialize the domain with the given path.

        :param domain_path: The path to the domain definition file.
        :type domain_path: Union[Path, str]
        """
        assert isinstance(domain_path, (Path, str)), "Invalid domain path type."
        self._advanced_parser = AdvancedParser(domain_path, AdvancedParserOptions())
        self._advanced_domain = self._advanced_parser.get_domain()
        self._repositories = self._advanced_domain.get_repositories()

    def get_name(self) -> 'str':
        """
        Get the name of the domain.

        :return: The name of the domain.
        :rtype: str
        """
        return self._advanced_domain.get_name()

    def get_requirements(self) -> 'list[str]':
        """
        Get the requirements of the domain.

        :return: A list of requirements of the domain.
        :rtype: list[str]
        """
        # The requirements are returned as a string in the format '(:requirements :requirement1 :requirement2 ...)'.
        # We remove the '(:requirements' prefix and the ')' suffix.
        requirements = str(self._advanced_domain.get_requirements())[15:-1].split()
        return requirements

    def get_numeric_functions(self, ignore_static: bool = False, ignore_fluent: bool = False, ignore_auxiliary: bool = False) -> 'list[NumericFunction]':
        """
        Get the numeric functions of the domain.

        :param ignore_static: If True, do not include static functions.
        :type ignore_static: bool
        :param ignore_fluent: If True, do not include fluent functions.
        :type ignore_fluent: bool
        :param ignore_auxiliary: If True, do not include auxiliary functions.
        :type ignore_auxiliary: bool
        :return: A list of numeric functions of the domain.
        :rtype: list[NumericFunction]
        """
        numeric_functions = []
        if not ignore_static:
            static_functions = self._advanced_domain.get_static_functions()
            numeric_functions.extend(NumericFunction(x) for x in static_functions)
        if not ignore_fluent:
            fluent_functions = self._advanced_domain.get_fluent_functions()
            numeric_functions.extend(NumericFunction(x) for x in fluent_functions)
        if not ignore_auxiliary:
            auxiliary_function = self._advanced_domain.get_auxiliary_function()
            if auxiliary_function:
                numeric_functions.append(NumericFunction(auxiliary_function))
        return numeric_functions

    def get_actions(self) -> 'list[Action]':
        """
        Get the actions of the domain.

        :return: A list of actions in the domain.
        :rtype: list[Action]
        """
        return [Action(x) for x in self._advanced_domain.get_actions()]

    def get_action(self, action_name: 'str') -> 'Action':
        """
        Get the action of the domain.

        :param action_name: The name of the action to retrieve.
        :type action_name: str
        :return: The action with the specified name.
        :rtype: Action
        """
        assert isinstance(action_name, str), "Invalid action name type."
        for action in self.get_actions():
            if action.get_name() == action_name:
                return action
        raise ValueError(f"Action '{action_name}' not found in the domain.")

    def has_action(self, action_name: 'str') -> 'bool':
        """
        Check if the domain has an action with the specified name.

        :param action_name: The name of the action to check.
        :type action_name: str
        :return: True if the action exists, False otherwise.
        :rtype: bool
        """
        assert isinstance(action_name, str), "Invalid action name type."
        for action in self.get_actions():
            if action.get_name() == action_name:
                return True
        return False

    def get_constants(self) -> 'list[Object]':
        """
        Get the constants of the domain.

        :return: A list of constants in the domain.
        :rtype: list[Object]
        """
        return [Object(x) for x in self._advanced_domain.get_constants()]

    def get_constant(self, constant_name: 'str') -> 'Object':
        """
        Get the constant of the domain.

        :param constant_name: The name of the constant to retrieve.
        :type constant_name: str
        :return: The constant with the specified name.
        :rtype: Object
        """
        assert isinstance(constant_name, str), "Invalid constant name type."
        for constant in self.get_constants():
            if constant.get_name() == constant_name:
                return constant
        raise ValueError(f"Constant '{constant_name}' not found in the domain.")

    def has_constant(self, constant_name: 'str') -> 'bool':
        """
        Check if the domain has a constant with the specified name.

        :param constant_name: The name of the constant to check.
        :type constant_name: str
        :return: True if the constant exists, False otherwise.
        :rtype: bool
        """
        assert isinstance(constant_name, str), "Invalid constant name type."
        for constant in self.get_constants():
            if constant.get_name() == constant_name:
                return True
        return False

    def get_predicates(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[Predicate]':
        """
        Get the predicates of the domain.

        :param ignore_static: If True, do not include static predicates.
        :type ignore_static: bool
        :param ignore_fluent: If True, do not include fluent predicates.
        :type ignore_fluent: bool
        :param ignore_derived: If True, do not include derived predicates.
        :type ignore_derived: bool
        :return: A list of predicates in the domain.
        :rtype: list[Predicate]
        """
        assert isinstance(ignore_static, bool), "Invalid ignore_static type."
        assert isinstance(ignore_fluent, bool), "Invalid ignore_fluent type."
        assert isinstance(ignore_derived, bool), "Invalid ignore_derived type."
        predicates = []
        if not ignore_static:
            predicates.extend(Predicate(x) for x in self._advanced_domain.get_static_predicates())
        if not ignore_fluent:
            predicates.extend(Predicate(x) for x in self._advanced_domain.get_fluent_predicates())
        if not ignore_derived:
            predicates.extend(Predicate(x) for x in self._advanced_domain.get_derived_predicates())
        return predicates

    def get_predicate(self, predicate_name: 'str') -> 'Predicate':
        """
        Get the predicate of the domain.

        :param predicate_name: The name of the predicate to retrieve.
        :type predicate_name: str
        :return: The predicate with the specified name.
        :rtype: Predicate
        """
        assert isinstance(predicate_name, str), "Invalid predicate name type."
        for predicate in self.get_predicates():
            if predicate.get_name() == predicate_name:
                return predicate
        raise ValueError(f"Predicate '{predicate_name}' not found in the domain.")

    def has_predicate(self, predicate_name: 'str') -> 'bool':
        """
        Check if the domain has a predicate with the specified name.

        :param predicate_name: The name of the predicate to check.
        :type predicate_name: str
        :return: True if the predicate exists, False otherwise.
        :rtype: bool
        """
        assert isinstance(predicate_name, str), "Invalid predicate name type."
        for predicate in self.get_predicates():
            if predicate.get_name() == predicate_name:
                return True
        return False


class Problem:
    """Class representing the PDDL problem."""

    def __init__(self, domain: 'Domain', problem_path: Union[Path, str], mode: 'str' = 'lifted') -> None:
        """
        Initialize the problem with the given domain, path, and mode.

        :param domain: The domain to which the problem belongs.
        :type domain: Domain
        :param problem_path: The path to the problem definition file.
        :type problem_path: Union[Path, str]
        :param mode: The mode of the problem, either 'grounded', 'lifted', or 'lifted_symmetry_pruning'.
        :type mode: str
        """
        assert isinstance(domain, Domain), "Invalid domain type."
        assert isinstance(problem_path, (Path, str)), "Invalid problem path type."
        assert isinstance(mode, str), "Invalid mode type."
        if mode == 'grounded':
            search_mode = GroundedOptions()
        elif mode == 'lifted':
            search_mode = LiftedOptions(LiftedKPKCOptions(SymmetryPruning.OFF))
        elif mode == 'lifted_symmetry_pruning':
            search_mode = LiftedOptions(LiftedKPKCOptions(SymmetryPruning.GI))
        else:
            raise ValueError("Invalid mode. Use 'grounded', 'lifted', or 'lifted_symmetry_pruning'.")
        self._domain = domain
        self._mode = mode
        self._advanced_problem = domain._advanced_parser.parse_problem(problem_path, AdvancedParserOptions())
        self._search_context = SearchContext.create(self._advanced_problem, SearchContextOptions(search_mode))
        self._static_ground_atom_indices = { atom.get_index() for atom in self._advanced_problem.get_static_initial_atoms() }

    def _to_advanced_term(self, term: 'Term') -> 'AdvancedTerm':
        assert isinstance(term, (Object, Variable)), "Invalid term type."
        advanced_term = term._advanced_object if isinstance(term, Object) else term._advanced_variable
        return self._advanced_problem.get_or_create_term(advanced_term)

    def get_index(self) -> 'int':
        """
        Get the index of the problem.

        :return: The index of the problem.
        :rtype: int
        """
        return self._advanced_problem.get_index()

    def get_domain(self) -> 'Domain':
        """
        Get the domain of the problem.

        :return: The domain of the problem.
        :rtype: Domain
        """
        return self._domain

    def get_mode(self) -> 'str':
        """
        Get the mode of the problem.

        :return: The mode of the problem.
        :rtype: str
        """
        return self._mode

    def get_name(self) -> 'str':
        """
        Get the name of the problem.

        :return: The name of the problem.
        :rtype: str
        """
        return self._advanced_problem.get_name()

    def get_objects(self) -> 'list[Object]':
        """
        Get the objects of the problem.

        :return: A list of objects in the problem.
        :rtype: list[Object]
        """
        return [Object(x) for x in self._advanced_problem.get_objects()]

    def get_object(self, object_name: 'str') -> 'Object':
        """
        Get the object of the problem.

        :param object_name: The name of the object to retrieve.
        :type object_name: str
        :return: The object with the specified name.
        :rtype: Object
        """
        assert isinstance(object_name, str), "Invalid object name type."
        for obj in self.get_objects():
            if obj.get_name() == object_name:
                return obj
        raise ValueError(f"Object '{object_name}' not found in the problem.")

    def has_object(self, object_name: 'str') -> 'bool':
        """
        Check if the problem has an object with the specified name.

        :param object_name: The name of the object to check.
        :type object_name: str
        :return: True if the object exists, False otherwise.
        :rtype: bool
        """
        assert isinstance(object_name, str), "Invalid object name type."
        for obj in self.get_objects():
            if obj.get_name() == object_name:
                return True
        return False

    def get_requirements(self) -> 'list[str]':
        """
        Get the requirements of the problem.

        :return: A list of requirements of the problem.
        :rtype: list[str]
        """
        requirements = str(self._advanced_problem.get_requirements()).removeprefix('(:requirements').removesuffix(')').split()
        return requirements

    def get_derived_goal_predicates(self) -> 'list[Predicate]':
        """
        Get the derived predicates used in the goal.
        These can be specific to the problem instance.
        This happens, for example, when quantifiers are used in the goal.

        :return: A list of derived predicates used in the goal.
        :rtype: list[Predicate]
        """
        return [Predicate(x.get_atom().get_predicate()) for x in self._advanced_problem.get_derived_goal_literals()]

    def get_derived_problem_predicates(self) -> 'list[Predicate]':
        """
        Get the derived predicates specific to the problem.

        :return: A list of derived predicates specific to the problem.
        :rtype: list[Predicate]
        """
        advanced_domain = self._advanced_problem.get_domain()
        domain_derived_predicates = advanced_domain.get_derived_predicates()
        all_derived_predicates = self._advanced_problem.get_problem_and_domain_derived_predicates()
        return [Predicate(x) for x in all_derived_predicates if x not in domain_derived_predicates]

    def get_initial_state(self) -> 'State':
        """
        Get the initial state of the problem.

        :return: The initial state of the problem.
        :rtype: State
        """
        initial_state, _ = self._search_context.get_state_repository().get_or_create_initial_state()
        return State(initial_state, self)

    def get_initial_atoms(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[GroundAtom]':
        """
        Get the initial atoms of the problem.

        :param ignore_static: If True, do not include static atoms.
        :type ignore_static: bool
        :param ignore_fluent: If True, do not include fluent atoms.
        :type ignore_fluent: bool
        :param ignore_derived: If True, do not include derived atoms.
        :type ignore_derived: bool
        :return: A list of initial ground atoms in the problem.
        :rtype: list[GroundAtom]
        """
        assert isinstance(ignore_static, bool), "Invalid ignore_static type."
        assert isinstance(ignore_fluent, bool), "Invalid ignore_fluent type."
        assert isinstance(ignore_derived, bool), "Invalid ignore_derived type."
        problem: 'AdvancedProblem' = self._search_context.get_problem()
        initial_atoms = []
        # Add static atoms if ignore_static is False
        if not ignore_static:
            static_atoms = problem.get_static_initial_atoms()
            initial_atoms.extend([GroundAtom(x) for x in static_atoms])
        # Add fluent atoms
        if not ignore_fluent:
            fluent_atoms = problem.get_fluent_initial_atoms()
            initial_atoms.extend([GroundAtom(x) for x in fluent_atoms])
        # Add derived atoms if ignore_derived is False
        if not ignore_derived:
            initial_state, _ = self._search_context.get_state_repository().get_or_create_initial_state()
            derived_indices = list(initial_state.get_derived_atoms())
            derived_atoms = problem.get_repositories().get_derived_ground_atoms_from_indices(derived_indices)
            initial_atoms.extend([GroundAtom(x) for x in derived_atoms])
        return initial_atoms

    def get_initial_function_values(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[tuple[NumericFunction, list[Object], float]]':
        raise NotImplementedError()

    def get_goal_condition(self) -> 'GroundConjunctiveCondition':
        """
        Get the propositional goal condition of the problem.

        :return: The goal condition of the problem.
        :rtype: GroundConjunctiveCondition
        """
        return GroundConjunctiveCondition(self._advanced_problem.get_goal_condition(), self)

    def new_atom(self, predicate: 'Predicate', terms: 'list[Term]') -> 'Atom':
        """
        Create an atom from the given predicate and terms.

        :param predicate: The predicate for the atom.
        :type predicate: Predicate
        :param terms: A list of terms (Objects or Variables) for the atom.
        :type terms: list[Term]
        :return: An Atom object representing the atom.
        :rtype: Atom
        """
        assert isinstance(predicate, Predicate), "Invalid predicate type."
        assert isinstance(terms, list), "Invalid terms type."
        advanced_terms = AdvancedTermList([self._to_advanced_term(term) for term in terms])
        return Atom(self._advanced_problem.get_or_create_atom(predicate._advanced_predicate, advanced_terms))

    def new_literal(self, atom: 'Atom', polarity: 'bool') -> 'Literal':
        """
        Create a literal with the given polarity.

        :param atom: The atom for the literal.
        :type atom: Atom
        :param polarity: The polarity of the literal (True for positive, False for negative).
        :type polarity: bool
        :return: A Literal object representing the literal.
        :rtype: Literal
        """
        assert isinstance(atom, Atom), "Invalid atom type."
        return Literal(self._advanced_problem.get_or_create_literal(polarity, atom._advanced_atom))

    def new_ground_atom(self, predicate: 'Predicate', objects: 'list[Object]') -> 'GroundAtom':
        """
        Create a ground atom from the given predicate and objects.

        :param predicate: The predicate for the ground atom.
        :type predicate: Predicate
        :param objects: A list of objects for the ground atom.
        :type objects: list[Object]
        :return: A GroundAtom object representing the ground atom.
        :rtype: GroundAtom
        """
        assert isinstance(predicate, Predicate), "Invalid predicate type."
        advanced_objects = AdvancedObjectList([x._advanced_object for x in objects])
        return GroundAtom(self._advanced_problem.get_or_create_ground_atom(predicate._advanced_predicate, advanced_objects))

    def new_ground_literal(self, ground_atom: 'GroundAtom', polarity: 'bool') -> 'GroundLiteral':
        """
        Create a ground literal with the given polarity.

        :param ground_atom: The ground atom for the ground literal.
        :type ground_atom: GroundAtom
        :param polarity: The polarity of the ground literal (True for positive, False for negative).
        :type polarity: bool
        :return: A GroundLiteral object representing the ground literal.
        :rtype: GroundLiteral
        """
        assert isinstance(ground_atom, GroundAtom), "Invalid ground atom type."
        predicate = ground_atom.get_predicate()
        # variables = predicate.get_parameters()  # TODO: This does not work, for some reason.
        variables = self.new_variable_list(['?x' + str(i + 1) for i in range(predicate.get_arity())])  # TODO: Why does this resolve the issue?
        advanced_terms = AdvancedTermList([self._to_advanced_term(variable) for variable in variables])
        advanced_atom = self._advanced_problem.get_or_create_atom(predicate._advanced_predicate, advanced_terms)
        advanced_literal = self._advanced_problem.get_or_create_literal(polarity, advanced_atom)
        advanced_objects = ground_atom._advanced_ground_atom.get_objects()
        return GroundLiteral(self._advanced_problem.ground(advanced_literal, advanced_objects))

    def new_variable(self, name: 'str', parameter_index: 'int') -> 'Variable':
        """
        Create a new variable with the given name and parameter index.

        :param name: The name of the variable.
        :type name: str
        :param parameter_index: The index of the parameter for the variable.
        :type parameter_index: int
        :return: A Variable object representing the new variable.
        :rtype: Variable
        """
        assert isinstance(name, str), "Invalid variable name type."
        assert isinstance(parameter_index, int), "Invalid parameter index type."
        return Variable(self._advanced_problem.get_or_create_variable(name, parameter_index))

    def new_variable_list(self, names: 'list[str]') -> 'list[Variable]':
        """
        Create a list of new variables with the given names.

        :param names: A list of names for the variables.
        :type names: list[str]
        :return: A list of Variable objects representing the new variables.
        :rtype: list[Variable]
        """
        assert isinstance(names, list), "Invalid names type."
        return [self.new_variable(name, parameter_index) for name, parameter_index in zip(names, range(len(names)))]

    def new_conjunctive_condition(self, variables: 'list[Variable]', literals: 'list[Literal]') -> 'ConjunctiveCondition':
        """
        Create a conjunctive condition from the given parameters, literals, and ground literals.

        :param variables: A list of variables for the conjunctive condition.
        :type variables: list[Variable]
        :param literals: A list of literals for the conjunctive condition.
        :type literals: list[Literal]
        :return: A ConjunctiveCondition object representing the conjunctive condition.
        :rtype: ConjunctiveCondition
        """
        assert isinstance(variables, list), "Invalid variables type."
        assert isinstance(literals, list), "Invalid literals type."
        advanced_variables = AdvancedVariableList(x._advanced_variable for x in variables)
        advanced_parameters = AdvancedParameterList(self._advanced_problem.get_or_create_parameter(x, AdvancedTypeList()) for x in advanced_variables)
        advanced_static_literals, advanced_fluent_literals, advanced_derived_literals = _split_literal_list(literals)
        advanced_numeric_constraints = AdvancedNumericConstraintList()
        advanced_conjunctive_condition = self._advanced_problem.get_or_create_conjunctive_condition(advanced_parameters,
                                                                                                    advanced_static_literals,
                                                                                                    advanced_fluent_literals,
                                                                                                    advanced_derived_literals,
                                                                                                    advanced_numeric_constraints)
        return ConjunctiveCondition(advanced_conjunctive_condition)

    def new_ground_conjunctive_condition(self, ground_literals: 'list[GroundLiteral]') -> 'GroundConjunctiveCondition':
        """
        Create a ground conjunctive condition from the given ground literals.

        :param ground_literals: A list of ground literals for the conjunctive condition.
        :type ground_literals: list[GroundLiteral]
        :param problem: The problem instance to which the ground conjunctive condition belongs.
        :type problem: Problem
        :return: A GroundConjunctiveCondition object representing the ground conjunctive condition.
        :rtype: GroundConjunctiveCondition
        """
        assert isinstance(ground_literals, list), "Invalid ground literals type."
        static_literals, fluent_literals, derived_literals = _split_ground_literal_list(ground_literals)
        advanced_condition = self._advanced_problem.get_or_create_ground_conjunctive_condition(static_literals, fluent_literals, derived_literals, AdvancedGroundNumericConstraintList())
        return GroundConjunctiveCondition(advanced_condition, self)

    def new_ground_action(self, action: 'Action', objects: 'list[Object]') -> 'GroundAction':
        """
        Create a new ground action with the given action schema and objects.

        :param action: The action schema of the ground action.
        :type action: Action
        :param objects: The objects of the ground action.
        :type objects: list[Object]
        :return: A new GroundAction instance.
        :rtype: GroundAction
        """
        assert isinstance(action, Action), "Invalid action type."
        assert isinstance(objects, list), "Invalid objects type."
        advanced_objects = AdvancedObjectList([x._advanced_object for x in objects])
        advanced_ground_action = self._advanced_problem.ground(action._advanced_action, advanced_objects)
        return GroundAction(advanced_ground_action, self)


class State:
    """
    Class representing a state in the search space.
    """

    def __init__(self, advanced_state: 'AdvancedState', problem: 'Problem') -> None:
        """
        Internal constructor for the State class; to create a state, use the new() method.

        :param advanced_state: An instance of an advanced state.
        :type advanced_state: AdvancedState
        :param problem: The problem instance to which the state belongs.
        :type advanced_state: AdvancedState
        """
        assert isinstance(advanced_state, AdvancedState), "Invalid advanced state type."
        assert isinstance(problem, Problem), "Invalid problem type."
        self._problem = problem
        self._advanced_state = advanced_state

    def get_index(self) -> 'int':
        """
        Returns the index of the state.

        :return: The index of the state.
        :rtype: int
        """
        return self._advanced_state.get_index()

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance to which the state belongs.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def get_atoms(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[GroundAtom]':
        """
        Returns the ground atoms of the state.

        :param ignore_static: If True, do not include static ground atoms.
        :type ignore_static: bool
        :param ignore_fluent: If True, do not include fluent ground atoms.
        :type ignore_fluent: bool
        :param ignore_derived: If True, do not include derived ground atoms.
        :type ignore_derived: bool
        :return: A list of ground atoms in the state.
        :rtype: list[GroundAtom]
        """
        ground_atoms = []
        if not ignore_static:
             static_ground_atoms = self._problem.get_initial_atoms(ignore_static = False, ignore_fluent = True, ignore_derived = True)
             ground_atoms.extend(static_ground_atoms)
        if not ignore_fluent:
            fluent_indices = list(self._advanced_state.get_fluent_atoms())
            fluent_ground_atoms = self._problem._advanced_problem.get_repositories().get_fluent_ground_atoms_from_indices(fluent_indices)
            ground_atoms.extend([GroundAtom(x) for x in fluent_ground_atoms])
        if not ignore_derived:
            derived_indices = list(self._advanced_state.get_derived_atoms())
            derived_ground_atoms = self._problem._advanced_problem.get_repositories().get_derived_ground_atoms_from_indices(derived_indices)
            ground_atoms.extend([GroundAtom(x) for x in derived_ground_atoms])
        return ground_atoms

    def get_function_values(self, ignore_static: bool = False, ignore_fluent: bool = False, ignore_auxiliary: bool = False) -> 'list[tuple[NumericFunction, list[Object], float]]':
        def helper_function(function_value: 'Union[AdvancedStaticGroundFunctionValue, AdvancedFluentGroundFunctionValue, AdvancedAuxiliaryGroundFunctionValue]') -> 'tuple[NumericFunction, list[Object], float]':
            function = function_value.get_function()
            schema = NumericFunction(function.get_function_skeleton())
            terms = [Object(x) for x in function.get_objects()]
            value = function_value.get_number()
            return (schema, terms, value)
        advanced_problem = self.get_problem()._advanced_problem
        function_values = []
        if not ignore_static:
            static_function_values = advanced_problem.get_static_function_values()
            function_values.extend(helper_function(x) for x in static_function_values)
        if not ignore_fluent:
            fluent_function_values = advanced_problem.get_fluent_function_values()
            function_values.extend(helper_function(x) for x in fluent_function_values)
        if not ignore_auxiliary:
            auxiliary_function_value = advanced_problem.get_auxiliary_function_value()
            if auxiliary_function_value:
                function_values.append(helper_function(auxiliary_function_value))
        return function_values

    def contains(self, ground_atom: 'GroundAtom') -> bool:
        """
        Checks if a ground atom is contained in the state.

        :param ground_atom: The ground atom to check for containment.
        :type ground_atom: GroundAtom
        :return: True if the ground atom is contained in the state, False otherwise.
        :rtype: bool
        """
        # TODO: This function is quite expensive. Expose a more efficient function on the C++ side.
        assert isinstance(ground_atom, GroundAtom) \
            or isinstance(ground_atom, AdvancedStaticGroundAtom) \
            or isinstance(ground_atom, AdvancedFluentGroundAtom) \
            or isinstance(ground_atom, AdvancedDerivedGroundAtom), "Invalid ground atom type."
        if isinstance(ground_atom, GroundAtom):
            if ground_atom.is_static():
                return ground_atom.get_index() in self._problem._static_ground_atom_indices
            if ground_atom.is_fluent():
                return ground_atom.get_index() in self._advanced_state.get_fluent_atoms()
            if ground_atom.is_derived():
                return ground_atom.get_index() in self._advanced_state.get_derived_atoms()
        else:
            if isinstance(ground_atom, AdvancedStaticGroundAtom):
                return ground_atom.get_index() in self._problem._static_ground_atom_indices
            if isinstance(ground_atom, AdvancedFluentGroundAtom):
                return ground_atom.get_index() in self._advanced_state.get_fluent_atoms()
            if isinstance(ground_atom, AdvancedDerivedGroundAtom):
                return ground_atom.get_index() in self._advanced_state.get_derived_atoms()
        raise RuntimeError("Internal error, please report this issue.")

    def contains_all(self, ground_atoms: 'list[GroundAtom]') -> bool:
        """
        Checks if all ground atoms are contained in the state.

        :param ground_atoms: A list of ground atoms to check for containment.
        :type ground_atoms: list[GroundAtom]
        :return: True if all ground atoms are contained in the state, False otherwise.
        :rtype: bool
        """
        assert isinstance(ground_atoms, list) \
            or isinstance(ground_atoms, AdvancedStaticGroundAtomList) \
            or isinstance(ground_atoms, AdvancedFluentGroundAtomList) \
            or isinstance(ground_atoms, AdvancedDerivedGroundAtomList), "Invalid ground atoms type."
        for ground_atom in ground_atoms:
            if not self.contains(ground_atom):
                return False
        return True

    def contains_none(self, ground_atoms: 'list[GroundAtom]') -> bool:
        """
        Checks if none of the ground atoms are contained in the state.

        :param ground_atoms: A list of ground atoms to check for containment.
        :type ground_atoms: list[GroundAtom]
        :return: True if none of the ground atoms are contained in the state, False otherwise.
        :rtype: bool
        """
        assert isinstance(ground_atoms, list) \
            or isinstance(ground_atoms, AdvancedStaticGroundAtomList) \
            or isinstance(ground_atoms, AdvancedFluentGroundAtomList) \
            or isinstance(ground_atoms, AdvancedDerivedGroundAtomList), "Invalid ground atoms type."
        for ground_atom in ground_atoms:
            if self.contains(ground_atom):
                return False
        return True

    def literal_holds(self, literal: 'GroundLiteral') -> bool:
        """
        Checks if a literal holds in the state.

        :param literal: The ground literal to check.
        :type literal: GroundLiteral
        :return: True if the literal holds in the state, False otherwise.
        :rtype: bool
        """
        if isinstance(literal._advanced_ground_literal, AdvancedStaticGroundLiteral):
            atom_is_in_state = (literal.get_atom().get_index() in self._problem._static_ground_atom_indices)
            return literal.get_polarity() == atom_is_in_state
        return self._advanced_state.literal_holds(literal._advanced_ground_literal)

    def _literals_hold_static(self, literals: 'AdvancedStaticGroundLiteralList') -> bool:
        if len(literals) == 0:
            return True
        static_positive_indices = { literal.get_atom().get_index() for literal in literals if literal.get_polarity() }
        static_negative_indices = { literal.get_atom().get_index() for literal in literals if not literal.get_polarity() }
        return self._problem._static_ground_atom_indices.issuperset(static_positive_indices) and \
               self._problem._static_ground_atom_indices.isdisjoint(static_negative_indices)

    def _literals_hold_fluent(self, literals: 'AdvancedFluentGroundLiteralList') -> bool:
        if len(literals) == 0:
            return True
        return self._advanced_state.literal_holds(literals)

    def _literals_hold_derived(self, literals: 'AdvancedDerivedGroundLiteralList') -> bool:
        if len(literals) == 0:
            return True
        return self._advanced_state.literal_holds(literals)

    def literals_hold(self, literals: 'list[GroundLiteral]', test_static = False) -> bool:
        """
        Checks if all literals hold in the state.

        :param literals: A list of ground literals to check.
        :type literals: list[GroundLiteral]
        :param test_static: If True, also check static literals.
        :type test_static: bool
        :return: True if all literals hold in the state, False otherwise.
        :rtype: bool
        """
        assert isinstance(literals, list), "Invalid literals type."
        advanced_static_ground_literals, advanced_fluent_ground_literals, advanced_derived_ground_literals = _split_ground_literal_list(literals)
        holds = self._literals_hold_static(advanced_static_ground_literals) if test_static else True
        holds = holds and self._literals_hold_fluent(advanced_fluent_ground_literals)
        holds = holds and self._literals_hold_derived(advanced_derived_ground_literals)
        return holds

    def numeric_holds(self, numeric: 'GroundNumericCondition') -> bool:
        """
        Checks if a numeric condition holds in the state.

        :param numeric: The ground numeric condition to check.
        :type numeric: GroundNumericCondition
        :return: True if the numeric condition holds in the state, False otherwise.
        :rtype: bool
        """
        assert isinstance(numeric, GroundNumericCondition), "Invalid numeric condition type."
        raise NotImplementedError('numeric_holds is not implemented yet.')

    def generate_applicable_actions(self, cache_result = True) -> 'list[GroundAction]':
        """
        Generates a list of all applicable ground actions in the state.

        :param cache_result: If True, cache the result for future calls.
        :type cache_result: bool
        :return: A list of applicable ground actions in the state.
        :rtype: list[GroundAction]
        """
        if hasattr(self, '_applicable_actions'):
            return self._applicable_actions
        aag = self._problem._search_context.get_applicable_action_generator()
        result = [GroundAction(x, self._problem) for x in aag.generate_applicable_actions(self._advanced_state)]
        result.sort(key=lambda x: x.get_index())  # Sort by index for consistency
        if cache_result:
            self._applicable_actions = result
        return result

    def __str__(self) -> 'str':
        """
        Returns a string representation of the state.

        :return: A string representation of the state.
        :rtype: str
        """
        return str(self._advanced_state)

    def __repr__(self) -> 'str':
        """
        Returns a string representation of the state.

        :return: A string representation of the state.
        :rtype: str
        """
        return self.__str__()

    def __hash__(self) -> 'int':
        """
        Returns a hash of the state.

        :return: A hash of the state.
        :rtype: int
        """
        return hash(self._advanced_state)

    def __eq__(self, other) -> bool:
        """
        Checks if two states are equal.

        :param other: The other state to compare with.
        :type other: State
        :return: True if the states are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, State):
            return False
        return self._advanced_state == other._advanced_state


class GroundConjunctiveCondition:
    """Class representing a ground conjunctive condition of ground literals."""

    def __init__(self, advanced_condition: 'AdvancedGroundConjunctiveCondition', problem: 'Problem') -> None:
        """
        Initialize the ground conjunctive condition with the given parameters. It is not possible to create AdvancedGroundConjunctiveCondition directly, so this class is used as a wrapper.

        :param advanced_condition: An instance of an advanced ground conjunctive condition.
        :type advanced_condition: AdvancedGroundConjunctiveCondition
        :param problem: The problem instance to which the ground conjunctive condition belongs.
        :type problem: Problem
        """
        assert isinstance(advanced_condition, AdvancedGroundConjunctiveCondition), "Invalid advanced condition type."
        assert isinstance(problem, Problem), "Invalid problem type."
        self._advanced_condition = advanced_condition
        self._problem = problem
        repositories = problem._advanced_problem.get_repositories()
        self._static_neg_advanced_ground_atoms = repositories.get_static_ground_atoms_from_indices(list(self._advanced_condition.get_static_negative_condition()))
        self._static_pos_advanced_ground_atoms = repositories.get_static_ground_atoms_from_indices(list(self._advanced_condition.get_static_positive_condition()))
        self._fluent_neg_advanced_ground_atoms = repositories.get_fluent_ground_atoms_from_indices(list(self._advanced_condition.get_fluent_negative_condition()))
        self._fluent_pos_advanced_ground_atoms = repositories.get_fluent_ground_atoms_from_indices(list(self._advanced_condition.get_fluent_positive_condition()))
        self._derived_neg_advanced_ground_atoms = repositories.get_derived_ground_atoms_from_indices(list(self._advanced_condition.get_derived_negative_condition()))
        self._derived_pos_advanced_ground_atoms = repositories.get_derived_ground_atoms_from_indices(list(self._advanced_condition.get_derived_positive_condition()))
        self._static_pos_ground_atoms = [GroundAtom(x) for x in self._static_pos_advanced_ground_atoms]
        self._static_neg_ground_atoms = [GroundAtom(x) for x in self._static_neg_advanced_ground_atoms]
        self._fluent_pos_ground_atoms = [GroundAtom(x) for x in self._fluent_pos_advanced_ground_atoms]
        self._fluent_neg_ground_atoms = [GroundAtom(x) for x in self._fluent_neg_advanced_ground_atoms]
        self._derived_pos_ground_atoms = [GroundAtom(x) for x in self._derived_pos_advanced_ground_atoms]
        self._derived_neg_ground_atoms = [GroundAtom(x) for x in self._derived_neg_advanced_ground_atoms]
        self._static_ground_literals = [GroundLiteral.new(x, True, self._problem) for x in self._static_pos_ground_atoms]
        self._static_ground_literals += [GroundLiteral.new(x, False, self._problem) for x in self._static_neg_ground_atoms]
        self._fluent_ground_literals = [GroundLiteral.new(x, True, self._problem) for x in self._fluent_pos_ground_atoms]
        self._fluent_ground_literals += [GroundLiteral.new(x, False, self._problem) for x in self._fluent_neg_ground_atoms]
        self._derived_ground_literals = [GroundLiteral.new(x, True, self._problem) for x in self._derived_pos_ground_atoms]
        self._derived_ground_literals += [GroundLiteral.new(x, False, self._problem) for x in self._derived_neg_ground_atoms]
        self._numerics = [GroundNumericCondition(x) for x in self._advanced_condition.get_numeric_constraints()]

    @staticmethod
    def new(ground_literals: 'list[GroundLiteral]', problem: 'Problem') -> 'GroundConjunctiveCondition':
        """
        Create a ground conjunctive condition from the given ground literals.

        :param ground_literals: A list of ground literals for the conjunctive condition.
        :type ground_literals: list[GroundLiteral]
        :param problem: The problem instance to which the ground conjunctive condition belongs.
        :type problem: Problem
        :return: A GroundConjunctiveCondition object representing the ground conjunctive condition.
        :rtype: GroundConjunctiveCondition
        """
        return problem.new_ground_conjunctive_condition(ground_literals)

    def get_problem(self) -> 'Problem':
        """
        Get the problem instance to which the ground conjunctive condition belongs.

        :return: The problem instance.
        :rtype: Problem
        """
        return self._problem

    def get_literals(self, ignore_static: 'bool' = False, ignore_fluent: 'bool' = False,  ignore_derived: 'bool' = False) -> 'list[GroundLiteral]':
        """
        Get the ground literals of the ground conjunctive condition.

        :param ignore_static: If True, do not include static ground literals.
        :type ignore_static: bool
        :param ignore_fluent: If True, do not include fluent ground literals.
        :type ignore_fluent: bool
        :param ignore_derived: If True, do not include derived ground literals.
        :type ignore_derived: bool
        :return: A list of ground literals in the condition.
        :rtype: list[GroundLiteral]
        """
        literals = []
        if not ignore_static:
            literals.extend(self._static_ground_literals)
        if not ignore_fluent:
            literals.extend(self._fluent_ground_literals)
        if not ignore_derived:
            literals.extend(self._derived_ground_literals)
        return literals

    def get_numerics(self) -> 'list[GroundNumericCondition]':
        """
        Get the numeric constraints of the ground conjunctive condition.

        :return: A list of ground numeric constraints in the condition.
        :rtype: list[GroundNumericCondition]
        """
        return self._numerics

    def holds(self, state: 'State') -> bool:
        """
        Check if the ground conjunctive condition holds in the given state.

        :param state: The state to check the condition against.
        :type state: State
        :return: True if the condition holds in the state, False otherwise.
        :rtype: bool
        """
        assert isinstance(state, State), "Invalid state type."
        return all(state.literal_holds(literal) for literal in self.get_literals()) and all(state.numeric_holds(numeric) for numeric in self.get_numerics())

    def lift(self, add_inequalities: bool = False) -> 'ConjunctiveCondition':
        """
        Lift the ground conjunctive condition to a conjunctive condition.

        :param add_inequalities: If True, add inequalities to the lifted condition. This is ignored if the domain does not support equality.
        :type add_inequalities: bool
        :return: A ConjunctiveCondition object representing the lifted condition.
        :rtype: ConjunctiveCondition
        """
        problem = self._problem
        variable_map = {}
        lifted_literals = []
        # Lift the ground literals in the  ground conjunctive condition.
        for literal in self.__iter__():
            if isinstance(literal, GroundLiteral):
                predicate = literal.get_atom().get_predicate()
                polarity = literal.get_polarity()
                grounded_terms = literal.get_atom().get_terms()
                lifted_terms = []
                for obj in grounded_terms:
                    if obj.get_index() in variable_map:
                        variable = variable_map[obj.get_index()]
                    else:
                        variable_id = len(variable_map)
                        variable = Variable.new(f'?x{variable_id}', variable_id, problem)
                        variable_map[obj.get_index()] = variable
                    lifted_terms.append(variable)
                lifted_atom = Atom.new(predicate, lifted_terms, problem)
                lifted_literals.append(Literal.new(lifted_atom, polarity, problem))
        # If the domain supports :equality, add inequalities if asked for.
        domain = problem.get_domain()
        requirements = domain.get_requirements()
        if add_inequalities and ':equality' in requirements:
            predicate_equal = domain.get_predicate('=')
            keys = list(variable_map.keys())
            for idx_i in range(len(keys)):
                for idx_j in range(idx_i + 1, len(keys)):
                    var_i = variable_map[keys[idx_i]]
                    var_j = variable_map[keys[idx_j]]
                    equality_atom = Atom.new(predicate_equal, [var_i, var_j], problem)
                    inequality_literal = Literal.new(equality_atom, False, problem)
                    lifted_literals.append(inequality_literal)
        # Construct the lifted conjunctive condition
        variables = list(variable_map.values())
        variables.sort(key=lambda x: x.get_index())
        return ConjunctiveCondition.new(variables, lifted_literals, problem)

    def __iter__(self) -> 'Iterator[Union[GroundLiteral, GroundNumericCondition]]':
        """
        Iterate over all ground literals and numeric conditions.

        :return: An iterable of GroundLiteral and GroundNumericCondition objects representing the conditions in the conjunction.
        :rtype: Iterator[Union[GroundLiteral, GroundNumericCondition]]
        """
        yield from self._static_ground_literals
        yield from self._fluent_ground_literals
        yield from self._derived_ground_literals
        yield from self.get_numerics()

    def __len__(self) -> 'int':
        """
        Get the number of literals in the ground conjunctive condition.

        :return: The number of literals in the condition.
        :rtype: int
        """
        return len(self._static_ground_literals) + len(self._fluent_ground_literals) + len(self._derived_ground_literals)

    def __str__(self):
        """
        Get a string representation of the ground conjunctive condition.

        :return: A string representation of the ground conjunctive condition.
        :rtype: str
        """
        return "GroundConjunctiveCondition([" + ", ".join(str(x) for x in self.__iter__()) + "])"

    def __repr__(self):
        """
        Get a string representation of the ground conjunctive condition.

        :return: A string representation of the ground conjunctive condition.
        :rtype: str
        """
        return str(self)


class ConjunctiveCondition:
    """Class representing a conjunctive condition of literals."""

    def __init__(self, advanced_condition: 'AdvancedConjunctiveCondition') -> None:
        """
        Internal constructor for the ConjunctiveCondition class; to create a conjunctive condition, use the new() method.

        :param advanced_condition: An instance of an advanced conjunctive condition.
        :type advanced_condition: AdvancedConjunctiveCondition
        """
        assert isinstance(advanced_condition, AdvancedConjunctiveCondition), "Invalid conjunctive condition type."
        self._advanced_conjunctive_condition = advanced_condition

    @staticmethod
    def new(variables: 'list[Variable]', literals: 'list[Literal]', problem: 'Problem') -> 'ConjunctiveCondition':
        """
        Create a conjunctive condition from the given parameters, literals, and ground literals.

        :param variables: A list of variables for the conjunctive condition.
        :type variables: list[Variable]
        :param literals: A list of literals for the conjunctive condition.
        :type literals: list[Literal]
        :param problem: The problem instance to which the conjunctive condition belongs.
        :type problem: Problem
        :return: A ConjunctiveCondition object representing the conjunctive condition.
        :rtype: ConjunctiveCondition
        """
        return problem.new_conjunctive_condition(variables, literals)

    @staticmethod
    def parse(problem: 'Problem', **words) -> 'ConjunctiveCondition':
        """Parse a conjunctive condition from the given words.
        Example: ConjunctiveCondition.parse(problem, 'at', 'robot1', '?l', 'box', '?b', 'not', 'at', '?b', '?l')

        :param problem: The problem to create the conjunctive condition for.
        :type problem: Problem
        :param words: A sequence of words representing polarities, predicate names, variables, and constants.
        :type words: str
        :return: A ConjunctiveCondition object representing the parsed condition.
        :rtype: ConjunctiveCondition
        """
        assert isinstance(problem, Problem), "Invalid problem type."
        raise NotImplementedError("Parsing of conjunctive conditions is not implemented yet.")


    def get_parameters(self) -> 'list[Variable]':
        """
        Get the parameters of the conjunctive condition.

        :return: A list of variables representing the parameters of the conjunctive condition.
        :rtype: list[Variable]
        """
        return [Variable(x) for x in self._advanced_conjunctive_condition.get_parameters()]

    def get_literals(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[Literal]':
        """
        Get the literals of the conjunctive condition.

        :param ignore_static: If True, do not include static literals.
        :type ignore_static: bool
        :param ignore_fluent: If True, do not include fluent literals.
        :type ignore_fluent: bool
        :param ignore_derived: If True, do not include derived literals.
        :type ignore_derived: bool
        :return: A list of literals in the conjunctive condition.
        :rtype: list[Literal]
        """
        literals = []
        if not ignore_static:
            literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_static_literals()])
        if not ignore_fluent:
            literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_fluent_literals()])
        if not ignore_derived:
            literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_derived_literals()])
        return literals

    def get_nullary_ground_literals(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[GroundLiteral]':
        """
        Get the nullary ground literals of the conjunctive condition.

        :param ignore_static: If True, do not include static ground literals.
        :type ignore_static: bool
        :param ignore_fluent: If True, do not include fluent ground literals.
        :type ignore_fluent: bool
        :param ignore_derived: If True, do not include derived ground literals.
        :type ignore_derived: bool
        :return: A list of nullary ground literals in the conjunctive condition.
        :rtype: list[GroundLiteral]
        """
        ground_literals = []
        if not ignore_static:
            ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_static_literals()])
        if not ignore_fluent:
            ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_fluent_literals()])
        if not ignore_derived:
            ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_derived_literals()])
        return ground_literals

    def get_numeric_conditions(self) -> 'list[NumericCondition]':
        advanced_numeric_constraints = self._advanced_conjunctive_condition.get_numeric_constraints()
        return [NumericCondition(x) for x in advanced_numeric_constraints]

    def ground(self, state: 'State', max_groundings: int = -1, blacklist: 'Union[list[Predicate], None]' = None) -> 'list[GroundConjunctiveCondition]':
        """
        Ground the conjunctive condition.

        :param state: The state to ground the condition against.
        :type state: State
        :param max_groundings: The maximum number of groundings to generate. If -1, generate all groundings.
        :type max_groundings: int
        :param blacklist: A list of predicates to be removed from the resulting groundings; these predicates are still included in the grounding process.
        :type blacklist: list[Predicate] | None
        :return: A list of GroundConjunctiveCondition objects representing the grounded conditions.
        :rtype: list[GroundConjunctiveCondition]
        """
        assert isinstance(state, State), "Invalid state type."
        assert isinstance(max_groundings, int), "Invalid max_groundings type."
        assert blacklist is None or isinstance(blacklist, list), "Invalid blacklist type."
        assert len(self.get_numeric_conditions()) == 0, "Numeric conditions are not supported."
        if max_groundings < 0:
            max_groundings = 1_000_000_000  # Generate all groundings
        problem = state._problem
        advanced_problem = problem._advanced_problem
        if not hasattr(self, '_satisficing_binding_generator'):
            self._satisficing_binding_generator = ConjunctiveConditionSatisficingBindingGenerator(self._advanced_conjunctive_condition, advanced_problem)
        result = []
        if (blacklist is None) or (len(blacklist) == 0):
            groundings = self._satisficing_binding_generator.generate_ground_conjunctions(state._advanced_state, max_groundings)
            for objects, (static_ground_literals, fluent_ground_literals, derived_ground_literals) in groundings:
                advanced_condition = advanced_problem.get_or_create_ground_conjunctive_condition(static_ground_literals, fluent_ground_literals, derived_ground_literals, AdvancedGroundNumericConstraintList())
                result.append(GroundConjunctiveCondition(advanced_condition, problem))
        else:
            static_blacklist = {x.get_index() for x in blacklist if x.is_static()}
            fluent_blacklist = {x.get_index() for x in blacklist if x.is_fluent()}
            derived_blacklist = {x.get_index() for x in blacklist if x.is_derived()}
            assert (len(static_blacklist) + len(fluent_blacklist) + len(derived_blacklist)) == len(blacklist), "Invalid blacklist, contains non-predicate elements."
            groundings = self._satisficing_binding_generator.generate_ground_conjunctions(state._advanced_state, max_groundings)
            for objects, (static_ground_literals, fluent_ground_literals, derived_ground_literals) in groundings:
                static_ground_literals = AdvancedStaticGroundLiteralList([x for x in static_ground_literals if x.get_atom().get_predicate().get_index() not in static_blacklist])
                fluent_ground_literals = AdvancedFluentGroundLiteralList([x for x in fluent_ground_literals if x.get_atom().get_predicate().get_index() not in fluent_blacklist])
                derived_ground_literals = AdvancedDerivedGroundLiteralList([x for x in derived_ground_literals if x.get_atom().get_predicate().get_index() not in derived_blacklist])
                advanced_condition = advanced_problem.get_or_create_ground_conjunctive_condition(static_ground_literals, fluent_ground_literals, derived_ground_literals, AdvancedGroundNumericConstraintList())
                result.append(GroundConjunctiveCondition(advanced_condition, problem))
        return result

    def __str__(self):
        """
        Get the string representation of the conjunctive condition.

        :return: A string representation of the conjunctive condition.
        :rtype: str
        """
        return str(self._advanced_conjunctive_condition)

    def __repr__(self):
        """
        Get the string representation of the conjunctive condition.

        :return: A string representation of the conjunctive condition.
        :rtype: str
        """
        return str(self._advanced_conjunctive_condition)

    def __hash__(self) -> 'int':
        """
        Get the hash of the conjunctive condition.

        :return: A hash of the conjunctive condition.
        :rtype: int
        """
        return hash(self._advanced_conjunctive_condition)

    def __eq__(self, other: 'object') -> bool:
        """
        Check if two conjunctive conditions are equal.

        :param other: The other conjunctive condition to compare with.
        :type other: ConjunctiveCondition
        :return: True if the conjunctive conditions are equal, False otherwise.
        :rtype: bool
        """
        if not isinstance(other, ConjunctiveCondition):
            return False
        return self._advanced_conjunctive_condition == other._advanced_conjunctive_condition


class NumericFunction:
    def __init__(self, advanced_function_skeleton: 'AdvancedFunctionSkeleton') -> None:
        assert isinstance(advanced_function_skeleton, AdvancedStaticFunctionSkeleton) \
            or isinstance(advanced_function_skeleton, AdvancedFluentFunctionSkeleton) \
            or isinstance(advanced_function_skeleton, AdvancedAuxiliaryFunctionSkeleton), "Invalid advanced function skeleton type."
        self._advanced_function_skeleton = advanced_function_skeleton

    def is_static(self) -> 'bool':
        """
        Get whether the numeric function is static.

        :return: True if the numeric function is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_function_skeleton, AdvancedStaticFunctionSkeleton)

    def is_fluent(self) -> 'bool':
        """
        Get whether the numeric function is fluent.

        :return: True if the numeric function is fluent, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_function_skeleton, AdvancedFluentFunctionSkeleton)

    def is_auxiliary(self) -> 'bool':
        """
        Get whether the numeric function is auxiliary.

        :return: True if the numeric function is auxiliary, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_function_skeleton, AdvancedAuxiliaryFunctionSkeleton)

    def get_index(self):
        """
        Get the index of the numeric function.

        :return: The index of the numeric function.
        :rtype: int
        """
        return self._advanced_function_skeleton.get_index()

    def get_name(self):
        """
        Get the name of the numeric function.

        :return: The name of the numeric function.
        :rtype: str
        """
        return self._advanced_function_skeleton.get_name()

    def get_parameters(self) -> 'list[Variable]':
        """
        Get the parameters of the numeric function.

        :return: A list of variables representing the parameters of the numeric function.
        :rtype: list[Variable]
        """
        return [Variable(x) for x in self._advanced_function_skeleton.get_parameters()]

    def __str__(self):
        """
        Get the string representation of the numeric function.

        :return: A string representation of the numeric function.
        :rtype: str
        """
        return f'({self.get_name()} {" ".join(str(x) for x in self.get_parameters())})'

    def __repr__(self):
        """
        Get the string representation of the numeric function.

        :return: A string representation of the numeric function.
        :rtype: str
        """
        return str(self)


class FunctionTerm:
    def __init__(self, advanced_function: 'AdvancedFunction') -> None:
        assert isinstance(advanced_function, AdvancedStaticFunction) \
            or isinstance(advanced_function, AdvancedFluentFunction) \
            or isinstance(advanced_function, AdvancedAuxiliaryFunction), "Invalid advanced function type."
        self._advanced_function = advanced_function

    def get_index(self) -> 'int':
        """
        Get the index of the function term.

        :return: The index of the function term.
        :rtype: int
        """
        return self._advanced_function.get_index()

    def is_static(self) -> 'bool':
        """
        Get whether the function term is static.

        :return: True if the function term is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_function, AdvancedStaticFunction)

    def is_fluent(self) -> 'bool':
        """
        Get whether the function term is fluent.

        :return: True if the function term is fluent, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_function, AdvancedFluentFunction)

    def is_auxiliary(self) -> 'bool':
        """
        Get whether the function term is auxiliary.

        :return: True if the function term is auxiliary, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_function, AdvancedAuxiliaryFunction)

    def get_numeric_function(self) -> 'NumericFunction':
        """
        Get the underlying numeric function.

        :return: The underlying numeric function.
        :rtype: NumericFunction
        """
        return NumericFunction(self._advanced_function.get_function_skeleton())

    def get_terms(self) -> 'list[Union[Object, Variable]]':
        """
        Get the terms of the function term.

        :return: A list of objects and variables representing the terms of the function term.
        :rtype: list[Union[Object, Variable]]
        """
        return [Object(x.get()) if isinstance(x.get(), AdvancedObject) else Variable(x.get()) for x in self._advanced_function.get_terms()]  # type: ignore

    def __str__(self):
        """
        Get the string representation of the function term.

        :return: A string representation of the function term.
        :rtype: str
        """
        return f'({self.get_numeric_function().get_name()} {" ".join(str(x) for x in self.get_terms())})'

    def __repr__(self):
        """
        Get the string representation of the function term.

        :return: A string representation of the function term.
        :rtype: str
        """
        return str(self)


class GroundFunctionTerm:
    def __init__(self, advanced_ground_function: 'AdvancedGroundFunction') -> None:
        assert isinstance(advanced_ground_function, AdvancedStaticGroundFunction) \
            or isinstance(advanced_ground_function, AdvancedFluentGroundFunction) \
            or isinstance(advanced_ground_function, AdvancedAuxiliaryGroundFunction), "Invalid advanced ground function type."
        self._advanced_ground_function = advanced_ground_function

    def get_index(self) -> 'int':
        """
        Get the index of the ground function term.

        :return: The index of the ground function term.
        :rtype: int
        """
        return self._advanced_ground_function.get_index()

    def is_static(self) -> 'bool':
        """
        Get whether the ground function term is static.

        :return: True if the ground function term is static, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_ground_function, AdvancedStaticGroundFunction)

    def is_fluent(self) -> 'bool':
        """
        Get whether the ground function term is fluent.

        :return: True if the ground function term is fluent, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_ground_function, AdvancedFluentGroundFunction)

    def is_auxiliary(self) -> 'bool':
        """
        Get whether the ground function term is auxiliary.

        :return: True if the ground function term is auxiliary, False otherwise.
        :rtype: bool
        """
        return isinstance(self._advanced_ground_function, AdvancedAuxiliaryGroundFunction)

    def get_numeric_function(self) -> 'NumericFunction':
        """
        Get the underlying numeric function.

        :return: The underlying numeric function.
        :rtype: NumericFunction
        """
        return NumericFunction(self._advanced_ground_function.get_function_skeleton())

    def get_terms(self) -> 'list[Object]':
        """
        Get the terms of the ground function term.

        :return: A list of objects representing the terms of the ground function term.
        :rtype: list[Object]
        """
        return [Object(x) for x in self._advanced_ground_function.get_objects()]

    def __str__(self):
        """
        Get the string representation of the ground function term.

        :return: A string representation of the ground function term.
        :rtype: str
        """
        return f'({self.get_numeric_function().get_name()} {" ".join(str(x) for x in self.get_terms())})'

    def __repr__(self):
        """
        Get the string representation of the ground function term.

        :return: A string representation of the ground function term.
        :rtype: str
        """
        return str(self)


class FunctionExpression:
    def __init__(self, advanced_function_expression_base: 'AdvancedFunctionExpressionBase') -> None:
        assert isinstance(advanced_function_expression_base, AdvancedFunctionExpressionBase), "Invalid advanced function expression type."
        self._advanced_function_expression: AdvancedFunctionExpression = advanced_function_expression_base.get()  # type: ignore
        self._operator_names = {
            'PLUS': '+',
            'MINUS': '-',
            'MUL': '*',
            'DIV': '/'
        }

    def get_index(self) -> 'int':
        return self._advanced_function_expression.get_index()

    def is_term(self) -> 'bool':
        return isinstance(self._advanced_function_expression, AdvancedFunctionExpressionNumber) \
               or isinstance(self._advanced_function_expression, AdvancedStaticFunctionExpressionFunction) \
               or isinstance(self._advanced_function_expression, AdvancedFluentFunctionExpressionFunction) \
               or isinstance(self._advanced_function_expression, AdvancedAuxiliaryFunctionExpressionFunction)

    def is_number_term(self) -> 'bool':
        return isinstance(self._advanced_function_expression, AdvancedFunctionExpressionNumber)

    def get_number_term(self) -> 'float':
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionNumber):
            return self._advanced_function_expression.get_number()
        raise RuntimeError("Number requested from non-number expression.")

    def is_function_term(self) -> 'bool':
        return isinstance(self._advanced_function_expression, AdvancedStaticFunctionExpressionFunction) \
               or isinstance(self._advanced_function_expression, AdvancedFluentFunctionExpressionFunction) \
               or isinstance(self._advanced_function_expression, AdvancedAuxiliaryFunctionExpressionFunction)

    def get_function_term(self) -> 'FunctionTerm':
        if isinstance(self._advanced_function_expression, AdvancedStaticFunctionExpressionFunction) \
           or isinstance(self._advanced_function_expression, AdvancedFluentFunctionExpressionFunction) \
           or isinstance(self._advanced_function_expression, AdvancedAuxiliaryFunctionExpressionFunction):
            return FunctionTerm(self._advanced_function_expression.get_function())
        raise RuntimeError("Function term requested from non-function expression.")

    def is_compound(self) -> 'bool':
        return isinstance(self._advanced_function_expression, AdvancedFunctionExpressionBinaryOperator) \
               or isinstance(self._advanced_function_expression, AdvancedFunctionExpressionMultiOperator) \
               or isinstance(self._advanced_function_expression, AdvancedFunctionExpressionMinus)

    def get_operator(self) -> 'str':
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionMinus):
            return self._operator_names['SUBTRACT']
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionBinaryOperator):
            operator = self._advanced_function_expression.get_binary_operator()
            return self._operator_names[operator.name]
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionMultiOperator):
            operator = self._advanced_function_expression.get_multi_operator()
            return self._operator_names[operator.name]
        raise RuntimeError("Operand requested from non-compound expression.")

    def get_num_operands(self) -> 'int':
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionMinus):
            return 1
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionBinaryOperator):
            return 2
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionMultiOperator):
            return len(self._advanced_function_expression.get_function_expressions())
        raise RuntimeError("Number of operands requested from non-compound expression.")

    def get_operands(self) -> 'list[FunctionExpression]':
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionMinus):
            advanced_function_expressions = self._advanced_function_expression.get_function_expression()
            return [FunctionExpression(advanced_function_expressions)]
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionBinaryOperator):
            advanced_left_function_expressions = self._advanced_function_expression.get_left_function_expression()
            advanced_right_function_expressions = self._advanced_function_expression.get_right_function_expression()
            return [FunctionExpression(advanced_left_function_expressions), FunctionExpression(advanced_right_function_expressions)]
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionMultiOperator):
            advanced_function_expressions = self._advanced_function_expression.get_function_expressions()
            return [FunctionExpression(x) for x in advanced_function_expressions]
        raise RuntimeError("Operands requested from non-compound expression.")

    def get_nth_operand(self, n: int) -> 'FunctionExpression':
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionMinus):
            if n == 0:
                advanced_function_expressions = self._advanced_function_expression.get_function_expression()
                return FunctionExpression(advanced_function_expressions)
            raise RuntimeError("Index out of bounds for unary operator expression.")
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionBinaryOperator):
            if n == 0:
                advanced_function_expressions = self._advanced_function_expression.get_left_function_expression()
                return FunctionExpression(advanced_function_expressions)
            if n == 1:
                advanced_function_expressions = self._advanced_function_expression.get_right_function_expression()
                return FunctionExpression(advanced_function_expressions)
            raise RuntimeError("Index out of bounds for binary operator expression.")
        if isinstance(self._advanced_function_expression, AdvancedFunctionExpressionMultiOperator):
            advanced_function_expressions = self._advanced_function_expression.get_function_expressions()
            if n < len(advanced_function_expressions):
                return FunctionExpression(advanced_function_expressions[n])
            raise RuntimeError("Index out of bounds for multi-operator expression.")
        raise RuntimeError("Nth expression requested from non-multi-operator expression.")

    def __str__(self) -> 'str':
        """
        Get a string representation of the expression.

        :return: A string representation of the expression.
        :rtype: str
        """
        if self.is_number_term():
            return f'{self.get_number_term():.2f}'
        elif self.is_function_term():
            outer_term = self.get_function_term()
            outer_symbol = outer_term.get_numeric_function()
            inner_terms = outer_term.get_terms()
            return f'({outer_symbol.get_name()} {" ".join(x.get_name() for x in inner_terms)})'
        elif self.is_compound():
            operands = self.get_operands()
            return f'({self.get_operator()} {" ".join(str(expr) for expr in operands)})'
        else:
            raise RuntimeError(f'Internal error. Please report this. Missing branch for type: {type(self._advanced_function_expression)}')

    def __repr__(self) -> 'str':
        """
        Get a string representation of the expression.

        :return: A string representation of the expression.
        :rtype: str
        """
        return str(self)


class GroundFunctionExpression:
    def __init__(self, advanced_ground_function_expression_base: 'AdvancedGroundFunctionExpressionBase') -> None:
        assert isinstance(advanced_ground_function_expression_base, AdvancedGroundFunctionExpressionBase), "Invalid advanced ground function expression type."
        self._advanced_ground_function_expression: AdvancedGroundFunctionExpression = advanced_ground_function_expression_base.get()  # type: ignore
        self._operator_names = {
            'PLUS': '+',
            'MINUS': '-',
            'MUL': '*',
            'DIV': '/'
        }

    def get_index(self) -> 'int':
        return self._advanced_ground_function_expression.get_index()

    def is_term(self) -> 'bool':
        return isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionNumber) \
               or isinstance(self._advanced_ground_function_expression, AdvancedStaticGroundFunctionExpressionFunction) \
               or isinstance(self._advanced_ground_function_expression, AdvancedFluentGroundFunctionExpressionFunction) \
               or isinstance(self._advanced_ground_function_expression, AdvancedAuxiliaryGroundFunctionExpressionFunction)

    def is_number_term(self) -> 'bool':
        return isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionNumber)

    def get_number_term(self) -> 'float':
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionNumber):
            return self._advanced_ground_function_expression.get_number()
        raise RuntimeError("Number requested from non-number expression.")

    def is_function_term(self) -> 'bool':
        return isinstance(self._advanced_ground_function_expression, AdvancedStaticGroundFunctionExpressionFunction) \
               or isinstance(self._advanced_ground_function_expression, AdvancedFluentGroundFunctionExpressionFunction) \
               or isinstance(self._advanced_ground_function_expression, AdvancedAuxiliaryGroundFunctionExpressionFunction)

    def get_function_term(self) -> 'GroundFunctionTerm':
        if isinstance(self._advanced_ground_function_expression, AdvancedStaticGroundFunctionExpressionFunction) \
           or isinstance(self._advanced_ground_function_expression, AdvancedFluentGroundFunctionExpressionFunction) \
           or isinstance(self._advanced_ground_function_expression, AdvancedAuxiliaryGroundFunctionExpressionFunction):
            return GroundFunctionTerm(self._advanced_ground_function_expression.get_function())
        raise RuntimeError("Function term requested from non-function expression.")

    def is_compound(self) -> 'bool':
        return isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionBinaryOperator) \
               or isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionMultiOperator) \
               or isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionMinus)

    def get_operator(self) -> 'str':
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionMinus):
            return self._operator_names['SUBTRACT']
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionBinaryOperator):
            operator = self._advanced_ground_function_expression.get_binary_operator()
            return self._operator_names[operator.name]
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionMultiOperator):
            operator = self._advanced_ground_function_expression.get_multi_operator()
            return self._operator_names[operator.name]
        raise RuntimeError("Operand requested from non-compound expression.")

    def get_num_operands(self) -> 'int':
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionMinus):
            return 1
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionBinaryOperator):
            return 2
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionMultiOperator):
            return len(self._advanced_ground_function_expression.get_function_expressions())
        raise RuntimeError("Number of operands requested from non-compound expression.")

    def get_operands(self) -> 'list[GroundFunctionExpression]':
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionMinus):
            advanced_function_expressions = self._advanced_ground_function_expression.get_function_expression()
            return [GroundFunctionExpression(advanced_function_expressions)]
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionBinaryOperator):
            advanced_left_function_expressions = self._advanced_ground_function_expression.get_left_function_expression()
            advanced_right_function_expressions = self._advanced_ground_function_expression.get_right_function_expression()
            return [GroundFunctionExpression(advanced_left_function_expressions), GroundFunctionExpression(advanced_right_function_expressions)]
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionMultiOperator):
            advanced_function_expressions = self._advanced_ground_function_expression.get_function_expressions()
            return [GroundFunctionExpression(x) for x in advanced_function_expressions]
        raise RuntimeError("Operands requested from non-compound expression.")

    def get_nth_operand(self, n: int) -> 'GroundFunctionExpression':
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionMinus):
            if n == 0:
                advanced_function_expressions = self._advanced_ground_function_expression.get_function_expression()
                return GroundFunctionExpression(advanced_function_expressions)
            raise RuntimeError("Index out of bounds for unary operator expression.")
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionBinaryOperator):
            if n == 0:
                advanced_function_expressions = self._advanced_ground_function_expression.get_left_function_expression()
                return GroundFunctionExpression(advanced_function_expressions)
            if n == 1:
                advanced_function_expressions = self._advanced_ground_function_expression.get_right_function_expression()
                return GroundFunctionExpression(advanced_function_expressions)
            raise RuntimeError("Index out of bounds for binary operator expression.")
        if isinstance(self._advanced_ground_function_expression, AdvancedGroundFunctionExpressionMultiOperator):
            advanced_function_expressions = self._advanced_ground_function_expression.get_function_expressions()
            if n < len(advanced_function_expressions):
                return GroundFunctionExpression(advanced_function_expressions[n])
            raise RuntimeError("Index out of bounds for multi-operator expression.")
        raise RuntimeError("Nth expression requested from non-multi-operator expression.")

    def __str__(self) -> 'str':
        """
        Get a string representation of the expression.

        :return: A string representation of the expression.
        :rtype: str
        """
        if self.is_number_term():
            return f'{self.get_number_term():.2f}'
        elif self.is_function_term():
            outer_term = self.get_function_term()
            outer_symbol = outer_term.get_numeric_function()
            inner_terms = outer_term.get_terms()
            return f'({outer_symbol.get_name()} {" ".join(x.get_name() for x in inner_terms)})'
        elif self.is_compound():
            operands = self.get_operands()
            return f'({self.get_operator()} {" ".join(str(expr) for expr in operands)})'
        else:
            raise RuntimeError(f'Internal error. Please report this. Missing branch for type: {type(self._advanced_ground_function_expression)}')

    def __repr__(self) -> 'str':
        """
        Get a string representation of the expression.

        :return: A string representation of the expression.
        :rtype: str
        """
        return str(self)


class NumericCondition:
    def __init__(self, advanced_numeric_constraint: AdvancedNumericConstraint) -> None:
        assert isinstance(advanced_numeric_constraint, AdvancedNumericConstraint), "Invalid ground numeric constraint type."
        self._advanced_numeric_constraint = advanced_numeric_constraint
        self._comparator_names = {
            'EQUAL': '==',
            'GREATER': '>',
            'GREATER_EQUAL': '>=',
            'LESS': '<',
            'LESS_EQUAL': '<='
        }

    def get_index(self) -> 'int':
        return self._advanced_numeric_constraint.get_index()

    def get_comparator(self) -> 'str':
        advanced_name = self._advanced_numeric_constraint.get_binary_comparator().name
        assert advanced_name in self._comparator_names
        return self._comparator_names[advanced_name]

    def get_left_expression(self) -> 'FunctionExpression':
        advanced_left_expression = self._advanced_numeric_constraint.get_left_function_expression()
        return FunctionExpression(advanced_left_expression)

    def get_right_expression(self) -> 'FunctionExpression':
        advanced_right_expression = self._advanced_numeric_constraint.get_right_function_expression()
        return FunctionExpression(advanced_right_expression)

    def __str__(self) -> 'str':
        """
        Get a string representation of the condition.

        :return: A string representation of the condition.
        :rtype: str
        """
        return f'({self.get_comparator()} {str(self.get_left_expression())} {str(self.get_right_expression())})'

    def __repr__(self) -> 'str':
        """
        Get a string representation of the condition.

        :return: A string representation of the condition.
        :rtype: str
        """
        return str(self)


class GroundNumericCondition:
    def __init__(self, advanced_ground_numeric_constraint: AdvancedGroundNumericConstraint) -> None:
        assert isinstance(advanced_ground_numeric_constraint, AdvancedGroundNumericConstraint), "Invalid ground numeric constraint type."
        self._advanced_ground_numeric_constraint = advanced_ground_numeric_constraint
        self._comparator_names = {
            'EQUAL': '==',
            'GREATER': '>',
            'GREATER_EQUAL': '>=',
            'LESS': '<',
            'LESS_EQUAL': '<='
        }

    def get_index(self) -> 'int':
        return self._advanced_ground_numeric_constraint.get_index()

    def get_comparator(self) -> 'str':
        advanced_name = self._advanced_ground_numeric_constraint.get_binary_comparator().name
        assert advanced_name in self._comparator_names
        return self._comparator_names[advanced_name]

    def get_left_expression(self) -> 'GroundFunctionExpression':
        advanced_left_expression = self._advanced_ground_numeric_constraint.get_left_function_expression()
        return GroundFunctionExpression(advanced_left_expression)

    def get_right_expression(self) -> 'GroundFunctionExpression':
        advanced_right_expression = self._advanced_ground_numeric_constraint.get_right_function_expression()
        return GroundFunctionExpression(advanced_right_expression)

    def __str__(self) -> 'str':
        """
        Get a string representation of the condition.

        :return: A string representation of the condition.
        :rtype: str
        """
        return f'({self.get_comparator()} {str(self.get_left_expression())} {str(self.get_right_expression())})'

    def __repr__(self) -> 'str':
        """
        Get a string representation of the condition.

        :return: A string representation of the condition.
        :rtype: str
        """
        return str(self)


# TODO: increase, decrease, scale-up, scale-down
