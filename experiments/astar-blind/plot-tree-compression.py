import matplotlib.pyplot as plt
import json

from collections import defaultdict

def main():
    # with open("combined-astar-blind-30-propositional-eval/properties", 'r') as file:
    with open("combined-astar-blind-30-numeric-eval/properties", 'r') as file:
        data = json.load(file)

        task_to_runs = defaultdict(list)
        for task, run in data.items():
            task_to_runs[tuple(run["id"][1:])].append(run)

        task_to_all_solved_runs = dict()
        for task, runs in task_to_runs.items():
            if all(run["coverage"] for run in runs):
                task_to_all_solved_runs[task] = runs

        count_compression_ratio_below_1 = 0

        X = []
        Y = []
        for task, runs in task_to_all_solved_runs.items():
            list_peak_mem = None
            tree_peak_mem = None
            list_avg_num_state_variables = None
            for run in runs:
                if run["algorithm"] == "list-lifted-astar-eager-blind":
                    list_peak_mem = run["state_peak_memory_usage_in_bytes"]
                    list_avg_num_state_variables = run["average_num_state_variables"] 
                elif run["algorithm"] == "tree-swiss-lifted-astar-eager-blind":
                    tree_peak_mem = run["state_peak_memory_usage_in_bytes"]

            assert(list_peak_mem is not None and tree_peak_mem is not None and list_avg_num_state_variables is not None)
            X.append(list_avg_num_state_variables)
            Y.append(tree_peak_mem / list_peak_mem)

            if tree_peak_mem / list_peak_mem < 1:
                count_compression_ratio_below_1 += 1

        print(" ".join([f"({x:.4f}, {y:.4f})" for x, y in zip(X, Y)]))
        print(count_compression_ratio_below_1)

        plt.scatter(X, Y)
        plt.xlabel("Avg. #state atoms")
        plt.ylabel("Compression ratio")
        plt.title("Scatterplot of Structural vs Memory Efficiency")
        plt.grid(True)
        plt.savefig("scatterplot.png", dpi=300, bbox_inches='tight')

if __name__ == "__main__":
    main()
