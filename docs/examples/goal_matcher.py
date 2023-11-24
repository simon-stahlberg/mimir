import argparse
import pymimir
import random
import time

from pathlib import Path

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('domain_path', type=Path, help='Path')
    parser.add_argument('problem_path', type=Path, help='Path')
    return parser.parse_args()

def main():
    args = parse_arguments()
    domain = pymimir.DomainParser(str(args.domain_path)).parse()
    problem = pymimir.ProblemParser(str(args.problem_path)).parse(domain)
    time_start = time.time()
    successor_generator = pymimir.GroundedSuccessorGenerator(problem)
    state_space = pymimir.StateSpace.new(problem, successor_generator)
    time_end = time.time()
    print(f'StateSpace: {state_space.num_states()} [{time_end - time_start:.1f} seconds]')
    if state_space is None:
        print('Too many states. Aborting.')
        exit(1)
    # If you have a many different quantified goals over a specific state space, then the GoalMatcher is suitable for the task.
    goal_matcher = pymimir.GoalMatcher(state_space)
    ground_atoms = problem.get_encountered_atoms()
    def get_random_atom():
        if random.randint(0, 1) > 0: return ground_atoms[random.randint(0, len(ground_atoms) - 1)]
        else: return domain.predicates[random.randint(0, len(domain.predicates) - 1)].as_atom()
    for i in range(1, 10):
        # We want to determine the distance to the closest state from the initial state.
        # It is possible to use "state_space.sample_state()," and Floyd-Warshall will be computed and cached for the state_space.
        # However, note that Floyd-Warshall is an O(n^3) algorithm and will likely take too much time and memory to be practical.
        # In this case, we suggest using "problem.replace_initial(...)" and computing a new state_space from the new initial state.
        from_state = state_space.get_initial_state()
        goal = [get_random_atom(), get_random_atom()]  # Construct your quantified goal here
        # Use "atom.replace_term(...)" to construct new atoms, e.g. partially quantified atoms.
        print(f'[{i}] -----')
        print(f'[{i}] Quantified goal: {goal}')
        time_start = time.time()
        to_state, distance = goal_matcher.best_match(from_state, goal)
        time_end = time.time()
        print(f'[{i}] Distance to closest state matching the goal: {distance}')
        print(f'[{i}] State: {"-" if to_state is None else to_state.get_atoms()}')
        print(f'[{i}] Time: {time_end - time_start:.5f} seconds')

if __name__ == '__main__':
    main()
