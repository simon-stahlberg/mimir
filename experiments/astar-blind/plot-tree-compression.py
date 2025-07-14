import matplotlib.pyplot as plt
import json

from collections import defaultdict

def main():
    # with open("filtered-astar-blind-30-propositional-eval/properties", 'r') as file:
    with open("filtered-astar-blind-30-numeric-eval/properties", 'r') as file:
        data = json.load(file)

        task_to_runs = defaultdict(list)
        for task, run in data.items():
            task_to_runs[tuple(run["id"][1:])].append(run)

        task_to_all_solved_runs = dict()
        for task, runs in task_to_runs.items():
            if all(run["coverage"] for run in runs):
                task_to_all_solved_runs[task] = runs

        X = []
        Y = []
        for task, runs in task_to_all_solved_runs.items():
            list_peak_mem = None
            tree_peak_mem = None
            list_avg_num_state_atoms = None
            tree_avg_num_slots = None
            for run in runs:
                if run["algorithm"] == "list-lifted-astar-eager-blind":
                    list_peak_mem = run["peak_memory_usage_in_bytes"]
                    list_avg_num_state_atoms = run["average_num_state_atoms"] 
                elif run["algorithm"] == "tree-lifted-astar-eager-blind":
                    tree_peak_mem = run["peak_memory_usage_in_bytes"]
                    tree_avg_num_slots = run["average_num_index_slots_per_state"] + run["average_num_double_slots_per_state"]

            X.append(list_avg_num_state_atoms / tree_avg_num_slots)
            Y.append(list_peak_mem / tree_peak_mem)

        print(" ".join([f"({x:.4f}, {y:.4f})" for x, y in zip(X, Y)]))

        plt.scatter(X, Y)
        plt.xlabel("List avg. #state atoms / Tree avg. #slots")
        plt.ylabel("List peak mem / Tree peak mem")
        plt.title("Scatterplot of Structural vs Memory Efficiency")
        plt.grid(True)
        plt.savefig("scatterplot.png", dpi=300, bbox_inches='tight')

if __name__ == "__main__":
    main()
