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

    def test_str_repr_hash(self):
        domain_path = DATA_DIR / 'woodworking' / 'domain.pddl'
        domain = Domain(domain_path)
        assert str(domain) is not None
        assert repr(domain) is not None
        assert hash(domain) is not None


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

    def test_numeric_goal_condition(self):
        domain_path = DATA_DIR / 'refuel' / 'domain.pddl'
        problem_path = DATA_DIR / 'refuel' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        actual_numeric_goal_condition = problem.get_numeric_goal_condition()
        pass

    def test_str_repr_hash(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        assert str(problem) is not None
        assert repr(problem) is not None
        assert hash(problem) is not None


class TestAction(unittest.TestCase):
    def test_precondition(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        domain = Domain(domain_path)
        action = domain.get_action('up')
        assert action.get_name() == 'up'
        actual_precondition = action.get_precondition()
        actual_parameters = actual_precondition.get_parameters()
        expected_parameters = ['?f1_0_0', '?f2_0_1']
        assert len(actual_parameters) == len(expected_parameters)
        for parameter in actual_parameters:
            assert parameter.get_index() is not None
            assert parameter.get_name() in expected_parameters
        actual_all_literals = actual_precondition.get_literals()
        expected_all_literals = ['(floor ?f1_0_0)', '(floor ?f2_0_1)', '(above ?f1_0_0 ?f2_0_1)', '(lift-at ?f1_0_0)', '(not (axiom_8))']
        assert len(actual_all_literals) == len(expected_all_literals)
        for literal in actual_all_literals:
            assert literal.get_index() is not None
            assert str(literal) in expected_all_literals
        actual_static_literals = actual_precondition.get_literals(False, True, True)
        expected_static_literals  = ['(floor ?f1_0_0)', '(floor ?f2_0_1)', '(above ?f1_0_0 ?f2_0_1)']
        assert len(actual_static_literals) == len(expected_static_literals)
        for literal in actual_static_literals:
            assert literal.get_index() is not None
            assert str(literal) in expected_static_literals
        actual_fluent_literals = actual_precondition.get_literals(True, False, True)
        expected_fluent_literals  = ['(lift-at ?f1_0_0)']
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

    def test_unconditional_effect(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        domain = Domain(domain_path)
        action = domain.get_action('up')
        assert action.get_name() == 'up'
        actual_conditional_effect = action.get_conditional_effect()
        assert len(actual_conditional_effect) == 1
        actual_condition = actual_conditional_effect[0].get_condition()
        assert len(actual_condition.get_parameters()) == 0
        assert len(actual_condition.get_literals()) == 0
        assert len(actual_condition.get_nullary_ground_literals()) == 0
        actual_effect = actual_conditional_effect[0].get_effect()
        actual_parameters = actual_effect.get_parameters()
        expected_parameters = ['?f1_0_0', '?f2_0_1']
        assert len(actual_parameters) == len(expected_parameters)
        for parameter in actual_parameters:
            assert parameter.get_index() is not None
            assert parameter.get_name() in expected_parameters
        actual_literals = actual_effect.get_literals()
        expected_literals = ['(lift-at ?f2_0_1)', '(not (lift-at ?f1_0_0))']
        assert len(actual_literals) == len(expected_literals)
        for literal in actual_literals:
            assert literal.get_index() is not None
            assert str(literal) in expected_literals

    def test_conditional_effect(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        domain = Domain(domain_path)
        action = domain.get_action('stop')
        assert action.get_name() == 'stop'
        actual_conditional_effect = action.get_conditional_effect()
        assert len(actual_conditional_effect) == 2
        # First conditional effect
        actual_first_condition = actual_conditional_effect[0].get_condition()
        assert len(actual_first_condition.get_parameters()) == 1
        assert len(actual_first_condition.get_literals()) == 3
        assert len(actual_first_condition.get_nullary_ground_literals()) == 0
        # Second conditional effect
        actual_second_condition = actual_conditional_effect[1].get_condition()
        assert len(actual_second_condition.get_parameters()) == 1
        assert len(actual_second_condition.get_literals()) == 3
        assert len(actual_second_condition.get_nullary_ground_literals()) == 0

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

    def test_str_repr_hash(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        domain = Domain(domain_path)
        action = domain.get_action('pickup')
        assert str(action) is not None
        assert repr(action) is not None
        assert hash(action) is not None


class TestState(unittest.TestCase):
    def test_get_initial_state(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        problem_path = DATA_DIR / 'miconic-fulladl' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        assert initial_state.get_index() is not None

    def test_get_atoms(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        problem_path = DATA_DIR / 'miconic-fulladl' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        assert initial_state.get_index() is not None
        initial_atoms = initial_state.get_atoms()
        initial_static_atoms = initial_state.get_atoms(ignore_fluent=True, ignore_derived=True)
        initial_fluent_atoms = initial_state.get_atoms(ignore_static=True, ignore_derived=True)
        initial_derived_atoms = initial_state.get_atoms(ignore_static=True, ignore_fluent=True)
        assert len(initial_atoms) == 28
        assert len(initial_static_atoms) == 22
        assert len(initial_fluent_atoms) == 1
        assert len(initial_derived_atoms) == 5
        for atom in initial_atoms:
            assert atom.get_index() is not None
            assert isinstance(atom, GroundAtom)

    def test_contains(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        problem_path = DATA_DIR / 'miconic-fulladl' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        assert initial_state.contains_all(initial_state.get_atoms())

    def test_literal_holds(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        problem_path = DATA_DIR / 'miconic-fulladl' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        actions = initial_state.generate_applicable_actions()
        assert len(actions) > 0
        for action in actions:
            assert action.get_precondition().holds(initial_state)

    def test_generate_applicable_actions(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        actions = initial_state.generate_applicable_actions()
        assert len(actions) > 0
        for index, action in enumerate(actions):
            assert action.get_precondition().holds(initial_state)
            successor_state =  action.apply(initial_state)
            assert successor_state.get_index() == (index + 1)  # Index 0 is the initial state

    def test_str_repr_hash(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        assert str(initial_state) is not None
        assert repr(initial_state) is not None
        assert hash(initial_state) is not None


class TestGroundConjunctiveCondition(unittest.TestCase):
    def test_holds(self):
        domain_path = DATA_DIR / 'miconic-fulladl' / 'domain.pddl'
        problem_path = DATA_DIR / 'miconic-fulladl' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        assert not problem.get_goal_condition().holds(initial_state)

    def test_grounder(self):
        domain_path = DATA_DIR / 'gripper' / 'domain.pddl'
        problem_path = DATA_DIR / 'gripper' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        pickup_condition = domain.get_action('pick').get_precondition()
        all_groundings = pickup_condition.ground(initial_state)
        single_grounding = pickup_condition.ground(initial_state, 1)
        assert len(all_groundings) == 4
        assert len(single_grounding) == 1

    def test_grounder_blacklist(self):
        domain_path = DATA_DIR / 'gripper' / 'domain.pddl'
        problem_path = DATA_DIR / 'gripper' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        pickup_condition = domain.get_action('pick').get_precondition()
        blacklist = [domain.get_predicate('ball')]
        all_groundings = pickup_condition.ground(initial_state, blacklist=blacklist)
        assert len(all_groundings) == 4
        assert len(all_groundings[0]) == 5

    def test_lift(self):
        domain_path = DATA_DIR / 'gripper' / 'domain.pddl'
        problem_path = DATA_DIR / 'gripper' / 'test_problem2.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        grounded_goal = problem.get_goal_condition()
        lifted_goal = grounded_goal.lift()
        assert len(lifted_goal.get_parameters()) == 3
        assert len(lifted_goal.get_literals()) == 2
        assert lifted_goal.get_literals()[0].get_atom().get_terms()[0] == lifted_goal.get_parameters()[0]
        assert lifted_goal.get_literals()[0].get_atom().get_terms()[1] == lifted_goal.get_parameters()[1]
        assert lifted_goal.get_literals()[1].get_atom().get_terms()[0] == lifted_goal.get_parameters()[2]
        assert lifted_goal.get_literals()[1].get_atom().get_terms()[1] == lifted_goal.get_parameters()[1]

    def test_lift_inequalities(self):
        domain_path = DATA_DIR / 'hiking' / 'domain.pddl'
        problem_path = DATA_DIR / 'hiking' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        grounded_goal = problem.get_goal_condition()
        lifted_goal_with_inequalities = grounded_goal.lift(True)
        assert len(lifted_goal_with_inequalities.get_parameters()) == 2
        assert len(lifted_goal_with_inequalities.get_literals()) == 2
        lifted_goal_without_inequalities = grounded_goal.lift(False)
        assert len(lifted_goal_without_inequalities.get_parameters()) == 2
        assert len(lifted_goal_without_inequalities.get_literals()) == 1

    def test_new_grounded_condition(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        ground_literals = [GroundLiteral.new(atom, True, problem) for atom in problem.get_initial_atoms(ignore_static=True, ignore_derived=True)]
        ground_condition = GroundConjunctiveCondition.new(ground_literals, problem)
        assert ground_condition.holds(problem.get_initial_state())

    def test_str_repr_hash(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        ground_literals = [GroundLiteral.new(atom, True, problem) for atom in problem.get_initial_atoms(ignore_static=True, ignore_derived=True)]
        ground_condition = GroundConjunctiveCondition.new(ground_literals, problem)
        assert str(ground_condition) is not None
        assert repr(ground_condition) is not None
        assert hash(ground_condition) is not None


class TestSearchAlgorithms(unittest.TestCase):
    def test_custom_heuristic(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path, mode='lifted')
        initial_state = problem.get_initial_state()
        class CustomHeuristic(Heuristic):
            def compute_value(self, state: State, is_goal_state: bool) -> float:
                return 0.0 if is_goal_state else 1.0
        heuristic = CustomHeuristic()
        result = astar_eager(problem, initial_state, heuristic)
        assert result.status == "solved"
        assert result.solution is not None
        assert len(result.solution) == 4
        assert result.solution_cost == 4.0

    def test_astar_eager(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        heuristic = BlindHeuristic(problem)
        result = astar_eager(problem, initial_state, heuristic)
        assert result.status == "solved"
        assert result.solution is not None
        assert len(result.solution) == 4
        assert result.solution_cost == 4.0

    def test_astar_eager_events(self):
        domain_path = DATA_DIR / 'childsnack' / 'domain.pddl'
        problem_path = DATA_DIR / 'childsnack' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        heuristic = BlindHeuristic(problem)
        expanded_goal_states = []
        expanded_states = []
        finished_f_layers = []
        generated_states = []
        pruned_states = []
        _ = astar_eager(problem,
                        initial_state,
                        heuristic,
                        on_expand_state=lambda state: expanded_states.append(state),
                        on_expand_goal_state=lambda state: expanded_goal_states.append(state),
                        on_generate_state=lambda state, action, cost, new_state: generated_states.append((state, action, cost, new_state)),
                        on_prune_state=lambda state: pruned_states.append(state),
                        on_finish_f_layer=lambda f_value: finished_f_layers.append(f_value))
        assert len(expanded_goal_states) == 1
        assert len(expanded_states) == 6
        assert len(finished_f_layers) == 4
        assert len(generated_states) == 16
        assert len(pruned_states) == 0

    def test_set_add_heuristic(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        heuristic = SetAddHeuristic(problem)
        initial_state = problem.get_initial_state()
        assert heuristic.compute_value(initial_state, True) == 0.0
        assert heuristic.compute_value(initial_state, False) == 5.0
        assert len(heuristic.get_preferred_actions()) == 0

    def test_max_heuristic(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        heuristic = MaxHeuristic(problem)
        initial_state = problem.get_initial_state()
        assert heuristic.compute_value(initial_state, True) == 0.0
        assert heuristic.compute_value(initial_state, False) == 2.0
        assert len(heuristic.get_preferred_actions()) == 0

    def test_add_heuristic(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        heuristic = MaxHeuristic(problem)
        initial_state = problem.get_initial_state()
        assert heuristic.compute_value(initial_state, True) == 0.0
        assert heuristic.compute_value(initial_state, False) == 2.0
        assert len(heuristic.get_preferred_actions()) == 0

    def test_perfect_heuristic(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        heuristic = PerfectHeuristic(problem)
        initial_state = problem.get_initial_state()
        # assert heuristic.compute_value(initial_state, True) == 0.0  # TODO: PerfectHeuristic seems to ignore the goal flag.
        assert heuristic.compute_value(initial_state, False) == 4.0
        assert len(heuristic.get_preferred_actions()) == 0

    def test_ff_heuristic(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        heuristic = FFHeuristic(problem)
        initial_state = problem.get_initial_state()
        assert heuristic.compute_value(initial_state, True) == 0.0
        assert heuristic.compute_value(initial_state, False) == 4.0
        assert len(heuristic.get_preferred_actions()) == 2

    def test_iw(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        result = iw(problem, initial_state, 1)
        assert result.status == "failed"
        assert result.solution is None
        assert result.solution_cost is None
        assert result.goal_state is None

    def test_iw_events(self):
        domain_path = DATA_DIR / 'childsnack' / 'domain.pddl'
        problem_path = DATA_DIR / 'childsnack' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        initial_state = problem.get_initial_state()
        expanded_states = []
        expanded_goal_states = []
        generated_states = []
        generated_new_states = []
        pruned_states = []
        _ = iw(problem,
               initial_state,
               1,
               on_expand_state=lambda state: expanded_states.append(state),
               on_expand_goal_state=lambda state: expanded_goal_states.append(state),
               on_generate_state=lambda state, action, cost, successor_state: generated_states.append((state, action, cost, successor_state)),
               on_generate_new_state=lambda state, action, cost, successor_state: generated_new_states.append((state, action, cost, successor_state)),
               on_prune_state=lambda state, action, cost, successor_state: pruned_states.append((state, action, cost, successor_state)))
        assert len(expanded_states) == 7
        assert len(expanded_goal_states) == 0
        assert len(generated_states) == 20
        assert len(generated_new_states) == 5
        assert len(pruned_states) == 15

    def test_str_repr_hash(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        heuristic = FFHeuristic(problem)
        assert str(heuristic) is not None
        assert repr(heuristic) is not None
        assert hash(heuristic) is not None


class TestStateSpaceSampler(unittest.TestCase):
    def test_sample_state(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        sampler = StateSpaceSampler.new(problem)
        assert isinstance(sampler, StateSpaceSampler)
        sampler.set_seed(0)
        assert sampler is not None
        state = sampler.sample_state()
        assert state is not None
        state_label = sampler.get_state_label(state)
        assert state_label is not None
        assert isinstance(state_label, StateLabel)
        assert state_label.steps_to_goal >= 0
        assert isinstance(state_label.is_initial, bool)
        assert isinstance(state_label.is_goal, bool)
        assert isinstance(state_label.is_dead_end, bool)
        labeled_states = list(sampler.sample_states(10))
        assert len(labeled_states) == 10

    def test_forward_transitions(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        sampler = StateSpaceSampler.new(problem)
        assert isinstance(sampler, StateSpaceSampler)
        sampler.set_seed(0)
        assert sampler is not None
        state = sampler.sample_state_n_steps_from_goal(1)
        assert state is not None
        state_label = sampler.get_state_label(state)
        assert state_label is not None
        assert state_label.steps_to_goal == 1
        forward_transitions = list(sampler.get_forward_transitions(state))
        assert len(forward_transitions) > 0
        for action, successor_state in forward_transitions:
            assert isinstance(action, GroundAction)
            assert isinstance(successor_state, State)
            assert action.get_index() is not None
            assert successor_state.get_index() is not None
            assert action.apply(state) == successor_state

    def test_backward_transitions(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        sampler = StateSpaceSampler.new(problem)
        assert isinstance(sampler, StateSpaceSampler)
        sampler.set_seed(0)
        assert sampler is not None
        state = sampler.sample_state_n_steps_from_goal(2)
        assert state is not None
        state_label = sampler.get_state_label(state)
        assert state_label is not None
        assert state_label.steps_to_goal == 2
        backward_transitions = list(sampler.get_backward_transitions(state))
        assert len(backward_transitions) > 0
        for action, predecessor_state in backward_transitions:
            assert isinstance(action, GroundAction)
            assert isinstance(predecessor_state, State)
            assert action.get_index() is not None
            assert predecessor_state.get_index() is not None
            assert action.apply(predecessor_state) == state

    def test_sample_dead_end_state(self):
        domain_path = DATA_DIR / 'spanner' / 'domain.pddl'
        problem_path = DATA_DIR / 'spanner' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        sampler = StateSpaceSampler.new(problem)
        assert isinstance(sampler, StateSpaceSampler)
        sampler.set_seed(0)
        assert sampler is not None
        dead_end_state = sampler.sample_dead_end_state()
        assert dead_end_state is not None
        state_label = sampler.get_state_label(dead_end_state)
        assert state_label is not None
        assert state_label.is_dead_end is True
        assert state_label.steps_to_goal >= 2_000_000_000
        dead_end_states = list(sampler.sample_dead_end_states(10))
        assert len(dead_end_states) == 10

    def test_str_repr_hash(self):
        domain_path = DATA_DIR / 'blocks_4' / 'domain.pddl'
        problem_path = DATA_DIR / 'blocks_4' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        sampler = StateSpaceSampler.new(problem)
        assert sampler is not None
        assert str(sampler) is not None
        assert repr(sampler) is not None
        assert hash(sampler) is not None

    def test_get_state(self):
        domain_path = DATA_DIR / 'spanner' / 'domain.pddl'
        problem_path = DATA_DIR / 'spanner' / 'test_problem.pddl'
        domain = Domain(domain_path)
        problem = Problem(domain, problem_path)
        sampler = StateSpaceSampler.new(problem)
        assert sampler is not None
        states = sampler.get_states()
        for state in states:
            expected_index = state.get_index()
            actual_index = sampler.get_state(expected_index).get_index()
            assert actual_index == expected_index

if __name__ == '__main__':
    unittest.main()
