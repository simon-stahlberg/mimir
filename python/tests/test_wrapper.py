import unittest

from pathlib import Path
from pymimir import *


DATA_DIR = (Path(__file__).parent.parent.parent).absolute() / 'data'


class TestDomain(unittest.TestCase):
    def test_name(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        domain = Domain(domain_path)
        actual_name = domain.get_name()
        expected_name = 'blocksworld'
        assert actual_name == expected_name

    def test_requirements(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        domain = Domain(domain_path)
        actual_requirements = domain.get_requirements()
        expected_requirements = [':strips']
        assert len(actual_requirements) == len(expected_requirements)
        for requirement in actual_requirements:
            assert requirement in expected_requirements

    def test_predicates(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        domain = Domain(domain_path)
        actual_predicates = domain.get_predicates()
        expected_predicates = [('number', 1), ('object', 1), ('on', 2), ('clear', 1), ('holding', 1), ('on-table', 1), ('arm-empty', 0)]
        assert len(actual_predicates) == len(expected_predicates)
        for predicate in actual_predicates:
            assert predicate.get_index() is not None
            assert (predicate.get_name(), predicate.get_arity()) in expected_predicates

    def test_actions(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        domain = Domain(domain_path)
        actual_actions = domain.get_actions()
        expected_actions = [('pickup', 1), ('putdown', 1), ('stack', 2), ('unstack', 2)]
        assert len(actual_actions) == len(expected_actions)
        for action in actual_actions:
            assert action.get_index() is not None
            assert (action.get_name(), action.get_arity()) in expected_actions

    def test_constants(self):
        domain_path = DATA_DIR / 'woodworking' / 'domain.pddl'
        domain = Domain(domain_path)
        actual_constants = domain.get_constants()
        expected_constants = ['verysmooth', 'smooth', 'rough', 'varnished', 'glazed', 'untreated', 'colourfragments', 'natural', 'small', 'medium', 'large']
        assert len(actual_constants) == len(expected_constants)
        for constant in actual_constants:
            assert constant.get_index() is not None
            assert constant.get_name() in expected_constants


class TestProblem(unittest.TestCase):
    def test_name(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        actual_name = problem.get_name()
        expected_name = 'blocksworld-300'
        assert problem.get_index() is not None
        assert actual_name == expected_name

    # def test_requirements(self):
    #     domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
    #     problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
    #     domain = Domain(domain_path)
    #     problem = Problem(domain, problem_path)
    #     actual_requirements = problem.get_requirements()
    #     expected_requirements = [':strips', ':typing'] # TODO: ":typing" is missing in the requirements of the problem.
    #     assert len(actual_requirements) == len(expected_requirements)
    #     for requirement in actual_requirements:
    #         assert requirement in expected_requirements

    def test_objects(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        actual_objects = problem.get_objects()
        expected_objects = ['b1', 'b2', 'b3']
        assert len(actual_objects) == len(expected_objects)
        for obj in actual_objects:
            assert obj.get_index() is not None
            assert obj.get_name() in expected_objects

    def test_initial_atoms(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        actual_initial_atoms = problem.get_initial_atoms()
        expected_initial_atoms = ['(arm-empty)', '(clear b2)', '(on-table b2)', '(clear b1)', '(on b1 b3)', '(on-table b3)']
        assert len(actual_initial_atoms) == len(expected_initial_atoms)
        for initial_atom in actual_initial_atoms:
            assert initial_atom.get_index() is not None
            assert str(initial_atom) in expected_initial_atoms

    def test_new_atom_and_literal(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        predicate = domain.get_predicate('on')
        variables = problem.new_variable_list(['?v1', '?v2'])
        actual_new_atom = problem.new_atom(predicate, [variables[0], variables[1]])
        expected_new_atom = '(on ?v1 ?v2)'
        assert actual_new_atom.get_index() is not None
        assert str(actual_new_atom) == expected_new_atom
        actual_new_literal = problem.new_literal(actual_new_atom, False)
        expected_new_literal = '(not (on ?v1 ?v2))'
        assert actual_new_literal.get_index() is not None
        assert str(actual_new_literal) == expected_new_literal

    def test_new_ground_atom_and_ground_literal(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        predicate = domain.get_predicate('on')
        objects = [problem.get_object('b1'), problem.get_object('b2')]
        actual_new_ground_atom = problem.new_ground_atom(predicate, [objects[0], objects[1]])
        expected_new_ground_atom = '(on b1 b2)'
        assert actual_new_ground_atom.get_index() is not None
        assert str(actual_new_ground_atom) == expected_new_ground_atom
        actual_new_ground_literal = problem.new_ground_literal(actual_new_ground_atom, False)
        expected_new_ground_literal = '(not (on b1 b2))'
        assert actual_new_ground_literal.get_index() is not None
        assert str(actual_new_ground_literal) == expected_new_ground_literal

    def test_goal_condition(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        actual_goal_condition = problem.get_goal_condition()
        expected_goal_condition = ['(clear b2)', '(on b2 b3)', '(on-table b3)', '(clear b1)', '(on-table b1)']
        assert len(actual_goal_condition) == len(expected_goal_condition)
        for goal_literal in actual_goal_condition:
            assert goal_literal.get_polarity() is True
            assert str(goal_literal.get_atom()) in expected_goal_condition


class TestAction(unittest.TestCase):
    def test_precondition(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        domain = Domain(domain_path)
        action = domain.get_action('up')
        assert action.get_name() == 'up'
        actual_precondition = action.get_precondition()
        actual_parameters = actual_precondition.get_parameters()
        expected_parameters = ['?f1_0', '?f2_0']
        assert len(actual_parameters) == len(expected_parameters)
        for parameter in actual_parameters:
            assert parameter.get_index() is not None
            assert parameter.get_name() in expected_parameters
        actual_all_literals = actual_precondition.get_literals()
        expected_all_literals = ['(floor ?f1_0)', '(floor ?f2_0)', '(above ?f1_0 ?f2_0)', '(lift-at ?f1_0)', '(not (axiom_8))']
        assert len(actual_all_literals) == len(expected_all_literals)
        for literal in actual_all_literals:
            assert literal.get_index() is not None
            assert str(literal) in expected_all_literals
        actual_static_literals = actual_precondition.get_literals(False, True, True)
        expected_static_literals  = ['(floor ?f1_0)', '(floor ?f2_0)', '(above ?f1_0 ?f2_0)']
        assert len(actual_static_literals) == len(expected_static_literals)
        for literal in actual_static_literals:
            assert literal.get_index() is not None
            assert str(literal) in expected_static_literals
        actual_fluent_literals = actual_precondition.get_literals(True, False, True)
        expected_fluent_literals  = ['(lift-at ?f1_0)']
        assert len(actual_fluent_literals) == len(expected_fluent_literals)
        for literal in actual_fluent_literals:
            assert literal.get_index() is not None
            assert str(literal) in expected_fluent_literals
        actual_derived_literals = actual_precondition.get_literals(True, True, False)
        expected_derived_literals  = ['(not (axiom_8))']
        assert len(actual_derived_literals) == len(expected_derived_literals)
        for literal in actual_derived_literals:
            assert literal.get_index() is not None
            assert str(literal) in expected_derived_literals

    def test_effect(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        domain = Domain(domain_path)
        action = domain.get_action('up')
        assert action.get_name() == 'up'
        actual_effect = action.get_unconditional_effect()
        actual_parameters = actual_effect.get_parameters()
        expected_parameters = ['?f1_0', '?f2_0']
        assert len(actual_parameters) == len(expected_parameters)
        for parameter in actual_parameters:
            assert parameter.get_index() is not None
            assert parameter.get_name() in expected_parameters
        actual_literals = actual_effect.get_literals()
        expected_literals = ['(lift-at ?f2_0)', '(not (lift-at ?f1_0))']
        assert len(actual_literals) == len(expected_literals)
        for literal in actual_literals:
            assert literal.get_index() is not None
            assert str(literal) in expected_literals

    def test_conditional_effect(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        domain = Domain(domain_path)
        action = domain.get_action('stop')
        assert action.get_name() == 'stop'
        actual_conditiona_effect = action.get_conditional_effect()
        pass

    def test_new_conjunctive_condition(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        variables = problem.new_variable_list(['?a', '?b', '?c'])
        predicate_on = domain.get_predicate('on')
        atom_on_ab = problem.new_atom(predicate_on, [variables[0], variables[1]])
        atom_on_bc = problem.new_atom(predicate_on, [variables[1], variables[2]])
        literal_on_ab = problem.new_literal(atom_on_ab, True)
        literal_on_bc = problem.new_literal(atom_on_bc, True)
        literals = [literal_on_ab, literal_on_bc]
        conjunctive_condition = problem.new_conjunctive_condition(variables, literals)
        assert len(conjunctive_condition.get_parameters()) == len(variables)
        for parameter in conjunctive_condition.get_parameters():
            assert parameter.get_index() is not None
            assert parameter in variables
        assert len(conjunctive_condition.get_literals()) == len(literals)
        for literal in conjunctive_condition.get_literals():
            assert literal.get_index() is not None
            assert literal in literals


class TestState(unittest.TestCase):
    def test_get_initial_state(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        problem_path = DATA_DIR / 'miconic-fulladl' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        assert initial_state.get_index() is not None

    def test_get_ground_atoms(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        problem_path = DATA_DIR / 'miconic-fulladl' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        assert initial_state.get_index() is not None
        initial_atoms = initial_state.get_ground_atoms()
        initial_static_atoms = initial_state.get_ground_atoms(ignore_fluent=True, ignore_derived=True)
        initial_fluent_atoms = initial_state.get_ground_atoms(ignore_static=True, ignore_derived=True)
        initial_derived_atoms = initial_state.get_ground_atoms(ignore_static=True, ignore_fluent=True)
        assert len(initial_atoms) == 24
        assert len(initial_static_atoms) == 22
        assert len(initial_fluent_atoms) == 1
        assert len(initial_derived_atoms) == 1
        for atom in initial_atoms:
            assert atom.get_index() is not None
            assert isinstance(atom, GroundAtom)

    def test_contains(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        problem_path = DATA_DIR / 'miconic-fulladl' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        assert initial_state.contains_all(initial_state.get_ground_atoms())

    def test_literal_holds(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        problem_path = DATA_DIR / 'miconic-fulladl' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        aag = ApplicableActionGenerator(problem)
        actions = aag.get_applicable_actions(initial_state)
        assert len(actions) > 0
        for action in actions:
            assert action.get_precondition().holds(initial_state)


class TestGroundConjunctiveCondition(unittest.TestCase):
    def test_holds(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        problem_path = DATA_DIR / 'miconic-fulladl' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        assert not problem.get_goal_condition().holds(initial_state)


class TestApplicableActionGenerator(unittest.TestCase):
    def test_get_applicable_actions(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        aag = ApplicableActionGenerator(problem)
        actions = aag.get_applicable_actions(initial_state)
        assert len(actions) > 0
        for index, action in enumerate(actions):
            assert action.get_precondition().holds(initial_state)
            successor_state =  action.apply(initial_state)
            assert successor_state.get_index() == (index + 1)  # Index 0 is the initial state


if __name__ == '__main__':
    unittest.main()
