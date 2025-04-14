from functools import lru_cache
from pathlib import Path
from typing import TypeAlias, Union

from pymimir.advanced.formalism import Action as AdvancedAction
from pymimir.advanced.formalism import ConditionalEffect as AdvancedConditionalEffect
from pymimir.advanced.formalism import ConjunctiveCondition as AdvancedConjunctiveCondition
from pymimir.advanced.formalism import GroundConjunctiveCondition as AdvancedGroundConjunctiveCondition
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
# from pymimir.advanced.formalism import NumericConstraintList as AdvancedNumericConstraintList  # TODO: Dominik -- Cannot import anymore.
from pymimir.advanced.formalism import Object as AdvancedObject
from pymimir.advanced.formalism import ObjectList as AdvancedObjectList
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
from pymimir.advanced.search import State as AdvancedState
from pymimir.advanced.search import LiftedApplicableActionGenerator as AdvancedLiftedApplicableActionGenerator
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
        assert isinstance(advanced_variable, AdvancedVariable), "Invalid variable type."
        self._advanced_variable = advanced_variable

    def get_index(self) -> 'int':
        """Get the index of the variable."""
        return self._advanced_variable.get_index()

    def get_name(self) -> 'str':
        """Get the name of the variable."""
        return self._advanced_variable.get_name()

    def __str__(self) -> 'str':
        """Get the string representation of the variable."""
        return str(self._advanced_variable)

    def __repr__(self) -> 'str':
        """Get the string representation of the variable."""
        return repr(self._advanced_variable)

    def __hash__(self) -> 'int':
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
        assert isinstance(advanced_object, AdvancedObject), "Invalid object type."
        self._advanced_object = advanced_object

    def get_index(self) -> 'int':
        """Get the index of the object."""
        return self._advanced_object.get_index()

    def get_name(self) -> 'str':
        """Get the name of the object."""
        return self._advanced_object.get_name()

    def __str__(self) -> 'str':
        """Get the string representation of the object."""
        return str(self._advanced_object)

    def __repr__(self) -> 'str':
        """Get the string representation of the object."""
        return repr(self._advanced_object)

    def __hash__(self) -> 'int':
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
        assert isinstance(advanced_predicate, AdvancedPredicate), "Invalid predicate type."
        self._advanced_predicate = advanced_predicate

    def get_index(self) -> 'int':
        """Get the index of the predicate."""
        return self._advanced_predicate.get_index()

    def get_name(self) -> 'str':
        """Get the name of the predicate."""
        return self._advanced_predicate.get_name()

    def get_arity(self) -> 'int':
        """Get the arity of the predicate."""
        return self._advanced_predicate.get_arity()

    def get_parameters(self) -> 'list[Variable]':
        """Get the parameters of the predicate."""
        return [Variable(x) for x in self._advanced_predicate.get_parameters()]

    def __str__(self):
        """Get the string representation of the predicate."""
        return str(self._advanced_predicate)

    def __repr__(self):
        """Get the string representation of the predicate."""
        return repr(self._advanced_predicate)

    def __hash__(self) -> 'int':
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
        assert isinstance(advanced_ground_atom, AdvancedGroundAtom), "Invalid ground atom type."
        self._advanced_ground_atom = advanced_ground_atom

    def get_index(self) -> 'int':
        """Get the index of the ground atom."""
        return self._advanced_ground_atom.get_index()

    def get_predicate(self) -> 'Predicate':
        """Get the predicate of the ground atom."""
        return Predicate(self._advanced_ground_atom.get_predicate())

    def get_terms(self) -> 'list[Object]':
        """Get the objects of the ground atom."""
        return [Object(x) for x in self._advanced_ground_atom.get_objects()]

    def get_arity(self) -> 'int':
        """Get the arity of the ground atom."""
        return self._advanced_ground_atom.get_arity()

    def __str__(self):
        """Get the string representation of the ground atom."""
        return str(self._advanced_ground_atom)

    def __repr__(self):
        """Get the string representation of the ground atom."""
        return repr(self._advanced_ground_atom)

    def __hash__(self) -> 'int':
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
        assert isinstance(advanced_atom, AdvancedAtom), "Invalid atom type."
        self._advanced_atom = advanced_atom

    def get_index(self) -> 'int':
        """Get the index of the atom."""
        return self._advanced_atom.get_index()

    def get_predicate(self) -> 'Predicate':
        """Get the predicate of the atom."""
        return Predicate(self._advanced_atom.get_predicate())

    def get_terms(self) -> 'list[Union[Object, Variable]]':
        """Get the terms of the atom."""
        return [Object(x) if isinstance(x, AdvancedObject) else Variable(x) for x in self._advanced_atom.get_terms()]

    def get_variables(self) -> 'list[Variable]':
        """Get the variables of the atom."""
        return [Variable(x) for x in self._advanced_atom.get_variables()]

    def get_arity(self) -> 'int':
        """Get the arity of the atom."""
        return len(self.get_terms())

    def __str__(self):
        """Get the string representation of the atom."""
        return str(self._advanced_atom)

    def __repr__(self):
        """Get the string representation of the atom."""
        return repr(self._advanced_atom)

    def __hash__(self) -> 'int':
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
        assert isinstance(advanced_ground_literal, AdvancedGroundLiteral), "Invalid ground literal type."
        self._advanced_ground_literal = advanced_ground_literal

    def get_index(self) -> 'int':
        """Get the index of the ground literal."""
        return self._advanced_ground_literal.get_index()

    def get_atom(self) -> 'GroundAtom':
        """Get the atom of the ground literal."""
        return GroundAtom(self._advanced_ground_literal.get_atom())

    def get_polarity(self) -> bool:
        """Get the polarity of the ground literal."""
        return self._advanced_ground_literal.get_polarity()

    def __str__(self):
        """Get the string representation of the ground literal."""
        return str(self._advanced_ground_literal)

    def __repr__(self):
        """Get the string representation of the ground literal."""
        return repr(self._advanced_ground_literal)

    def __hash__(self) -> 'int':
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
        assert isinstance(advanced_literal, AdvancedLiteral), "Invalid literal type."
        self._advanced_literal = advanced_literal

    def get_index(self) -> 'int':
        """Get the index of the literal."""
        return self._advanced_literal.get_index()

    def get_atom(self) -> 'Atom':
        """Get the atom of the literal."""
        return Atom(self._advanced_literal.get_atom())

    def get_polarity(self) -> bool:
        """Get the polarity of the literal."""
        return self._advanced_literal.get_polarity()

    def __str__(self) -> 'str':
        """Get the string representation of the literal."""
        return str(self._advanced_literal)

    def __repr__(self) -> 'str':
        """Get the string representation of the literal."""
        return repr(self._advanced_literal)

    def __hash__(self) -> 'int':
        """Get the hash of the literal."""
        return hash(self._advanced_literal)

    def __eq__(self, other: object) -> bool:
        """Check if two literals are equal."""
        if not isinstance(other, Literal):
            return False
        return self._advanced_literal == other._advanced_literal


def _split_literal_list(literals: list[Literal]) -> 'tuple[AdvancedStaticLiteralList, AdvancedFluentLiteralList, AdvancedDerivedLiteralList]':
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

#     def get_index(self) -> 'int':
#         """Get the index of the numeric constraint."""
#         return self._advanced_numeric_constraint.get_index()

#     def get_binary_comparator(self):
#         """Get the binary comparator of the numeric constraint."""
#         raise NotImplementedError("Not implemented.")
#         return self._advanced_numeric_constraint.get_binary_comparator()

#     def get_left_function_expression(self) -> 'FunctionExpression':
#         """Get the left function expression of the numeric constraint."""
#         return self._advanced_numeric_constraint.get_left_function_expression()

#     def get_right_function_expression(self) -> 'FunctionExpression':
#         """Get the right function expression of the numeric constraint."""
#         return self._advanced_numeric_constraint.get_right_function_expression()


# class NumericEffect:
#     """NumericEffect class for the PDDL domain."""
#     _advanced_numeric_effect: AdvancedNumericEffect = None

#     def __init__(self, advanced_numeric_effect: AdvancedNumericEffect) -> None:
#         self._advanced_numeric_effect = advanced_numeric_effect

#     def get_index(self) -> 'int':
#         """Get the index of the numeric effect."""
#         return self._advanced_numeric_effect.get_index()

#     def get_function_expression(self) -> 'FunctionExpression':
#         """Get the function expression of the numeric effect."""
#         return self._advanced_numeric_effect.get_function_expression()

#     def get_function(self) -> 'FunctionExpression':
#         """Get the function of the numeric effect."""
#         raise NotImplementedError("Not implemented.")


class ConjunctiveEffect:
    """ConjunctiveEffect class for the PDDL domain."""
    _advanced_conjunctive_effect: AdvancedConjunctiveEffect = None

    def __init__(self, advanced_effect: AdvancedConjunctiveEffect) -> None:
        assert isinstance(advanced_effect, AdvancedConjunctiveEffect), "Invalid conjunctive effect type."
        self._advanced_conjunctive_effect = advanced_effect

    def get_index(self) -> 'int':
        """Get the index of the conjunctive effect."""
        return self._advanced_conjunctive_effect.get_index()

    def get_parameters(self) -> 'list[Variable]':
        """Get the parameters of the conjunctive effect."""
        return [Variable(x) for x in self._advanced_conjunctive_effect.get_parameters()]

    def get_literals(self) -> 'list[Literal]':
        """Get the literals of the conjunctive effect."""
        return [Literal(x) for x in self._advanced_conjunctive_effect.get_literals()]

    # def get_numeric_effects(self) -> 'list[NumericEffect]':
    #     """Get the fluent numeric effects of the conjunctive effect."""
    #     numeric_effects = list(self._advanced_conjunctive_effect.get_fluent_numeric_effects())
    #     auxiliary_numeric_effect = self._advanced_conjunctive_effect.get_auxiliary_numeric_effect()
    #     if auxiliary_numeric_effect: numeric_effects.append(auxiliary_numeric_effect)
    #     return numeric_effects

    def __str__(self):
        """Get the string representation of the conjunctive effect."""
        return str(self._advanced_conjunctive_effect)

    def __repr__(self):
        """Get the string representation of the conjunctive effect."""
        return repr(self._advanced_conjunctive_effect)

    def __hash__(self) -> 'int':
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
        assert isinstance(advanced_conditional_effect, AdvancedConditionalEffect), "Invalid conditional effect type."
        self._advanced_conditional_effect = advanced_conditional_effect

    def get_index(self) -> 'int':
        """Get the index of the conditional effect."""
        return self._advanced_conditional_effect.get_index()

    def get_parameters(self) -> 'ConjunctiveCondition':
        """Get the parameters of the conditional effect."""
        return ConjunctiveCondition(self._advanced_conditional_effect.get_parameters())

    def get_static_conditions(self) -> 'ConjunctiveEffect':
        """Get the static conditions of the conditional effect."""
        return ConjunctiveEffect(self._advanced_conditional_effect.get_static_conditions())

    def __str__(self):
        """Get the string representation of the conditional effect."""
        return str(self._advanced_conditional_effect)

    def __repr__(self):
        """Get the string representation of the conditional effect."""
        return repr(self._advanced_conditional_effect)

    def __hash__(self) -> 'int':
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
        assert isinstance(advanced_action, AdvancedAction), "Invalid action type."
        self._advanced_action = advanced_action

    def get_index(self) -> 'int':
        """Get the index of the action."""
        return self._advanced_action.get_index()

    def get_arity(self) -> 'int':
        """Get the arity of the action."""
        return self._advanced_action.get_arity()

    def get_name(self) -> 'str':
        """Get the name of the action."""
        return self._advanced_action.get_name()

    def get_parameters(self) -> 'list[Variable]':
        """Get the parameters of the action."""
        return [Variable(x) for x in self._advanced_action.get_parameters()]

    def get_conjunctive_condition(self) -> 'ConjunctiveCondition':
        """Get the conjunctive condition of the action."""
        return ConjunctiveCondition(self._advanced_action.get_conjunctive_condition())

    def get_conjunctive_effect(self) -> 'ConjunctiveEffect':
        """Get the conjunctive effect of the action."""
        return ConjunctiveEffect(self._advanced_action.get_conjunctive_effect())

    def get_conditional_effect(self) -> 'list[ConditionalEffect]':
        """Get the conditional effect of the action."""
        return [ConditionalEffect(x) for x in self._advanced_action.get_conditional_effects()]

    def __str__(self):
        """Get the string representation of the action."""
        return str(self._advanced_action)

    def __repr__(self):
        """Get the string representation of the action."""
        return repr(self._advanced_action)

    def __hash__(self) -> 'int':
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

    def __init__(self, domain_path: Union[Path, str]) -> None:
        """
        Initialize the domain with the given path.

        Args:
            domain_path (Path): The file path to the domain definition file.
        """
        assert isinstance(domain_path, (Path, str)), "Invalid domain path type."
        self._advanced_parser = AdvancedParser(str(domain_path))
        self._advanced_domain = self._advanced_parser.get_domain()
        self._repositories = self._advanced_domain.get_repositories()

    def get_name(self) -> 'str':
        return self._advanced_domain.get_name()

    def get_requirements(self) -> 'list[str]':
        """Get the requirements of the domain."""
        requirements = str(self._advanced_domain.get_requirements()).removeprefix('(:requirements').removesuffix(')').split()
        return requirements

    def get_actions(self) -> 'list[Action]':
        """Get the actions of the domain."""
        return [Action(x) for x in self._advanced_domain.get_actions()]

    def get_action(self, action_name: str) -> 'Action':
        """Get the action of the domain."""
        for action in self.get_actions():
            if action.get_name() == action_name:
                return action
        raise ValueError(f"Action '{action_name}' not found in the domain.")

    def get_constants(self) -> 'list[Object]':
        """Get the constants of the domain."""
        return [Object(x) for x in self._advanced_domain.get_constants()]

    def get_constant(self, constant_name: str) -> 'Object':
        """Get the constant of the domain."""
        for constant in self.get_constants():
            if constant.get_name() == constant_name:
                return constant
        raise ValueError(f"Constant '{constant_name}' not found in the domain.")

    def get_predicates(self, skip_static = False, skip_fluent = False, skip_derived = False) -> 'list[Predicate]':
        """Get the predicates of the domain."""
        predicates = []
        if not skip_static:
            predicates.extend([Predicate(x) for x in self._advanced_domain.get_static_predicates()])
        if not skip_fluent:
            predicates.extend([Predicate(x) for x in self._advanced_domain.get_fluent_predicates()])
        if not skip_derived:
            predicates.extend([Predicate(x) for x in self._advanced_domain.get_derived_predicates()])
        return predicates

    def get_predicate(self, predicate_name: str) -> 'Predicate':
        """Get the predicate of the domain."""
        for predicate in self.get_predicates():
            if predicate.get_name() == predicate_name:
                return predicate
        raise ValueError(f"Predicate '{predicate_name}' not found in the domain.")


class Problem:
    """Problem class for the PDDL problem."""
    _domain: Domain = None
    _search_context: SearchContext = None
    _advanced_problem: AdvancedProblem = None

    def __init__(self, domain: Domain, problem_path: Union[Path, str], mode: str = 'lifted') -> None:
        """
        Initialize the problem with the given domain, path, and mode.

        Args:
            domain (Domain): The PDDL domain.
            problem_path (Path): The path to the problem definition file.
            mode (str): The search mode, either 'lifted' or 'grounded'.
        """
        assert isinstance(domain, Domain), "Invalid domain type."
        assert isinstance(problem_path, (Path, str)), "Invalid problem path type."
        if mode not in ['lifted', 'grounded']:
            raise ValueError("Invalid mode. Use 'lifted' or 'grounded'.")
        search_mode = SearchMode.LIFTED if mode == 'lifted' else SearchMode.GROUNDED
        self._domain = domain
        advanced_problem = domain._advanced_parser.parse_problem(str(problem_path))
        self._search_context = SearchContext.create(advanced_problem, SearchContextOptions(search_mode))
        self._advanced_problem = self._search_context.get_problem()

    def _to_advanced_term(self, term: Term) -> 'AdvancedTerm':
        assert isinstance(term, (Object, Variable)), "Invalid term type."
        advanced_term = term._advanced_object if isinstance(term, Object) else term._advanced_variable
        return self._advanced_problem.get_or_create_term(advanced_term)

    def get_index(self) -> 'int':
        """Get the index of the problem."""
        return self._advanced_problem.get_index()

    def get_name(self) -> 'str':
        """Get the name of the problem."""
        return self._advanced_problem.get_name()

    def get_objects(self) -> 'list[Object]':
        """Get the objects of the problem."""
        return [Object(x) for x in self._advanced_problem.get_objects()]

    def get_object(self, object_name: str) -> 'Object':
        """Get the object of the problem."""
        for obj in self.get_objects():
            if obj.get_name() == object_name:
                return obj
        raise ValueError(f"Object '{object_name}' not found in the problem.")

    def get_requirements(self) -> 'list[str]':
        """Get the requirements of the problem."""
        requirements = str(self._advanced_problem.get_requirements()).removeprefix('(:requirements').removesuffix(')').split()
        return requirements

    def get_initial_state(self) -> 'State':
        """Get the initial state of the problem."""
        initial_state, _ = self._search_context.get_state_repository().get_or_create_initial_state()
        return State(self, initial_state)

    def get_initial_atoms(self, skip_static = False, skip_fluent = False, skip_derived = False) -> 'list[GroundAtom]':
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
            derived_atoms = problem.get_repositories().get_derived_ground_atoms_from_indices(derived_indices)
            initial_atoms.extend([GroundAtom(x) for x in derived_atoms])
        return initial_atoms

    def get_goal_condition(self) -> 'GroundConjunctiveCondition':
        """Get the goal condition of the problem."""
        static_goal = self._advanced_problem.get_static_goal_condition()
        fluent_goal = self._advanced_problem.get_fluent_goal_condition()
        derived_goal = self._advanced_problem.get_derived_goal_condition()
        return GroundConjunctiveCondition()  # TODO: Dominik -- How do I create one?

    def new_atom(self, predicate: Predicate, terms: list[Term]) -> 'Atom':
        """Create an atom from the given predicate and terms."""
        assert isinstance(predicate, Predicate), "Invalid predicate type."
        advanced_terms = AdvancedTermList([self._to_advanced_term(term) for term in terms])
        return Atom(self._advanced_problem.get_or_create_atom(predicate._advanced_predicate, advanced_terms))

    def new_literal(self, atom: Atom, polarity: bool) -> 'Literal':
        """Create a literal with the given polarity."""
        assert isinstance(atom, Atom), "Invalid atom type."
        return Literal(self._advanced_problem.get_or_create_literal(polarity, atom._advanced_atom))

    def new_ground_atom(self, predicate: Predicate, objects: list[Object]) -> 'GroundAtom':
        """Create a ground atom from the given predicate and objects."""
        assert isinstance(predicate, Predicate), "Invalid predicate type."
        advanced_objects = AdvancedObjectList([x._advanced_object for x in objects])
        return GroundAtom(self._advanced_problem.get_or_create_ground_atom(predicate._advanced_predicate, advanced_objects))

    def new_ground_literal(self, ground_atom: GroundAtom, polarity: bool) -> 'GroundLiteral':
        """Create a ground literal with the given polarity."""
        assert isinstance(ground_atom, GroundAtom), "Invalid ground atom type."
        predicate = ground_atom.get_predicate()
        # variables = predicate.get_parameters()  # TODO: This does not work, for some reason.
        variables = self.new_variable_list(['?x' + str(i + 1) for i in range(predicate.get_arity())])  # TODO: Why does this resolve the issue?
        advanced_terms = AdvancedTermList([self._to_advanced_term(variable) for variable in variables])
        advanced_atom = self._advanced_problem.get_or_create_atom(predicate._advanced_predicate, advanced_terms)
        advanced_literal = self._advanced_problem.get_or_create_literal(polarity, advanced_atom)
        advanced_objects = ground_atom._advanced_ground_atom.get_objects()
        return self._advanced_problem.ground(advanced_literal, advanced_objects)

    def new_variable(self, name: str, parameter_index: int) -> 'Variable':
        return Variable(self._advanced_problem.get_or_create_variable(name, parameter_index))

    def new_variable_list(self, names: list[str]) -> 'list[Variable]':
        return [self.new_variable(name, parameter_index) for name, parameter_index in zip(names, range(len(names)))]

    def new_conjunctive_condition(self, variables: list[Variable], literals: list[Literal]) -> 'ConjunctiveCondition':
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


class State:
    """
    State class representing a node in the search space.
    """
    def __init__(self, problem: Problem, advanced_state: AdvancedState):
        self._problem = problem
        self._advanced_state = advanced_state

    def get_index(self) -> 'int':
        """
        Returns the index of the state.
        """
        return self._advanced_state.get_index()

    def get_ground_atoms(self, skip_static = True, skip_fluent = False, skip_derived = False) -> 'list[GroundAtom]':
        """
        Returns the ground atoms of the state.
        """
        ground_atoms = []
        if not skip_static:
             static_ground_atoms = self._problem.get_initial_atoms(skip_static = False, skip_fluent = True, skip_derived = True)
             ground_atoms.extend([GroundAtom(x) for x in static_ground_atoms])
        if not skip_fluent:
            fluent_indices = self._advanced_state.get_fluent_atoms()
            fluent_ground_atoms = self._problem._advanced_problem.get_repositories().get_fluent_ground_atoms_from_indices(fluent_indices)
            ground_atoms.extend([GroundAtom(x) for x in fluent_ground_atoms])
        if not skip_derived:
            derived_indices = self._advanced_state.get_derived_atoms()
            derived_ground_atoms = self._problem._advanced_problem.get_repositories().get_derived_ground_atoms_from_indices(derived_indices)
            ground_atoms.extend([GroundAtom(x) for x in derived_ground_atoms])
        return ground_atoms

    def literal_holds(self, literal: GroundLiteral) -> bool:
        """
        Checks if a literal holds in the state.
        """
        return self._advanced_state.literal_holds(literal)

    def literals_hold(self, atoms: list[GroundLiteral]) -> bool:
        """
        Checks if all literals hold in the state.
        """
        return self._advanced_state.literals_hold(atoms)

    def __str__(self) -> 'str':
        """
        Returns a string representation of the state.
        """
        return self._advanced_state.to_string(self._problem._advanced_problem)

    def __repr__(self) -> 'str':
        """
        Returns a string representation of the state.
        """
        return self.__str__()

    def __hash__(self) -> 'int':
        """
        Returns a hash of the state.
        """
        return hash(self._advanced_state)

    def __eq__(self, other) -> bool:
        """
        Checks if two states are equal.
        """
        if not isinstance(other, State):
            return False
        return self._advanced_state == other._advanced_state


class ApplicableActionGenerator:
    """
    Class to generate applicable actions for a given state.
    """
    def __init__(self, problem: Problem):
        self._problem = problem

    def get_applicable_actions(self, state: State) -> 'list[Action]':
        """
        Returns the applicable actions for the given state.
        """
        return self._problem.get_applicable_actions(state._advanced_state)


class GroundConjunctiveCondition:
    _advanced_ground_conjunctive_condition: AdvancedGroundConjunctiveCondition = None

    def __init__(self, advanced_ground_conjunctive_condition: AdvancedGroundConjunctiveCondition) -> None:
        assert isinstance(advanced_ground_conjunctive_condition, AdvancedGroundConjunctiveCondition), "Invalid ground conjunctive condition type."
        self._advanced_ground_conjunctive_condition = advanced_ground_conjunctive_condition

    def holds(self, state: State) -> bool:
        """Check if the ground conjunctive condition holds in the given state."""
        assert isinstance(state, State), "Invalid state type."
        static_pos = self._advanced_ground_conjunctive_condition.get_static_positive_condition()
        static_neg = self._advanced_ground_conjunctive_condition.get_static_negative_condition()
        fluent_pos = self._advanced_ground_conjunctive_condition.get_fluent_positive_condition()
        fluent_neg = self._advanced_ground_conjunctive_condition.get_fluent_negative_condition()
        derived_pos = self._advanced_ground_conjunctive_condition.get_derived_positive_condition()
        derived_neg = self._advanced_ground_conjunctive_condition.get_derived_negative_condition()
        raise NotImplementedError("Not implemented.")
        # return state.literal_holds(static_pos) and \
        #        state.literal_holds(static_neg) and \
        #        state.literal_holds(fluent_pos) and \
        #        state.literal_holds(fluent_neg) and \
        #        state.literal_holds(derived_pos) and \
        #        state.literal_holds(derived_neg)


class ConjunctiveCondition:
    _advanced_conjunctive_condition: AdvancedConjunctiveCondition = None

    def __init__(self, advanced_condition: AdvancedConjunctiveCondition) -> None:
        assert isinstance(advanced_condition, AdvancedConjunctiveCondition), "Invalid conjunctive condition type."
        self._advanced_conjunctive_condition = advanced_condition

    def get_parameters(self) -> 'list[Variable]':
        """Get the parameters of the conjunctive condition."""
        return [Variable(x) for x in self._advanced_conjunctive_condition.get_parameters()]

    def get_literals(self, skip_static = False, skip_fluent = False, skip_derived = False) -> 'list[Literal]':
        """Get the literals of the conjunctive condition."""
        literals = []
        if not skip_static:
            literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_static_literals()])
        if not skip_fluent:
            literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_fluent_literals()])
        if not skip_derived:
            literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_derived_literals()])
        return literals

    def get_nullary_ground_literals(self, skip_static = False, skip_fluent = False, skip_derived = False) -> 'list[GroundLiteral]':
        """Get the nullary ground literals of the conjunctive condition."""
        ground_literals = []
        if not skip_static:
            ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_static_literals()])
        if not skip_fluent:
            ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_fluent_literals()])
        if not skip_derived:
            ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_derived_literals()])
        return ground_literals

    def __str__(self):
        """Get the string representation of the conjunctive condition."""
        return str(self._advanced_conjunctive_condition)

    def __repr__(self):
        """Get the string representation of the conjunctive condition."""
        return repr(self._advanced_conjunctive_condition)

    def __hash__(self) -> 'int':
        """Get the hash of the conjunctive condition."""
        return hash(self._advanced_conjunctive_condition)

    def __eq__(self, other: object) -> bool:
        """Check if two conjunctive conditions are equal."""
        if not isinstance(other, ConjunctiveCondition):
            return False
        return self._advanced_conjunctive_condition == other._advanced_conjunctive_condition

    # def get_numeric_constraints(self) -> 'list[NumericConstraint]':
    #     """Get the numeric constraints of the conjunctive condition."""
    #     return list(self._advanced_conjunctive_condition.get_numeric_constraints())
