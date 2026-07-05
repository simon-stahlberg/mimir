from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import faulthandler
import threading

import pymimir.advanced.formalism as formalism
import pymimir.advanced.search as search


ROOT_DIR = (Path(__file__).parent.parent.parent.parent).absolute()
DOMAIN_FILEPATH = str(ROOT_DIR / "data" / "gripper" / "domain.pddl")
PROBLEM_FILEPATH = str(ROOT_DIR / "data" / "gripper" / "test_problem.pddl")
NUM_THREADS = 8
OUTER_ITERATIONS = 4
THREAD_TEST_TIMEOUT_SECONDS = 120


def _run_threaded(func, num_threads=NUM_THREADS, outer_iterations=OUTER_ITERATIONS):
    for _ in range(outer_iterations):
        barrier = threading.Barrier(num_threads)
        with ThreadPoolExecutor(max_workers=num_threads) as executor:
            futures = [executor.submit(func, barrier) for _ in range(num_threads)]
            for future in futures:
                future.result()


def _run_known_astar_solution():
    search_context = search.SearchContext.create(DOMAIN_FILEPATH, PROBLEM_FILEPATH, search.SearchContextOptions())
    grounder = search.LiftedGrounder(search_context.get_problem())
    heuristic = search.MaxHeuristic.create(grounder)
    result = search.find_solution_astar_eager(search_context, heuristic, search.AStarEagerOptions())

    assert result.status == search.SearchStatus.SOLVED
    assert len(result.plan) == 3


def test_independent_search_workloads_can_run_concurrently():
    def worker(barrier):
        barrier.wait()
        _run_known_astar_solution()

    faulthandler.dump_traceback_later(THREAD_TEST_TIMEOUT_SECONDS)
    try:
        _run_threaded(worker)
    finally:
        faulthandler.cancel_dump_traceback_later()


def test_shared_problem_read_only_access_can_run_concurrently():
    problem = formalism.Problem.create(DOMAIN_FILEPATH, PROBLEM_FILEPATH, formalism.ParserOptions())
    domain = problem.get_domain()

    expected_domain_name = "gripper-strips"
    expected_problem_name = "gripper-2"
    expected_action_names = sorted(action.get_name() for action in domain.get_actions())
    expected_object_names = sorted(obj.get_name() for obj in problem.get_objects())
    expected_initial_state = str(search.SearchContext.create(problem, search.SearchContextOptions()).get_state_repository().get_or_create_initial_state()[0])

    def worker(barrier):
        barrier.wait()
        assert domain.get_name() == expected_domain_name
        assert problem.get_name() == expected_problem_name
        assert sorted(action.get_name() for action in domain.get_actions()) == expected_action_names
        assert sorted(obj.get_name() for obj in problem.get_objects()) == expected_object_names

        search_context = search.SearchContext.create(problem, search.SearchContextOptions())
        initial_state, _ = search_context.get_state_repository().get_or_create_initial_state()
        assert str(initial_state) == expected_initial_state

    faulthandler.dump_traceback_later(THREAD_TEST_TIMEOUT_SECONDS)
    try:
        _run_threaded(worker)
    finally:
        faulthandler.cancel_dump_traceback_later()
