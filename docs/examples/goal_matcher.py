import argparse
import mimir
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
    domain = mimir.DomainParser(str(args.domain_path)).parse()
    problem = mimir.ProblemParser(str(args.problem_path)).parse(domain)
    time_start = time.time()
    successor_generator = mimir.GroundedSuccessorGenerator(problem)
    state_space = mimir.StateSpace.new(problem, successor_generator)
    time_end = time.time()
    print(f'StateSpace: {state_space.num_states()} [{time_end - time_start:.1f} seconds]')
    if state_space is None:
        print('Too many states. Aborting.')
        exit(1)
    # If you have a many different quantified goals over a specific state space, then the GoalMatcher is suitable for the task.
    goal_matcher = mimir.GoalMatcher(state_space)
    ground_atoms = problem.get_encountered_atoms()
    def get_random_atom():
        if random.randint(0, 1) > 0: return ground_atoms[random.randint(0, len(ground_atoms) - 1)]
        else: return domain.predicates[random.randint(0, len(domain.predicates) - 1)].as_atom()
    goal = [get_random_atom(), get_random_atom()]  # Construct your quantified goal here
    print(f'Quantified goal: {goal}')
    time_start = time.time()
    state, distance = goal_matcher.best_match(goal)
    time_end = time.time()
    print(f'Distance to closest state matching the goal: {distance}')
    print(f'State: {"-" if state is None else state.get_atoms()}')
    print(f'Time: {time_end - time_start:.5f} seconds')

if __name__ == '__main__':
    main()
