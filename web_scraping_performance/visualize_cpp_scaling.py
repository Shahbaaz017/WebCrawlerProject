import pandas as pd
import matplotlib.pyplot as plt

def create_scaling_charts(csv_file="cpp_openmp_scaling_results.csv"):
    """
    Reads C++ OpenMP thread scaling data and generates charts for
    Time vs. Speedup and Memory Usage.
    """
    try:
        df = pd.read_csv(csv_file)
    except FileNotFoundError:
        print(f"Error: The file '{csv_file}' was not found.")
        print("Please run 'run_cpp_scaling_experiment.py' first.")
        return

    print("--- C++ OpenMP Thread Scaling Results ---")
    print(df)

    # --- Chart 1: Time vs. Speedup (Dual-Axis Line Chart) ---
    fig, ax1 = plt.subplots(figsize=(12, 7))

    color = 'tab:blue'
    ax1.set_xlabel('Number of Threads (OMP_NUM_THREADS)')
    ax1.set_ylabel('Average Time (s) (Lower is Better)', color=color)
    ax1.plot(df["Threads"], df["Average Time (s)"], color=color, marker='o', label='Execution Time')
    ax1.tick_params(axis='y', labelcolor=color)
    ax1.grid(True, which="both", ls="--")
    ax1.set_xticks(df["Threads"])
    ax1.get_xaxis().set_major_formatter(plt.ScalarFormatter())

    ax2 = ax1.twinx()
    color = 'tab:red'
    ax2.set_ylabel('Speedup (Higher is Better)', color=color)
    ax2.plot(df["Threads"], df["Speedup"], color=color, marker='s', linestyle='--', label='Speedup Factor')
    ax2.tick_params(axis='y', labelcolor=color)

    fig.suptitle('C++ OpenMP Performance vs. Thread Count', fontsize=16)
    fig.tight_layout(rect=[0, 0, 1, 0.96])
    lines, labels = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax2.legend(lines + lines2, labels + labels2, loc='upper center')
    
    output_file_1 = "cpp_time_vs_speedup_chart.png"
    plt.savefig(output_file_1)
    print(f"\nTime vs. Speedup chart saved as '{output_file_1}'")
    plt.show()

    # --- Chart 2: Memory Usage (Bar Chart) ---
    plt.figure(figsize=(10, 6))
    plt.bar(df["Threads"].astype(str), df["Peak Memory (MB)"], color='darkred')
    plt.xlabel("Number of Threads (OMP_NUM_THREADS)")
    plt.ylabel("Peak Memory Usage (MB)")
    plt.title("C++ OpenMP Peak Memory Usage vs. Number of Threads")
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()
    
    output_file_2 = "cpp_memory_usage_chart.png"
    plt.savefig(output_file_2)
    print(f"Memory usage chart saved as '{output_file_2}'")
    plt.show()

if __name__ == "__main__":
    create_scaling_charts()