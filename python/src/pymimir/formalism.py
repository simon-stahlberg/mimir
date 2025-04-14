from functools import lru_cache
from pathlib import Path
from typing import TypeAlias, Union

from pymimir.advanced.formalism import Action as AdvancedAction
from pymimir.advanced.formalism import ConditionalEffect as AdvancedConditionalEffect
from pymimir.advanced.formalism import ConjunctiveCondition as AdvancedConjunctiveCondition
from pymimir.advanced.formalism import ConjunctiveEffect as AdvancedConjunctiveEffect
from pymimir.advanced.formalism import DerivedAtom as AdvancedDerivedAtom
from pymimir.advanced.formalism import DerivedGroundAtom as AdvancedDerivedGroundAtom
from pymimir.advanced.formalism import DerivedGroundLiteral as AdvancedDerivedGroundLiteral
from pymimir.advanced.formalism import DerivedLiteral as AdvancedDerivedLiteral
from pymimir.advanced.formalism import DerivedLiteralList as AdvancedDerivedLiteralList
from pymimir.advanced.formalism import DerivedPredicate as AdvancedDerivedPredicate
from pymimir.advanced.formalism import Domain as AdvancedDomain
from pymimir.advanced.formalism import FluentAtom as AdvancedFluentAtom
from pymimir.advanced.formalism import FluentGroundAtom as AdvancedFluentGroundAtom
from pymimir.advanced.formalism import FluentGroundLiteral as AdvancedFluentGroundLiteral
from pymimir.advanced.formalism import FluentLiteral as AdvancedFluentLiteral
from pymimir.advanced.formalism import FluentLiteralList as AdvancedFluentLiteralList
from pymimir.advanced.formalism import FluentPredicate as AdvancedFluentPredicate
from pymimir.advanced.formalism import NumericConstraintList as AdvancedNumericConstraintList
from pymimir.advanced.formalism import Object as AdvancedObject
from pymimir.advanced.formalism import Parser as AdvancedParser
from pymimir.advanced.formalism import Problem as AdvancedProblem
from pymimir.advanced.formalism import Repositories as AdvancedRepositories
from pymimir.advanced.formalism import StaticAtom as AdvancedStaticAtom
from pymimir.advanced.formalism import StaticGroundAtom as AdvancedStaticGroundAtom
from pymimir.advanced.formalism import StaticGroundLiteral as AdvancedStaticGroundLiteral
from pymimir.advanced.formalism import StaticLiteral as AdvancedStaticLiteral
from pymimir.advanced.formalism import StaticLiteralList as AdvancedStaticLiteralList
from pymimir.advanced.formalism import StaticPredicate as AdvancedStaticPredicate
from pymimir.advanced.formalism import Term as AdvancedTerm
from pymimir.advanced.formalism import TermList as AdvancedTermList
from pymimir.advanced.formalism import Variable as AdvancedVariable
from pymimir.advanced.formalism import VariableList as AdvancedVariableList
from pymimir.advanced.search import GroundedAxiomEvaluator as AdvancedGroundedAxiomEvaluator
from pymimir.advanced.search import LiftedAxiomEvaluator as AdvancedLiftedAxiomEvaluator
from pymimir.advanced.search import SearchMode, SearchContext, SearchContextOptions
# from pymimir.advanced.formalism import AuxiliaryGroundNumericEffect as AdvancedAuxiliaryGroundNumericEffect
# from pymimir.advanced.formalism import AuxiliaryNumericEffect as AdvancedAuxiliaryNumericEffect
# from pymimir.advanced.formalism import FluentGroundNumericEffect as AdvancedFluentGroundNumericEffect
# from pymimir.advanced.formalism import FluentNumericEffect as AdvancedFluentNumericEffect
# from pymimir.advanced.formalism import FunctionExpression as AdvancedFunctionExpression
# from pymimir.advanced.formalism import NumericConstraint as AdvancedNumericConstraint


AdvancedAtom: TypeAlias = Union[AdvancedStaticAtom, AdvancedFluentAtom, AdvancedDerivedAtom]
AdvancedGroundAtom: TypeAlias = Union[AdvancedStaticGroundAtom, AdvancedFluentGroundAtom, AdvancedDerivedGroundAtom]
AdvancedLiteral: TypeAlias = Union[AdvancedStaticLiteral, AdvancedFluentLiteral, AdvancedDerivedLiteral]
AdvancedGroundLiteral: TypeAlias = Union[AdvancedStaticGroundLiteral, AdvancedFluentGroundLiteral, AdvancedDerivedGroundLiteral]
AdvancedPredicate: TypeAlias = Union[AdvancedStaticPredicate, AdvancedFluentPredicate, AdvancedDerivedPredicate]
AdvancedAxiomEvaluator: TypeAlias = Union[AdvancedGroundedAxiomEvaluator, AdvancedLiftedAxiomEvaluator]
# AdvancedNumericEffect: TypeAlias = Union[AdvancedFluentNumericEffect, AdvancedAuxiliaryNumericEffect]
# AdvancedGroundNumericEffect: TypeAlias = Union[AdvancedFluentGroundNumericEffect, AdvancedAuxiliaryGroundNumericEffect]


class Variable:
    _advanced_variable: AdvancedVariable = None

    def __init__(self, advanced_variable: AdvancedVariable) -> None:
        self._advanced_variable = advanced_variable

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the variable."""
        return self._advanced_variable.get_index()

    @lru_cache(maxsize=None)
    def get_name(self) -> str:
        """Get the name of the variable."""
        return self._advanced_variable.get_name()

    @lru_cache(maxsize=None)
    def __str__(self) -> str:
        """Get the string representation of the variable."""
        return str(self._advanced_variable)

    @lru_cache(maxsize=None)
    def __repr__(self) -> str:
        """Get the string representation of the variable."""
        return repr(self._advanced_variable)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the variable."""
        return hash(self._advanced_variable)

    def __eq__(self, other: object) -> bool:
        """Check if two variables are equal."""
        if not isinstance(other, Variable):
            return False
        return self._advanced_variable == other._advanced_variable


class Object:
    """Object class for the PDDL domain."""
    _advanced_object: AdvancedObject = None

    def __init__(self, advanced_object: AdvancedObject) -> None:
        self._advanced_object = advanced_object

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the object."""
        return self._advanced_object.get_index()

    @lru_cache(maxsize=None)
    def get_name(self) -> str:
        """Get the name of the object."""
        return self._advanced_object.get_name()

    @lru_cache(maxsize=None)
    def __str__(self) -> str:
        """Get the string representation of the object."""
        return str(self._advanced_object)

    @lru_cache(maxsize=None)
    def __repr__(self) -> str:
        """Get the string representation of the object."""
        return repr(self._advanced_object)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the object."""
        return hash(self._advanced_object)

    def __eq__(self, other: object) -> bool:
        """Check if two objects are equal."""
        if not isinstance(other, Object):
            return False
        return self._advanced_object == other._advanced_object


Term: TypeAlias = Union[Object, Variable]


class Predicate:
    """Predicate class for the PDDL domain."""
    _advanced_predicate: AdvancedPredicate = None

    def __init__(self, advanced_predicate: AdvancedPredicate) -> None:
        self._advanced_predicate = advanced_predicate

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the predicate."""
        return self._advanced_predicate.get_index()

    @lru_cache(maxsize=None)
    def get_name(self) -> str:
        """Get the name of the predicate."""
        return self._advanced_predicate.get_name()

    @lru_cache(maxsize=None)
    def get_arity(self) -> int:
        """Get the arity of the predicate."""
        return self._advanced_predicate.get_arity()

    @lru_cache(maxsize=None)
    def get_parameters(self) -> list[Variable]:
        """Get the parameters of the predicate."""
        return [Variable(x) for x in self._advanced_predicate.get_parameters()]

    @lru_cache(maxsize=None)
    def __str__(self):
        """Get the string representation of the predicate."""
        return str(self._advanced_predicate)

    @lru_cache(maxsize=None)
    def __repr__(self):
        """Get the string representation of the predicate."""
        return repr(self._advanced_predicate)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the predicate."""
        return hash(self._advanced_predicate)

    def __eq__(self, other: object) -> bool:
        """Check if two predicates are equal."""
        if not isinstance(other, Predicate):
            return False
        return self._advanced_predicate == other._advanced_predicate


class GroundAtom:
    """GroundAtom class for the PDDL domain."""
    _advanced_ground_atom: AdvancedGroundAtom = None

    def __init__(self, advanced_ground_atom: AdvancedGroundAtom) -> None:
        self._advanced_ground_atom = advanced_ground_atom

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the ground atom."""
        return self._advanced_ground_atom.get_index()

    @lru_cache(maxsize=None)
    def get_predicate(self) -> Predicate:
        """Get the predicate of the ground atom."""
        return Predicate(self._advanced_ground_atom.get_predicate())

    @lru_cache(maxsize=None)
    def get_terms(self) -> list[Object]:
        """Get the objects of the ground atom."""
        return [Object(x) for x in self._advanced_ground_atom.get_objects()]

    @lru_cache(maxsize=None)
    def get_arity(self) -> int:
        """Get the arity of the ground atom."""
        return self._advanced_ground_atom.get_arity()

    @lru_cache(maxsize=None)
    def __str__(self):
        """Get the string representation of the ground atom."""
        return str(self._advanced_ground_atom)

    @lru_cache(maxsize=None)
    def __repr__(self):
        """Get the string representation of the ground atom."""
        return repr(self._advanced_ground_atom)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the ground atom."""
        return hash(self._advanced_ground_atom)

    def __eq__(self, other: object) -> bool:
        """Check if two ground atoms are equal."""
        if not isinstance(other, GroundAtom):
            return False
        return self._advanced_ground_atom == other._advanced_ground_atom


class Atom:
    """Atom class for the PDDL domain."""
    _advanced_atom: AdvancedAtom = None

    def __init__(self, advanced_atom: AdvancedAtom) -> None:
        self._advanced_atom = advanced_atom

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the atom."""
        return self._advanced_atom.get_index()

    @lru_cache(maxsize=None)
    def get_predicate(self) -> Predicate:
        """Get the predicate of the atom."""
        return Predicate(self._advanced_atom.get_predicate())

    @lru_cache(maxsize=None)
    def get_terms(self) -> list[Union[Object, Variable]]:
        """Get the terms of the atom."""
        return [Object(x) if isinstance(x, AdvancedObject) else Variable(x) for x in self._advanced_atom.get_terms()]

    @lru_cache(maxsize=None)
    def get_variables(self) -> list[Variable]:
        """Get the variables of the atom."""
        return [Variable(x) for x in self._advanced_atom.get_variables()]

    @lru_cache(maxsize=None)
    def get_arity(self) -> int:
        """Get the arity of the atom."""
        return len(self.get_terms())

    @lru_cache(maxsize=None)
    def __str__(self):
        """Get the string representation of the atom."""
        return str(self._advanced_atom)

    @lru_cache(maxsize=None)
    def __repr__(self):
        """Get the string representation of the atom."""
        return repr(self._advanced_atom)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the atom."""
        return hash(self._advanced_atom)

    def __eq__(self, other: object) -> bool:
        """Check if two atoms are equal."""
        if not isinstance(other, Atom):
            return False
        return self._advanced_atom == other._advanced_atom


class GroundLiteral:
    """GroundLiteral class for the PDDL domain."""
    _advanced_ground_literal: AdvancedGroundLiteral = None

    def __init__(self, advanced_ground_literal: AdvancedGroundLiteral) -> None:
        self._advanced_ground_literal = advanced_ground_literal

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the ground literal."""
        return self._advanced_ground_literal.get_index()

    @lru_cache(maxsize=None)
    def get_atom(self) -> GroundAtom:
        """Get the atom of the ground literal."""
        return GroundAtom(self._advanced_ground_literal.get_atom())

    @lru_cache(maxsize=None)
    def get_polarity(self) -> bool:
        """Get the polarity of the ground literal."""
        return self._advanced_ground_literal.get_polarity()

    @lru_cache(maxsize=None)
    def __str__(self):
        """Get the string representation of the ground literal."""
        return str(self._advanced_ground_literal)

    @lru_cache(maxsize=None)
    def __repr__(self):
        """Get the string representation of the ground literal."""
        return repr(self._advanced_ground_literal)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the ground literal."""
        return hash(self._advanced_ground_literal)

    def __eq__(self, other: object) -> bool:
        """Check if two ground literals are equal."""
        if not isinstance(other, GroundLiteral):
            return False
        return self._advanced_ground_literal == other._advanced_ground_literal


class Literal:
    """Literal class for the PDDL domain."""
    _advanced_literal = None

    def __init__(self, advanced_literal: AdvancedLiteral) -> None:
            self._advanced_literal = advanced_literal

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the literal."""
        return self._advanced_literal.get_index()

    @lru_cache(maxsize=None)
    def get_atom(self) -> Atom:
        """Get the atom of the literal."""
        return Atom(self._advanced_literal.get_atom())

    @lru_cache(maxsize=None)
    def get_polarity(self) -> bool:
        """Get the polarity of the literal."""
        return self._advanced_literal.get_polarity()

    @lru_cache(maxsize=None)
    def __str__(self) -> str:
        """Get the string representation of the literal."""
        return str(self._advanced_literal)

    @lru_cache(maxsize=None)
    def __repr__(self) -> str:
        """Get the string representation of the literal."""
        return repr(self._advanced_literal)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the literal."""
        return hash(self._advanced_literal)

    def __eq__(self, other: object) -> bool:
        """Check if two literals are equal."""
        if not isinstance(other, Literal):
            return False
        return self._advanced_literal == other._advanced_literal


def _split_literal_list(literals: list[Literal]) -> tuple[AdvancedStaticLiteralList, AdvancedFluentLiteralList, AdvancedDerivedLiteralList]:
    """Split the literal list into static, fluent, and derived literals."""
    advanced_static_literals = AdvancedStaticLiteralList([x._advanced_literal for x in literals if isinstance(x, AdvancedStaticLiteral)])
    advanced_fluent_literals = AdvancedFluentLiteralList([x._advanced_literal for x in literals if isinstance(x, AdvancedFluentLiteral)])
    advanced_derived_literals = AdvancedDerivedLiteralList([x._advanced_literal for x in literals if isinstance(x, AdvancedDerivedLiteral)])
    return advanced_static_literals, advanced_fluent_literals, advanced_derived_literals
    #

# class FunctionExpression:
#     """FunctionExpression class for the PDDL domain."""
#     _advanced_function_expression: AdvancedFunctionExpression = None

#     def __init__(self, advanced_function_expression: AdvancedFunctionExpression) -> None:
#         self._advanced_function_expression = advanced_function_expression
#         raise NotImplementedError("Not implemented.")


# class NumericConstraint:
#     """NumericConstraint class for the PDDL domain."""
#     _advanced_numeric_constraint: AdvancedNumericConstraint = None

#     def __init__(self, advanced_numeric_constraint: AdvancedNumericConstraint) -> None:
#         self._advanced_numeric_constraint = advanced_numeric_constraint

#     def get_index(self) -> int:
#         """Get the index of the numeric constraint."""
#         return self._advanced_numeric_constraint.get_index()

#     def get_binary_comparator(self):
#         """Get the binary comparator of the numeric constraint."""
#         raise NotImplementedError("Not implemented.")
#         return self._advanced_numeric_constraint.get_binary_comparator()

#     def get_left_function_expression(self) -> FunctionExpression:
#         """Get the left function expression of the numeric constraint."""
#         return self._advanced_numeric_constraint.get_left_function_expression()

#     def get_right_function_expression(self) -> FunctionExpression:
#         """Get the right function expression of the numeric constraint."""
#         return self._advanced_numeric_constraint.get_right_function_expression()


class ConjunctiveCondition:
    _advanced_conjunctive_condition: AdvancedConjunctiveCondition = None

    def __init__(self, advanced_condition: AdvancedConjunctiveCondition) -> None:
        self._advanced_conjunctive_condition = advanced_condition

    @lru_cache(maxsize=None)
    def get_parameters(self) -> list[Variable]:
        """Get the parameters of the conjunctive condition."""
        return [Variable(x) for x in self._advanced_conjunctive_condition.get_parameters()]

    @lru_cache(maxsize=None)
    def get_literals(self) -> list[Literal]:
        """Get the literals of the conjunctive condition."""
        literals = []
        literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_static_literals()])
        literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_fluent_literals()])
        literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_derived_literals()])
        return literals

    @lru_cache(maxsize=None)
    def get_nullary_ground_literals(self) -> list[GroundLiteral]:
        """Get the nullary ground literals of the conjunctive condition."""
        ground_literals = []
        ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_static_literals()])
        ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_fluent_literals()])
        ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_derived_literals()])
        return ground_literals

    @lru_cache(maxsize=None)
    def __str__(self):
        """Get the string representation of the conjunctive condition."""
        return str(self._advanced_conjunctive_condition)

    @lru_cache(maxsize=None)
    def __repr__(self):
        """Get the string representation of the conjunctive condition."""
        return repr(self._advanced_conjunctive_condition)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the conjunctive condition."""
        return hash(self._advanced_conjunctive_condition)

    def __eq__(self, other: object) -> bool:
        """Check if two conjunctive conditions are equal."""
        if not isinstance(other, ConjunctiveCondition):
            return False
        return self._advanced_conjunctive_condition == other._advanced_conjunctive_condition

    # def get_numeric_constraints(self) -> list[NumericConstraint]:
    #     """Get the numeric constraints of the conjunctive condition."""
    #     return list(self._advanced_conjunctive_condition.get_numeric_constraints())


# class NumericEffect:
#     """NumericEffect class for the PDDL domain."""
#     _advanced_numeric_effect: AdvancedNumericEffect = None

#     def __init__(self, advanced_numeric_effect: AdvancedNumericEffect) -> None:
#         self._advanced_numeric_effect = advanced_numeric_effect

#     def get_index(self) -> int:
#         """Get the index of the numeric effect."""
#         return self._advanced_numeric_effect.get_index()

#     def get_function_expression(self) -> FunctionExpression:
#         """Get the function expression of the numeric effect."""
#         return self._advanced_numeric_effect.get_function_expression()

#     def get_function(self) -> FunctionExpression:
#         """Get the function of the numeric effect."""
#         raise NotImplementedError("Not implemented.")


class ConjunctiveEffect:
    """ConjunctiveEffect class for the PDDL domain."""
    _advanced_conjunctive_effect: AdvancedConjunctiveEffect = None

    def __init__(self, advanced_effect: AdvancedConjunctiveEffect) -> None:
        self._advanced_conjunctive_effect = advanced_effect

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the conjunctive effect."""
        return self._advanced_conjunctive_effect.get_index()

    @lru_cache(maxsize=None)
    def get_parameters(self) -> list[Variable]:
        """Get the parameters of the conjunctive effect."""
        return [Variable(x) for x in self._advanced_conjunctive_effect.get_parameters()]

    @lru_cache(maxsize=None)
    def get_literals(self) -> list[Literal]:
        """Get the literals of the conjunctive effect."""
        return [Literal(x) for x in self._advanced_conjunctive_effect.get_literals()]

    # def get_numeric_effects(self) -> list[NumericEffect]:
    #     """Get the fluent numeric effects of the conjunctive effect."""
    #     numeric_effects = list(self._advanced_conjunctive_effect.get_fluent_numeric_effects())
    #     auxiliary_numeric_effect = self._advanced_conjunctive_effect.get_auxiliary_numeric_effect()
    #     if auxiliary_numeric_effect: numeric_effects.append(auxiliary_numeric_effect)
    #     return numeric_effects

    @lru_cache(maxsize=None)
    def __str__(self):
        """Get the string representation of the conjunctive effect."""
        return str(self._advanced_conjunctive_effect)

    @lru_cache(maxsize=None)
    def __repr__(self):
        """Get the string representation of the conjunctive effect."""
        return repr(self._advanced_conjunctive_effect)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the conjunctive effect."""
        return hash(self._advanced_conjunctive_effect)

    def __eq__(self, other: object) -> bool:
        """Check if two conjunctive effects are equal."""
        if not isinstance(other, ConjunctiveEffect):
            return False
        return self._advanced_conjunctive_effect == other._advanced_conjunctive_effect


class ConditionalEffect:
    """ConditionalEffect class for the PDDL domain."""
    _advanced_conditional_effect: AdvancedConditionalEffect = None

    def __init__(self, advanced_conditional_effect: AdvancedConditionalEffect) -> None:
        self._advanced_conditional_effect = advanced_conditional_effect

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the conditional effect."""
        return self._advanced_conditional_effect.get_index()

    @lru_cache(maxsize=None)
    def get_parameters(self) -> ConjunctiveCondition:
        """Get the parameters of the conditional effect."""
        return ConjunctiveCondition(self._advanced_conditional_effect.get_parameters())

    @lru_cache(maxsize=None)
    def get_static_conditions(self) -> ConjunctiveEffect:
        """Get the static conditions of the conditional effect."""
        return ConjunctiveEffect(self._advanced_conditional_effect.get_static_conditions())

    @lru_cache(maxsize=None)
    def __str__(self):
        """Get the string representation of the conditional effect."""
        return str(self._advanced_conditional_effect)

    @lru_cache(maxsize=None)
    def __repr__(self):
        """Get the string representation of the conditional effect."""
        return repr(self._advanced_conditional_effect)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the conditional effect."""
        return hash(self._advanced_conditional_effect)

    def __eq__(self, other: object) -> bool:
        """Check if two conditional effects are equal."""
        if not isinstance(other, ConditionalEffect):
            return False
        return self._advanced_conditional_effect == other._advanced_conditional_effect


class Action:
    _advanced_action: AdvancedAction = None

    def __init__(self, advanced_action: AdvancedAction) -> None:
        self._advanced_action = advanced_action

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the action."""
        return self._advanced_action.get_index()

    @lru_cache(maxsize=None)
    def get_arity(self) -> int:
        """Get the arity of the action."""
        return self._advanced_action.get_arity()

    @lru_cache(maxsize=None)
    def get_name(self) -> str:
        """Get the name of the action."""
        return self._advanced_action.get_name()

    @lru_cache(maxsize=None)
    def get_parameters(self) -> list[Variable]:
        """Get the parameters of the action."""
        return [Variable(x) for x in self._advanced_action.get_parameters()]

    @lru_cache(maxsize=None)
    def get_conjunctive_condition(self) -> ConjunctiveCondition:
        """Get the conjunctive condition of the action."""
        return ConjunctiveCondition(self._advanced_action.get_conjunctive_condition())

    @lru_cache(maxsize=None)
    def get_conjunctive_effect(self) -> ConjunctiveEffect:
        """Get the conjunctive effect of the action."""
        return ConjunctiveEffect(self._advanced_action.get_conjunctive_effect())

    @lru_cache(maxsize=None)
    def get_conditional_effect(self) -> list[ConditionalEffect]:
        """Get the conditional effect of the action."""
        return [ConditionalEffect(x) for x in self._advanced_action.get_conditional_effects()]

    @lru_cache(maxsize=None)
    def __str__(self):
        """Get the string representation of the action."""
        return str(self._advanced_action)

    @lru_cache(maxsize=None)
    def __repr__(self):
        """Get the string representation of the action."""
        return repr(self._advanced_action)

    @lru_cache(maxsize=None)
    def __hash__(self) -> int:
        """Get the hash of the action."""
        return hash(self._advanced_action)

    def __eq__(self, other: object) -> bool:
        """Check if two actions are equal."""
        if not isinstance(other, Action):
            return False
        return self._advanced_action == other._advanced_action


class Domain:
    """Domain class for the PDDL domain."""
    _advanced_parser: AdvancedParser = None
    _advanced_domain: AdvancedDomain = None
    _repositories: AdvancedRepositories = None

    def __init__(self, domain_path: Path) -> None:
        """
        Initialize the domain with the given path.

        Args:
            domain_path (Path): The file path to the domain definition file.
        """
        self._advanced_parser = AdvancedParser(str(domain_path))
        self._advanced_domain = self._advanced_parser.get_domain()
        self._repositories = self._advanced_domain.get_repositories()

    @lru_cache(maxsize=None)
    def get_name(self) -> str:
        self._advanced_domain.get_name()

    @lru_cache(maxsize=None)
    def get_actions(self) -> list[Action]:
        """Get the actions of the domain."""
        return [Action(x) for x in self._advanced_domain.get_actions()]

    @lru_cache(maxsize=None)
    def get_constants(self) -> list[Object]:
        """Get the constants of the domain."""
        return [Object(x) for x in self._advanced_domain.get_constants()]

    @lru_cache(maxsize=None)
    def get_predicates(self, skip_static = False, skip_fluent = False, skip_derived = False) -> list[Predicate]:
        """Get the predicates of the domain."""
        predicates = []
        if not skip_static:
            predicates.extend([Predicate(x) for x in self._advanced_domain.get_static_predicates()])
        if not skip_fluent:
            predicates.extend([Predicate(x) for x in self._advanced_domain.get_fluent_predicates()])
        if not skip_derived:
            predicates.extend([Predicate(x) for x in self._advanced_domain.get_derived_predicates()])
        return predicates


class Problem:
    """Problem class for the PDDL problem."""
    _domain: Domain = None
    _search_context: SearchContext = None
    _advanced_problem: AdvancedProblem = None

    def __init__(self, domain: Domain, problem_path: Path, mode: SearchMode = SearchMode.LIFTED) -> None:
        """
        Initialize the problem with the given domain, path, and mode.

        Args:
            domain (Domain): The PDDL domain.
            problem_path (Path): The path to the problem definition file.
            mode (SearchMode): The search mode.
        """
        self._domain = domain
        advanced_problem = domain._advanced_parser.parse_problem(str(problem_path))
        self._search_context = SearchContext.create(advanced_problem, SearchContextOptions(mode))
        self._advanced_problem = self._search_context.get_problem()

    @lru_cache(maxsize=None)
    def get_index(self) -> int:
        """Get the index of the problem."""
        return self._advanced_problem.get_index()

    @lru_cache(maxsize=None)
    def get_name(self) -> str:
        """Get the name of the problem."""
        return self._advanced_problem.get_name()

    @lru_cache(maxsize=None)
    def get_objects(self) -> list[Object]:
        """Get the objects of the problem."""
        return [Object(x) for x in self._advanced_problem.get_objects()]

    @lru_cache(maxsize=None)
    def get_requirements(self) -> list[str]:
        """Get the requirements of the problem."""
        return self._advanced_problem.get_requirements()  # TODO: Dominik -- Crashes

    # TODO: Update skip_derived to False
    @lru_cache(maxsize=None)
    def get_initial_atoms(self, skip_static = True, skip_fluent = False, skip_derived = True) -> list[GroundAtom]:
        """Get the initial atoms of the problem."""
        problem: AdvancedProblem = self._search_context.get_problem()
        initial_atoms = []
        # Add static atoms if skip_static is False
        if not skip_static:
            static_atoms = problem.get_static_initial_atoms()
            initial_atoms.extend([GroundAtom(x) for x in static_atoms])
        # Add fluent atoms
        if not skip_fluent:
            fluent_atoms = problem.get_fluent_initial_atoms()
            initial_atoms.extend([GroundAtom(x) for x in fluent_atoms])
        # Add derived atoms if skip_derived is False
        if not skip_derived:
            initial_state, _ = self._search_context.get_state_repository().get_or_create_initial_state()
            derived_indices = initial_state.get_derived_atoms()
            derived_atoms = problem.get_repositories().get_derived_ground_atoms_from_indices(derived_indices)  # TODO: Dominik -- Crashes
            initial_atoms.extend([GroundAtom(x) for x in derived_atoms])
        return initial_atoms

    def get_static_goal_condition(self) -> ConjunctiveCondition:
        return ConjunctiveCondition(self.get_static_goal_condition())

    # def ground(self) -> Any:
    #     return self.ground() # TODO: Figure out how this ground function work.

    def new_atom(self, predicate: Predicate, terms: list[Term]) -> Atom:
        """Create an atom from the given predicate and terms."""
        advanced_terms = [AdvancedTerm(x._advanced_object) if isinstance(x, AdvancedObject) else AdvancedTerm(x._advanced_variable) for x in terms]
        return Atom(self._advanced_problem.get_or_create_atom(predicate._advanced_predicate, AdvancedTermList(advanced_terms)))

    def new_literal(self, atom: Atom, polarity: bool) -> Literal:
        """Create a literal with the given polarity."""
        return Literal(self._advanced_problem.get_or_create_literal(polarity, atom._advanced_atom))

    def new_ground_atom(self, predicate: Predicate, objects: list[Object]) -> GroundAtom:
        """Create a ground atom from the given predicate and objects."""
        advanced_objects = [x._advanced_object for x in objects]
        return GroundAtom(self._advanced_problem.get_or_create_ground_atom(predicate._advanced_predicate, AdvancedTermList(advanced_objects)))

    # def new_ground_literal(self, ground_atom: GroundAtom, polarity: bool) -> GroundLiteral:
    #     """Create a ground literal with the given polarity."""
    #     return GroundLiteral(polarity, self._advanced_problem.get_or_create_ground_literal(...))  # TODO: Dominik - Missing create?

    def new_variable(self, name: str, parameter_index: int) -> Variable:
        return Variable(self._advanced_problem.get_or_create_variable(name, parameter_index))

    def new_variable_list(self, names: list[str]) -> list[Variable]:
        return [self.new_variable(name, parameter_index) for name, parameter_index in zip(names, range(len(names)))]

    def new_conjunctive_condition(self, variables: list[Variable], literals: list[Literal]) -> ConjunctiveCondition:
        """Create a conjunctive condition from the given parameters, literals, and ground literals."""
        advanced_variables = AdvancedVariableList([x._advanced_variable for x in variables])
        advanced_static_literals, advanced_fluent_literals, advanced_derived_literals = _split_literal_list(literals)
        advanced_numeric_constraints = AdvancedNumericConstraintList()
        return ConjunctiveCondition(self._advanced_problem.get_or_create_conjunctive_condition(advanced_variables,
                                                                                               advanced_static_literals,
                                                                                               advanced_fluent_literals,
                                                                                               advanced_derived_literals,
                                                                                               advanced_numeric_constraints))
        # TODO: Dominik -- Python binding needs to be fixed because of Hana bindings.