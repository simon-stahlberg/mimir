import matplotlib.pyplot as plt
import json

from collections import defaultdict

MAX_MEMORY_MB = 20_000_000_000
MAX_AVG_NUM_STATE_VARIABLES = 200_000_000

def main():
    with open("combined-astar-blind-300-propositional-eval/properties", 'r') as file:
    # with open("combined-astar-blind-300-numeric-eval/properties", 'r') as file:
        data = json.load(file)

        task_to_runs = defaultdict(list)
        for task, run in data.items():
            task_to_runs[tuple(run["id"][1:])].append(run)

        count_compression_ratio_below_1 = 0

        X = []
        Y = []
        for task, runs in task_to_runs.items():
            list_peak_mem = None
            tree_peak_mem = None
            avg_num_state_variables = None
            for run in runs:
                if "average_num_state_variables" in run and avg_num_state_variables is None:
                    avg_num_state_variables = run["average_num_state_variables"] 

                if run["algorithm"] == "list-lifted-astar-eager-blind":
                    if "state_peak_memory_usage_in_bytes" in run:
                        list_peak_mem = run["state_peak_memory_usage_in_bytes"]
                    else:
                        list_peak_mem = MAX_MEMORY_MB
                elif run["algorithm"] == "dtdb-s-lifted-astar-eager-blind":
                    if "state_peak_memory_usage_in_bytes" in run:
                        tree_peak_mem = run["state_peak_memory_usage_in_bytes"]
                    else:
                        tree_peak_mem = MAX_MEMORY_MB

            if avg_num_state_variables is None:
                avg_num_state_variables = MAX_AVG_NUM_STATE_VARIABLES
            
            X.append(avg_num_state_variables)
            Y.append(list_peak_mem / tree_peak_mem)

            if list_peak_mem > tree_peak_mem and list_peak_mem != MAX_MEMORY_MB and tree_peak_mem != MAX_MEMORY_MB:
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