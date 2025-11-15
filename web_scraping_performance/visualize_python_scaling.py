import pandas as pd
import matplotlib.pyplot as plt

def create_scaling_charts(csv_file="python_thread_scaling_results.csv"):
    """
    Reads Python thread scaling data and generates two charts:
    1. A dual-axis line chart for Time vs. Speedup.
    2. A bar chart for Memory Usage.
    """
    try:
        df = pd.read_csv(csv_file)
    except FileNotFoundError:
        print(f"Error: The file '{csv_file}' was not found.")
        print("Please run 'run_python_scaling_experiment.py' first.")
        return

    print("--- Python Thread Scaling Results ---")
    print(df)

    # --- Chart 1: Time vs. Speedup (Dual-Axis Line Chart) ---
    fig, ax1 = plt.subplots(figsize=(12, 7))

    # Plot Average Time on the primary (left) y-axis
    color = 'tab:blue'
    ax1.set_xlabel('Number of Threads')
    ax1.set_ylabel('Average Time (s) (Lower is Better)', color=color)
    ax1.plot(df["Threads"], df["Average Time (s)"], color=color, marker='o', label='Execution Time')
    ax1.tick_params(axis='y', labelcolor=color)
    ax1.grid(True, which="both", ls="--")

    # Use a logarithmic scale for the x-axis for better visualization
    ax1.set_xscale('log', base=2)
    ax1.set_xticks(df["Threads"])
    ax1.get_xaxis().set_major_formatter(plt.ScalarFormatter())

    # Create a second y-axis (right) that shares the same x-axis
    ax2 = ax1.twinx()
    color = 'tab:red'
    ax2.set_ylabel('Speedup (Higher is Better)', color=color)
    ax2.plot(df["Threads"], df["Speedup"], color=color, marker='s', linestyle='--', label='Speedup Factor')
    ax2.tick_params(axis='y', labelcolor=color)

    fig.suptitle('Python Multi-threaded Performance vs. Thread Count', fontsize=16)
    fig.tight_layout(rect=[0, 0, 1, 0.96]) # Adjust layout to make room for suptitle

    # Add a single legend for both lines
    lines, labels = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax2.legend(lines + lines2, labels + labels2, loc='upper center')
    
    output_file_1 = "python_time_vs_speedup_chart.png"
    plt.savefig(output_file_1)
    print(f"\nTime vs. Speedup chart saved as '{output_file_1}'")
    plt.show()


    # --- Chart 2: Memory Usage (Bar Chart) ---
    plt.figure(figsize=(10, 6))
    plt.bar(df["Threads"].astype(str), df["Peak Memory (MB)"], color='seagreen')
    plt.xlabel("Number of Threads")
    plt.ylabel("Peak Memory Usage (MB)")
    plt.title("Peak Memory Usage vs. Number of Threads")
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()
    
    output_file_2 = "python_memory_usage_chart.png"
    plt.savefig(output_file_2)
    print(f"Memory usage chart saved as '{output_file_2}'")
    plt.show()


if __name__ == "__main__":
    create_scaling_charts()