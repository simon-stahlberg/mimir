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
    def get_random_atom(): return domain.predicates[random.randint(0, len(domain.predicates) - 1)].as_atom()
    quantified_goal = [get_random_atom(), get_random_atom()]  # Construct your quantified goal here
    print(f'Goal: {quantified_goal}')
    print(f'State: {problem.initial}')
    time_start = time.time()
    # If you have a specific quantified goal in mind and many different states, then the LiteralGround is suitable for the task.
    literal_grounder = pymimir.LiteralGrounder(problem, quantified_goal)
    bindings = literal_grounder.ground(problem.create_state(problem.initial))
    time_end = time.time()
    print(f'# ground goals: {len(bindings)} [{time_end - time_start:.8f} seconds]')
    for ground_goal, binding in bindings:
        print(f'Ground goal: {ground_goal}, binding: {binding}')

if __name__ == '__main__':
    main()
