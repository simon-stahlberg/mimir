from argparse import ArgumentParser

from mimir import LiftedFFHeuristic, Problem, astar


parser = ArgumentParser()
parser.add_argument("domain")
parser.add_argument("problem")
args = parser.parse_args()

problem = Problem.from_files(args.domain, args.problem)
result = astar(problem, LiftedFFHeuristic(problem), timeout_seconds=30)

print(result.status.value)
if result.solution is not None:
    for action in result.solution.plan:
        print(action, action.cost)
    print("cost:", result.solution.cost)
