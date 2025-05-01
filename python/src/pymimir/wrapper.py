from pathlib import Path
from typing import  Union, Iterable

from pymimir.advanced.formalism import Action as AdvancedAction
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
from pymimir.advanced.formalism import Domain as AdvancedDomain
from pymimir.advanced.formalism import FluentAtom as AdvancedFluentAtom
from pymimir.advanced.formalism import FluentGroundAtom as AdvancedFluentGroundAtom
from pymimir.advanced.formalism import FluentGroundAtomList as AdvancedFluentGroundAtomList
from pymimir.advanced.formalism import FluentGroundLiteral as AdvancedFluentGroundLiteral
from pymimir.advanced.formalism import FluentGroundLiteralList as AdvancedFluentGroundLiteralList
from pymimir.advanced.formalism import FluentLiteral as AdvancedFluentLiteral
from pymimir.advanced.formalism import FluentLiteralList as AdvancedFluentLiteralList
from pymimir.advanced.formalism import FluentPredicate as AdvancedFluentPredicate
from pymimir.advanced.formalism import GroundAction as AdvancedGroundAction
from pymimir.advanced.formalism import GroundConditionalEffect as AdvancedGroundConditionalEffect
from pymimir.advanced.formalism import GroundConjunctiveCondition as AdvancedGroundConjunctiveCondition
from pymimir.advanced.formalism import GroundConjunctiveEffect as AdvancedGroundConjunctiveEffect
from pymimir.advanced.formalism import NumericConstraintList as AdvancedNumericConstraintList
from pymimir.advanced.formalism import Object as AdvancedObject
from pymimir.advanced.formalism import ObjectList as AdvancedObjectList
from pymimir.advanced.formalism import Parser as AdvancedParser
from pymimir.advanced.formalism import Problem as AdvancedProblem
from pymimir.advanced.formalism import Repositories as AdvancedRepositories
from pymimir.advanced.formalism import StaticAtom as AdvancedStaticAtom
from pymimir.advanced.formalism import StaticGroundAtom as AdvancedStaticGroundAtom
from pymimir.advanced.formalism import StaticGroundAtomList as AdvancedStaticGroundAtomList
from pymimir.advanced.formalism import StaticGroundLiteral as AdvancedStaticGroundLiteral
from pymimir.advanced.formalism import StaticGroundLiteralList as AdvancedStaticGroundLiteralList
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


# -------------------------------
# Type aliases for advanced types
# -------------------------------

AdvancedAtom = Union[AdvancedStaticAtom, AdvancedFluentAtom, AdvancedDerivedAtom]
AdvancedGroundAtom = Union[AdvancedStaticGroundAtom, AdvancedFluentGroundAtom, AdvancedDerivedGroundAtom]
AdvancedGroundAtomList = Union[AdvancedStaticGroundAtomList, AdvancedFluentGroundAtomList, AdvancedDerivedGroundAtomList]
AdvancedLiteral = Union[AdvancedStaticLiteral, AdvancedFluentLiteral, AdvancedDerivedLiteral]
AdvancedLiteralList = Union[AdvancedStaticLiteralList, AdvancedFluentLiteralList, AdvancedDerivedLiteralList]
AdvancedGroundLiteral = Union[AdvancedStaticGroundLiteral, AdvancedFluentGroundLiteral, AdvancedDerivedGroundLiteral]
AdvancedGroundLiteralList = Union[AdvancedStaticGroundLiteralList, AdvancedFluentGroundLiteralList, AdvancedDerivedGroundLiteralList]
AdvancedPredicate = Union[AdvancedStaticPredicate, AdvancedFluentPredicate, AdvancedDerivedPredicate]
AdvancedAxiomEvaluator = Union[AdvancedGroundedAxiomEvaluator, AdvancedLiftedAxiomEvaluator]


# ------------
# Type aliases
# ------------

Term = Union['Object', 'Variable']


# ----------------
# Helper functions
# ----------------

def _split_ground_literal_list(literals: 'list[GroundLiteral]') -> 'tuple[AdvancedStaticGroundLiteralList, AdvancedFluentGroundLiteralList, AdvancedDerivedGroundLiteralList]':
    """Split the ground literal list into static, fluent, and derived ground literals."""
    advanced_static_ground_literals = AdvancedStaticGroundLiteralList([x._advanced_ground_literal for x in literals if isinstance(x._advanced_ground_literal, AdvancedStaticGroundLiteral)])
    advanced_fluent_ground_literals = AdvancedFluentGroundLiteralList([x._advanced_ground_literal for x in literals if isinstance(x._advanced_ground_literal, AdvancedFluentGroundLiteral)])
    advanced_derived_ground_literals = AdvancedDerivedGroundLiteralList([x._advanced_ground_literal for x in literals if isinstance(x._advanced_ground_literal, AdvancedDerivedGroundLiteral)])
    assert len(advanced_static_ground_literals) + len(advanced_fluent_ground_literals) + len(advanced_derived_ground_literals) == len(literals), "Invalid ground literal list."
    return advanced_static_ground_literals, advanced_fluent_ground_literals, advanced_derived_ground_literals

def _split_literal_list(literals: 'list[Literal]') -> 'tuple[AdvancedStaticLiteralList, AdvancedFluentLiteralList, AdvancedDerivedLiteralList]':
    """Split the literal list into static, fluent, and derived literals."""
    advanced_static_literals = AdvancedStaticLiteralList([x._advanced_literal for x in literals if isinstance(x._advanced_literal, AdvancedStaticLiteral)])
    advanced_fluent_literals = AdvancedFluentLiteralList([x._advanced_literal for x in literals if isinstance(x._advanced_literal, AdvancedFluentLiteral)])
    advanced_derived_literals = AdvancedDerivedLiteralList([x._advanced_literal for x in literals if isinstance(x._advanced_literal, AdvancedDerivedLiteral)])
    assert len(advanced_static_literals) + len(advanced_fluent_literals) + len(advanced_derived_literals) == len(literals), "Invalid literal list."
    return advanced_static_literals, advanced_fluent_literals, advanced_derived_literals


# -------
# Classes
# -------

class Variable:
    _advanced_variable: 'AdvancedVariable' = None

    def __init__(self, advanced_variable: 'AdvancedVariable') -> None:
        """Internal constructor for the Variable class; to create a variable, use the new() method."""
        assert isinstance(advanced_variable, AdvancedVariable), "Invalid variable type."
        self._advanced_variable = advanced_variable

    @staticmethod
    def new(name: 'str', parameter_index: 'int', problem: 'Problem') -> 'Variable':
        """Create a new variable with the given name."""
        return problem.new_variable(name, parameter_index)

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

    def __eq__(self, other: 'object') -> bool:
        """Check if two variables are equal."""
        if not isinstance(other, Variable):
            return False
        return self._advanced_variable == other._advanced_variable


class Object:
    """Object class for the PDDL domain."""
    _advanced_object: 'AdvancedObject' = None

    def __init__(self, advanced_object: 'AdvancedObject') -> None:
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

    def __eq__(self, other: 'object') -> bool:
        """Check if two objects are equal."""
        if not isinstance(other, Object):
            return False
        return self._advanced_object == other._advanced_object


class Predicate:
    """Predicate class for the PDDL domain."""
    _advanced_predicate: 'AdvancedPredicate' = None

    def __init__(self, advanced_predicate: 'AdvancedPredicate') -> None:
        assert isinstance(advanced_predicate, AdvancedStaticPredicate) \
            or isinstance(advanced_predicate, AdvancedFluentPredicate) \
            or isinstance(advanced_predicate, AdvancedDerivedPredicate), "Invalid predicate type."
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

    def __eq__(self, other: 'object') -> bool:
        """Check if two predicates are equal."""
        if not isinstance(other, Predicate):
            return False
        return self._advanced_predicate == other._advanced_predicate


class GroundAtom:
    """GroundAtom class for the PDDL domain."""
    _advanced_ground_atom: 'AdvancedGroundAtom' = None

    def __init__(self, advanced_ground_atom: 'AdvancedGroundAtom') -> None:
        """Internal constructor for the GroundAtom class; to create a ground atom, use the new() method."""
        assert isinstance(advanced_ground_atom, AdvancedStaticGroundAtom) \
            or isinstance(advanced_ground_atom, AdvancedFluentGroundAtom) \
            or isinstance(advanced_ground_atom, AdvancedDerivedGroundAtom), "Invalid ground atom type."
        self._advanced_ground_atom = advanced_ground_atom

    @staticmethod
    def new(predicate: 'Predicate', objects: 'list[Object]', problem: 'Problem') -> 'GroundAtom':
        """Create a new ground atom with the given predicate and objects."""
        return problem.new_ground_atom(predicate, objects)

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

    def is_static(self) -> 'bool':
        """Get whether the ground atom is static."""
        return isinstance(self._advanced_ground_atom, AdvancedStaticGroundAtom)

    def is_fluent(self) -> 'bool':
        """Get whether the ground atom is fluent."""
        return isinstance(self._advanced_ground_atom, AdvancedFluentGroundAtom)

    def is_derived(self) -> 'bool':
        """Get whether the ground atom is derived."""
        return isinstance(self._advanced_ground_atom, AdvancedDerivedGroundAtom)

    def __str__(self):
        """Get the string representation of the ground atom."""
        return str(self._advanced_ground_atom)

    def __repr__(self):
        """Get the string representation of the ground atom."""
        return repr(self._advanced_ground_atom)

    def __hash__(self) -> 'int':
        """Get the hash of the ground atom."""
        return hash(self._advanced_ground_atom)

    def __eq__(self, other: 'object') -> bool:
        """Check if two ground atoms are equal."""
        if not isinstance(other, GroundAtom):
            return False
        return self._advanced_ground_atom == other._advanced_ground_atom


class Atom:
    """Atom class for the PDDL domain."""
    _advanced_atom: 'AdvancedAtom' = None

    def __init__(self, advanced_atom: 'AdvancedAtom') -> None:
        """Internal constructor for the Atom class; to create an atom, use the new() method."""
        assert isinstance(advanced_atom, AdvancedStaticAtom) \
            or isinstance(advanced_atom, AdvancedFluentAtom) \
            or isinstance(advanced_atom, AdvancedDerivedAtom), "Invalid atom type."
        self._advanced_atom = advanced_atom

    @staticmethod
    def new(predicate: 'Predicate', terms: 'list[Term]', problem: 'Problem') -> 'Atom':
        """Create a new atom with the given predicate and terms."""
        return problem.new_atom(predicate, terms)

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

    def __eq__(self, other: 'object') -> bool:
        """Check if two atoms are equal."""
        if not isinstance(other, Atom):
            return False
        return self._advanced_atom == other._advanced_atom


class GroundLiteral:
    """GroundLiteral class for the PDDL domain."""
    _advanced_ground_literal: 'AdvancedGroundLiteral' = None

    def __init__(self, advanced_ground_literal: 'AdvancedGroundLiteral') -> None:
        """Internal constructor for the GroundLiteral class; to create a ground literal, use the new() method."""
        assert isinstance(advanced_ground_literal, AdvancedGroundLiteral), "Invalid ground literal type."
        self._advanced_ground_literal = advanced_ground_literal

    @staticmethod
    def new(ground_atom: 'GroundAtom', polarity: 'bool', problem: 'Problem') -> 'GroundLiteral':
        """Create a new ground literal with the given ground atom and polarity."""
        return problem.new_ground_literal(ground_atom, polarity)

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

    def __eq__(self, other: 'object') -> bool:
        """Check if two ground literals are equal."""
        if not isinstance(other, GroundLiteral):
            return False
        return self._advanced_ground_literal == other._advanced_ground_literal


class Literal:
    """Literal class for the PDDL domain."""
    _advanced_literal: 'AdvancedLiteral' = None

    def __init__(self, advanced_literal: 'AdvancedLiteral') -> None:
        """Internal constructor for the Literal class; to create a literal, use the new() method."""
        assert isinstance(advanced_literal, AdvancedStaticLiteral) \
            or isinstance(advanced_literal, AdvancedFluentLiteral) \
            or isinstance(advanced_literal, AdvancedDerivedLiteral), "Invalid literal type."
        self._advanced_literal = advanced_literal

    @staticmethod
    def new(atom: 'Atom', polarity: 'bool', problem: 'Problem') -> 'Literal':
        """Create a new literal with the given atom and polarity."""
        return problem.new_literal(atom, polarity)

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

    def __eq__(self, other: 'object') -> bool:
        """Check if two literals are equal."""
        if not isinstance(other, Literal):
            return False
        return self._advanced_literal == other._advanced_literal


class GroundEffect:
    """GroundEffectList class for the PDDL domain."""
    _advanced_ground_effect: 'AdvancedGroundConjunctiveEffect' = None

    def __init__(self, advanced_effect: 'AdvancedGroundConjunctiveEffect', problem: 'Problem') -> None:
        assert isinstance(advanced_effect, AdvancedGroundConjunctiveEffect), "Invalid conjunctive effect type."
        assert isinstance(problem, Problem), "Invalid problem type."
        self._advanced_ground_effect = advanced_effect
        self._problem = problem

    def get_index(self) -> 'int':
        """Get the index of the ground effect list."""
        return self._advanced_ground_effect.get_index()

    def get_add_list(self) -> 'list[GroundAtom]':
        """Get the add list of the ground effect."""
        repositories = self._problem._advanced_problem.get_repositories()
        advanced_ground_atoms = repositories.get_fluent_ground_atoms_from_indices(self._advanced_ground_effect.get_positive_effects())
        return [GroundAtom(x) for x in advanced_ground_atoms]

    def get_delete_list(self) -> 'list[GroundAtom]':
        """Get the delete list of the ground effect."""
        repositories = self._problem._advanced_problem.get_repositories()
        advanced_ground_atoms = repositories.get_fluent_ground_atoms_from_indices(self._advanced_ground_effect.get_negative_effects())
        return [GroundAtom(x) for x in advanced_ground_atoms]

    def __str__(self):
        """Get the string representation of the ground effect list."""
        return self._advanced_ground_effect.to_string(self._problem._advanced_problem)

    def __repr__(self):
        """Get the string representation of the ground effect list."""
        return self._advanced_ground_effect.to_string(self._problem._advanced_problem)

    def __hash__(self) -> 'int':
        """Get the hash of the ground effect list."""
        return hash(self._advanced_ground_effect)

    def __eq__(self, other: 'object') -> bool:
        """Check if two ground effect lists are equal."""
        if not isinstance(other, GroundEffect):
            return False
        return self._advanced_ground_effect == other._advanced_ground_effect


class GroundConditionalEffect:
    """GroundConditionalEffect class for the PDDL domain."""
    _advanced_conditional_effect: 'AdvancedGroundConditionalEffect' = None

    def __init__(self, advanced_conditional_effect: 'AdvancedGroundConditionalEffect', problem: 'Problem') -> None:
        assert isinstance(advanced_conditional_effect, AdvancedGroundConditionalEffect), "Invalid conditional effect type."
        assert isinstance(problem, Problem), "Invalid problem type."
        self._advanced_conditional_effect = advanced_conditional_effect
        self._problem = problem

    def get_index(self) -> 'int':
        """Get the index of the ground conditional effect."""
        return self._advanced_conditional_effect.get_index()

    def get_condition(self) -> 'GroundConjunctiveCondition':
        """Get the condition of the ground conditional effect."""
        return GroundConjunctiveCondition(advanced_condition=self._advanced_conditional_effect.get_conjunctive_condition())

    def get_effect(self) -> 'GroundEffect':
        """Get the effect list of the ground conditional effect."""
        return GroundEffect(self._advanced_conditional_effect.get_conjunctive_effect(), self._problem)

    def __str__(self):
        """Get the string representation of the ground conditional effect."""
        return self._advanced_conditional_effect.to_string(self._problem._advanced_problem)

    def __repr__(self):
        """Get the string representation of the ground conditional effect."""
        return self._advanced_conditional_effect.to_string(self._problem._advanced_problem)

    def __hash__(self) -> 'int':
        """Get the hash of the ground conditional effect."""
        return hash(self._advanced_conditional_effect)

    def __eq__(self, other: 'object') -> bool:
        """Check if two ground conditional effects are equal."""
        if not isinstance(other, GroundConditionalEffect):
            return False
        return self._advanced_conditional_effect == other._advanced_conditional_effect


class Effect:
    """EffectList class for the PDDL domain."""
    _advanced_conjunctive_effect: 'AdvancedConjunctiveEffect' = None

    def __init__(self, advanced_effect: 'AdvancedConjunctiveEffect') -> None:
        assert isinstance(advanced_effect, AdvancedConjunctiveEffect), "Invalid conjunctive effect type."
        self._advanced_conjunctive_effect = advanced_effect

    def get_index(self) -> 'int':
        """Get the index of the effect list."""
        return self._advanced_conjunctive_effect.get_index()

    def get_parameters(self) -> 'list[Variable]':
        """Get the parameters of the effect list."""
        return [Variable(x) for x in self._advanced_conjunctive_effect.get_parameters()]

    def get_literals(self) -> 'list[Literal]':
        """Get the literals of the effect list."""
        return [Literal(x) for x in self._advanced_conjunctive_effect.get_literals()]

    def __iter__(self) -> 'Iterable[Literal]':
        """Get an iterator over the literals of the effect list."""
        return iter(self.get_literals())

    def __str__(self):
        """Get the string representation of the effect list."""
        return str(self._advanced_conjunctive_effect)

    def __repr__(self):
        """Get the string representation of the effect list."""
        return repr(self._advanced_conjunctive_effect)

    def __hash__(self) -> 'int':
        """Get the hash of the effect list."""
        return hash(self._advanced_conjunctive_effect)

    def __eq__(self, other: 'object') -> bool:
        """Check if two effect lists are equal."""
        if not isinstance(other, Effect):
            return False
        return self._advanced_conjunctive_effect == other._advanced_conjunctive_effect


class ConditionalEffect:
    """ConditionalEffect class for the PDDL domain."""
    _advanced_conditional_effect: 'AdvancedConditionalEffect' = None

    def __init__(self, advanced_conditional_effect: 'AdvancedConditionalEffect') -> None:
        assert isinstance(advanced_conditional_effect, AdvancedConditionalEffect), "Invalid conditional effect type."
        self._advanced_conditional_effect = advanced_conditional_effect

    def get_index(self) -> 'int':
        """Get the index of the conditional effect."""
        return self._advanced_conditional_effect.get_index()

    def get_conjunctive_condition(self) -> 'ConjunctiveCondition':
        """Get the static conditions of the conditional effect."""
        return ConjunctiveCondition(self._advanced_conditional_effect.get_conjunctive_condition())

    def get_conjunctive_effect(self) -> 'Effect':
        """Get the effect list of the conditional effect."""
        return Effect(self._advanced_conditional_effect.get_conjunctive_effect())

    def __str__(self):
        """Get the string representation of the conditional effect."""
        return str(self._advanced_conditional_effect)

    def __repr__(self):
        """Get the string representation of the conditional effect."""
        return repr(self._advanced_conditional_effect)

    def __hash__(self) -> 'int':
        """Get the hash of the conditional effect."""
        return hash(self._advanced_conditional_effect)

    def __eq__(self, other: 'object') -> bool:
        """Check if two conditional effects are equal."""
        if not isinstance(other, ConditionalEffect):
            return False
        return self._advanced_conditional_effect == other._advanced_conditional_effect


class Action:
    """Class representing action schemas in the domain."""
    _advanced_action: 'AdvancedAction' = None

    def __init__(self, advanced_action: 'AdvancedAction') -> None:
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

    def get_precondition(self) -> 'ConjunctiveCondition':
        """Get the conjunctive condition of the action."""
        return ConjunctiveCondition(self._advanced_action.get_conjunctive_condition())

    def get_effect(self) -> 'tuple[Effect, list[ConditionalEffect]]':
        """Get both the unconditional and conditional effects of the action."""
        return self.get_unconditional_effect(), self.get_conditional_effect()

    def get_unconditional_effect(self) -> 'Effect':
        """Get the effect list of the action."""
        return Effect(self._advanced_action.get_conjunctive_effect())

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

    def __eq__(self, other: 'object') -> bool:
        """Check if two actions are equal."""
        if not isinstance(other, Action):
            return False
        return self._advanced_action == other._advanced_action


class GroundAction:
    """Class representing ground actions in the problem."""
    _advanced_ground_action: 'AdvancedGroundAction' = None

    def __init__(self, advanced_ground_action: 'AdvancedGroundAction', problem: 'Problem') -> None:
        assert isinstance(advanced_ground_action, AdvancedGroundAction), "Invalid ground action type."
        self._advanced_ground_action = advanced_ground_action
        self._problem = problem

    def get_index(self) -> 'int':
        """Get the index of the ground action."""
        return self._advanced_ground_action.get_index()

    def get_action(self) -> 'Action':
        """Get the name of the ground action."""
        return Action(self._advanced_ground_action.get_action())

    def get_objects(self) -> 'list[Object]':
        """Get the objects of the ground action."""
        return [Object(x) for x in self._advanced_ground_action.get_objects()]

    def get_precondition(self) -> 'GroundConjunctiveCondition':
        """Get the conjunctive condition of the ground action."""
        return GroundConjunctiveCondition(advanced_condition=self._advanced_ground_action.get_conjunctive_condition())

    def get_effect(self) -> 'tuple[GroundEffect, list[GroundConditionalEffect]]':
        """Get both the unconditional and conditional effects of the ground action."""
        return self.get_unconditional_effect(), self.get_conditional_effect()

    def get_unconditional_effect(self) -> 'GroundEffect':
        """Get the effect list of the ground action."""
        return GroundEffect(advanced_effect=self._advanced_ground_action.get_conjunctive_effect())

    def get_conditional_effect(self) -> 'list[GroundConditionalEffect]':
        """Get the conditional effect of the ground action."""
        return [GroundConditionalEffect(x) for x in self._advanced_ground_action.get_conditional_effects()]

    def apply(self, state: 'State') -> 'State':
        """Apply the ground action to the given state."""
        assert isinstance(state, State), "Invalid state type."
        assert state._problem == self._problem, "State and action belong to different problems."
        sr = self._problem._search_context.get_state_repository()
        advanced_successor_state, _ = sr.get_or_create_successor_state(state._advanced_state, self._advanced_ground_action, 0.0)
        return State(advanced_successor_state, self._problem)

    def __str__(self):
        """Get the string representation of the ground action."""
        return self._advanced_ground_action.to_string_for_plan(self._problem._advanced_problem)

    def __repr__(self):
        """Get the string representation of the ground action."""
        return self._advanced_ground_action.to_string_for_plan(self._problem._advanced_problem)

    def __hash__(self) -> 'int':
        """Get the hash of the ground action."""
        return hash(self._advanced_ground_action)

    def __eq__(self, other: 'object') -> bool:
        """Check if two ground actions are equal."""
        if not isinstance(other, GroundAction):
            return False
        return self._advanced_ground_action == other._advanced_ground_action


class Domain:
    """Domain class for the PDDL domain."""
    _advanced_parser: 'AdvancedParser' = None
    _advanced_domain: 'AdvancedDomain' = None
    _repositories: 'AdvancedRepositories' = None

    def __init__(self, domain_path: 'Union[Path, str]') -> None:
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
        # The requirements are returned as a string in the format '(:requirements :requirement1 :requirement2 ...)'.
        # We remove the '(:requirements' prefix and the ')' suffix.
        requirements = str(self._advanced_domain.get_requirements())[15:-1].split()
        return requirements

    def get_actions(self) -> 'list[Action]':
        """Get the actions of the domain."""
        return [Action(x) for x in self._advanced_domain.get_actions()]

    def get_action(self, action_name: 'str') -> 'Action':
        """Get the action of the domain."""
        assert isinstance(action_name, str), "Invalid action name type."
        for action in self.get_actions():
            if action.get_name() == action_name:
                return action
        raise ValueError(f"Action '{action_name}' not found in the domain.")

    def get_constants(self) -> 'list[Object]':
        """Get the constants of the domain."""
        return [Object(x) for x in self._advanced_domain.get_constants()]

    def get_constant(self, constant_name: 'str') -> 'Object':
        """Get the constant of the domain."""
        assert isinstance(constant_name, str), "Invalid constant name type."
        for constant in self.get_constants():
            if constant.get_name() == constant_name:
                return constant
        raise ValueError(f"Constant '{constant_name}' not found in the domain.")

    def get_predicates(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[Predicate]':
        """Get the predicates of the domain."""
        assert isinstance(ignore_static, bool), "Invalid ignore_static type."
        assert isinstance(ignore_fluent, bool), "Invalid ignore_fluent type."
        assert isinstance(ignore_derived, bool), "Invalid ignore_derived type."
        predicates = []
        if not ignore_static:
            predicates.extend([Predicate(x) for x in self._advanced_domain.get_static_predicates()])
        if not ignore_fluent:
            predicates.extend([Predicate(x) for x in self._advanced_domain.get_fluent_predicates()])
        if not ignore_derived:
            predicates.extend([Predicate(x) for x in self._advanced_domain.get_derived_predicates()])
        return predicates

    def get_predicate(self, predicate_name: 'str') -> 'Predicate':
        """Get the predicate of the domain."""
        assert isinstance(predicate_name, str), "Invalid predicate name type."
        for predicate in self.get_predicates():
            if predicate.get_name() == predicate_name:
                return predicate
        raise ValueError(f"Predicate '{predicate_name}' not found in the domain.")


class Problem:
    """Problem class for the PDDL problem."""
    _domain: 'Domain' = None
    _search_context: 'SearchContext' = None
    _advanced_problem: 'AdvancedProblem' = None
    _static_ground_atom_indices: 'set[int]' = None

    def __init__(self, domain: 'Domain', problem_path: Union[Path, str], mode: 'str' = 'lifted') -> None:
        """
        Initialize the problem with the given domain, path, and mode.

        Args:
            domain (Domain): The PDDL domain.
            problem_path (Path): The path to the problem definition file.
            mode (str): The search mode, either 'lifted' or 'grounded'.
        """
        assert isinstance(domain, Domain), "Invalid domain type."
        assert isinstance(problem_path, (Path, str)), "Invalid problem path type."
        assert isinstance(mode, str), "Invalid mode type."
        if mode not in ['lifted', 'grounded']:
            raise ValueError("Invalid mode. Use 'lifted' or 'grounded'.")
        search_mode = SearchMode.LIFTED if mode == 'lifted' else SearchMode.GROUNDED
        self._domain = domain
        advanced_problem = domain._advanced_parser.parse_problem(str(problem_path))
        self._search_context = SearchContext.create(advanced_problem, SearchContextOptions(search_mode))
        self._advanced_problem = self._search_context.get_problem()
        self._static_ground_atom_indices = { atom.get_index() for atom in self._advanced_problem.get_static_initial_atoms() }

    def _to_advanced_term(self, term: 'Term') -> 'AdvancedTerm':
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

    def get_object(self, object_name: 'str') -> 'Object':
        """Get the object of the problem."""
        assert isinstance(object_name, str), "Invalid object name type."
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
        return State(initial_state, self)

    def get_initial_atoms(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[GroundAtom]':
        """Get the initial atoms of the problem."""
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

    def get_goal_condition(self) -> 'GroundConjunctiveCondition':
        """Get the goal condition of the problem."""
        static_goal = self._advanced_problem.get_static_goal_condition()
        fluent_goal = self._advanced_problem.get_fluent_goal_condition()
        derived_goal = self._advanced_problem.get_derived_goal_condition()
        return GroundConjunctiveCondition(static_goal, fluent_goal, derived_goal)

    def new_atom(self, predicate: 'Predicate', terms: 'list[Term]') -> 'Atom':
        """Create an atom from the given predicate and terms."""
        assert isinstance(predicate, Predicate), "Invalid predicate type."
        advanced_terms = AdvancedTermList([self._to_advanced_term(term) for term in terms])
        return Atom(self._advanced_problem.get_or_create_atom(predicate._advanced_predicate, advanced_terms))

    def new_literal(self, atom: 'Atom', polarity: 'bool') -> 'Literal':
        """Create a literal with the given polarity."""
        assert isinstance(atom, Atom), "Invalid atom type."
        return Literal(self._advanced_problem.get_or_create_literal(polarity, atom._advanced_atom))

    def new_ground_atom(self, predicate: 'Predicate', objects: 'list[Object]') -> 'GroundAtom':
        """Create a ground atom from the given predicate and objects."""
        assert isinstance(predicate, Predicate), "Invalid predicate type."
        advanced_objects = AdvancedObjectList([x._advanced_object for x in objects])
        return GroundAtom(self._advanced_problem.get_or_create_ground_atom(predicate._advanced_predicate, advanced_objects))

    def new_ground_literal(self, ground_atom: 'GroundAtom', polarity: 'bool') -> 'GroundLiteral':
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

    def new_variable(self, name: 'str', parameter_index: 'int') -> 'Variable':
        assert isinstance(name, str), "Invalid variable name type."
        assert isinstance(parameter_index, int), "Invalid parameter index type."
        return Variable(self._advanced_problem.get_or_create_variable(name, parameter_index))

    def new_variable_list(self, names: 'list[str]') -> 'list[Variable]':
        assert isinstance(names, list), "Invalid names type."
        return [self.new_variable(name, parameter_index) for name, parameter_index in zip(names, range(len(names)))]

    def new_conjunctive_condition(self, variables: 'list[Variable]', literals: 'list[Literal]') -> 'ConjunctiveCondition':
        """Create a conjunctive condition from the given parameters, literals, and ground literals."""
        assert isinstance(variables, list), "Invalid variables type."
        assert isinstance(literals, list), "Invalid literals type."
        advanced_variables = AdvancedVariableList([x._advanced_variable for x in variables])
        advanced_static_literals, advanced_fluent_literals, advanced_derived_literals = _split_literal_list(literals)
        advanced_numeric_constraints = AdvancedNumericConstraintList()
        return ConjunctiveCondition(self._advanced_problem.get_or_create_conjunctive_condition(advanced_variables,
                                                                                               advanced_static_literals,
                                                                                               advanced_fluent_literals,
                                                                                               advanced_derived_literals,
                                                                                               advanced_numeric_constraints))


class State:
    """
    Class representing a state in the search space.
    """
    def __init__(self, advanced_state: 'AdvancedState', problem: 'Problem') -> None:
        self._problem = problem
        self._advanced_state = advanced_state

    def get_index(self) -> 'int':
        """
        Returns the index of the state.
        """
        return self._advanced_state.get_index()

    def get_ground_atoms(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[GroundAtom]':
        """
        Returns the ground atoms of the state.
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

    def contains(self, ground_atom: 'GroundAtom') -> bool:
        """
        Checks if a ground atom is contained in the state.
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
        """
        if isinstance(literal._advanced_ground_literal, AdvancedStaticGroundLiteral):
            atom_is_in_state = (literal.get_atom().get_index() in self._problem._static_ground_atom_indices)
            return literal.get_polarity() == atom_is_in_state
        return self._advanced_state.literal_holds(literal)

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
        """
        assert isinstance(literals, list), "Invalid literals type."
        advanced_static_ground_literals, advanced_fluent_ground_literals, advanced_derived_ground_literals = _split_ground_literal_list(literals)
        holds = self._literals_hold_static(advanced_static_ground_literals) if test_static else True
        holds = holds and self._literals_hold_fluent(advanced_fluent_ground_literals)
        holds = holds and self._literals_hold_derived(advanced_derived_ground_literals)
        return holds

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


class GroundConjunctiveCondition:
    _static_ground_literals: 'AdvancedStaticGroundLiteralList' = None
    _fluent_ground_literals: 'AdvancedFluentGroundLiteralList' = None
    _derived_ground_literals: 'AdvancedDerivedGroundLiteralList' = None
    _advanced_condition: 'AdvancedGroundConjunctiveCondition' = None

    def __init__(self, static_ground_literals: 'AdvancedStaticLiteralList' = None, fluent_ground_literals: 'AdvancedFluentLiteralList' = None, derived_ground_literals: 'AdvancedDerivedLiteralList' = None, advanced_condition: 'AdvancedGroundConjunctiveCondition' = None) -> None:
        """
        Initialize the ground conjunctive condition with the given parameters. It is not possible to create AdvancedGroundConjunctiveCondition directly, so this class is used as a wrapper.
        """
        if advanced_condition is not None:
            assert isinstance(advanced_condition, AdvancedGroundConjunctiveCondition), "Invalid advanced condition type."
            self._advanced_condition = advanced_condition
        else:
            assert static_ground_literals is not None and fluent_ground_literals is not None and derived_ground_literals is not None, "All literal lists must be provided if advanced_condition is not used."
            self._static_ground_literals = static_ground_literals
            self._fluent_ground_literals = fluent_ground_literals
            self._derived_ground_literals = derived_ground_literals

    def holds(self, state: 'State') -> bool:
        """Check if the ground conjunctive condition holds in the given state."""
        assert isinstance(state, State), "Invalid state type."
        if self._advanced_condition is not None:
            repositories = state._problem._advanced_problem.get_repositories()
            static_negative = repositories.get_static_ground_atoms_from_indices(list(self._advanced_condition.get_static_negative_condition()))
            static_positive = repositories.get_static_ground_atoms_from_indices(list(self._advanced_condition.get_static_positive_condition()))
            fluent_negative = repositories.get_fluent_ground_atoms_from_indices(list(self._advanced_condition.get_fluent_negative_condition()))
            fluent_positive = repositories.get_fluent_ground_atoms_from_indices(list(self._advanced_condition.get_fluent_positive_condition()))
            derived_negative = repositories.get_derived_ground_atoms_from_indices(list(self._advanced_condition.get_derived_negative_condition()))
            derived_positive = repositories.get_derived_ground_atoms_from_indices(list(self._advanced_condition.get_derived_positive_condition()))
            holds_positive = state.contains_all(static_positive) and state.contains_all(fluent_positive) and state.contains_all(derived_positive)
            holds_negative = state.contains_none(static_negative) and state.contains_none(fluent_negative) and state.contains_none(derived_negative)
            return holds_positive and holds_negative
        else:
            return state._literals_hold_static(self._static_ground_literals) and \
                   state._literals_hold_fluent(self._fluent_ground_literals) and \
                   state._literals_hold_derived(self._derived_ground_literals)

    def __iter__(self) -> 'Iterable[GroundLiteral]':
        """Iterate over all literals in the ground conjunctive condition."""
        if self._advanced_condition is not None:
            raise NotImplementedError("TODO.")
        else:
            yield from self._static_ground_literals
            yield from self._fluent_ground_literals
            yield from self._derived_ground_literals

    def __len__(self) -> 'int':
        """Get the number of literals in the ground conjunctive condition."""
        if self._advanced_condition is not None:
            raise NotImplementedError("TODO.")
        else:
            return len(self._static_ground_literals) + len(self._fluent_ground_literals) + len(self._derived_ground_literals)


class ConjunctiveCondition:
    _advanced_conjunctive_condition: 'AdvancedConjunctiveCondition' = None

    def __init__(self, advanced_condition: 'AdvancedConjunctiveCondition') -> None:
        assert isinstance(advanced_condition, AdvancedConjunctiveCondition), "Invalid conjunctive condition type."
        self._advanced_conjunctive_condition = advanced_condition

    def get_parameters(self) -> 'list[Variable]':
        """Get the parameters of the conjunctive condition."""
        return [Variable(x) for x in self._advanced_conjunctive_condition.get_parameters()]

    def get_literals(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[Literal]':
        """Get the literals of the conjunctive condition."""
        literals = []
        if not ignore_static:
            literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_static_literals()])
        if not ignore_fluent:
            literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_fluent_literals()])
        if not ignore_derived:
            literals.extend([Literal(x) for x in self._advanced_conjunctive_condition.get_derived_literals()])
        return literals

    def get_nullary_ground_literals(self, ignore_static = False, ignore_fluent = False, ignore_derived = False) -> 'list[GroundLiteral]':
        """Get the nullary ground literals of the conjunctive condition."""
        ground_literals = []
        if not ignore_static:
            ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_static_literals()])
        if not ignore_fluent:
            ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_fluent_literals()])
        if not ignore_derived:
            ground_literals.extend([GroundLiteral(x) for x in self._advanced_conjunctive_condition.get_nullary_ground_derived_literals()])
        return ground_literals

    def ground(self, state: 'State') -> 'Iterable[GroundConjunctiveCondition]':
        """Ground the conjunctive condition in the given state."""
        assert isinstance(state, State), "Invalid state type."
        # state._problem._advanced_problem.ground(self._advanced_conjunctive_condition, state._advanced_state)
        raise NotImplementedError("Grounding not implemented yet.")

    def __str__(self):
        """Get the string representation of the conjunctive condition."""
        return str(self._advanced_conjunctive_condition)

    def __repr__(self):
        """Get the string representation of the conjunctive condition."""
        return repr(self._advanced_conjunctive_condition)

    def __hash__(self) -> 'int':
        """Get the hash of the conjunctive condition."""
        return hash(self._advanced_conjunctive_condition)

    def __eq__(self, other: 'object') -> bool:
        """Check if two conjunctive conditions are equal."""
        if not isinstance(other, ConjunctiveCondition):
            return False
        return self._advanced_conjunctive_condition == other._advanced_conjunctive_condition


class ApplicableActionGenerator:
    """
    Class to generate applicable actions for a given state.
    """
    def __init__(self, problem: 'Problem'):
        self._problem = problem
        self._aag = problem._search_context.get_applicable_action_generator()

    def get_applicable_actions(self, state: 'State') -> 'list[GroundAction]':
        """
        Returns the applicable ground actions for the given state.
        """
        return [GroundAction(ground_action, self._problem) for ground_action in self._aag.generate_applicable_actions(state._advanced_state)]
