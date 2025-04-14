import unittest

from pathlib import Path
from wrapper import *


DATA_DIR = (Path(__file__).parent.parent.parent.parent).absolute() / 'data'


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
    #     expected_requirements = [':strips', ':typing'] # TODO: Dominik -- :typing is missing in the requirements of the problem.
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
        actual_initial_atoms = problem.get_initial_atoms(skip_derived=True)  # TODO: Dominik -- Crashes with skip_derived=False
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

    # def test_goal_condition(self):
    #     domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
    #     problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
    #     domain = Domain(domain_path)
    #     problem = Problem(domain, problem_path)
    #     actual_goal_condition = problem.get_goal_condition()  # TODO: Dominik -- Not implemented.
    #     expected_goal_condition = ['(clear b2)', '(on b2 b3)', '(on-table b3)', '(clear b1)', '(on-table b1)']
    #     assert len(actual_goal_condition) == len(expected_goal_condition)
    #     for goal_literal in actual_goal_condition:
    #         assert goal_literal.get_polarity() is True
    #         assert str(goal_literal.get_atom()) in expected_goal_condition


class TestAction(unittest.TestCase):
    def test_precondition(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        domain = Domain(domain_path)
        action = domain.get_action('up')
        assert action.get_name() == 'up'
        actual_precondition = action.get_conjunctive_condition()
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
        actual_effect = action.get_conjunctive_effect()
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



if __name__ == '__main__':
    unittest.main()
