import pandas as pd
import matplotlib.pyplot as plt

def create_scaling_chart(csv_file="python_thread_scaling_results.csv"):
    """
    Reads Python thread scaling data and generates a line chart.
    """
    try:
        df = pd.read_csv(csv_file)
    except FileNotFoundError:
        print(f"Error: The file '{csv_file}' was not found.")
        print("Please run 'run_python_scaling_experiment.py' first.")
        return

    print("--- Python Thread Scaling Results ---")
    print(df)

    # --- Create the Line Chart ---
    plt.figure(figsize=(10, 6))
    
    plt.plot(df["Threads"], df["Average Time (s)"], marker='o', linestyle='-')

    plt.xlabel("Number of Threads")
    plt.ylabel("Average Time (s) (Lower is Better)")
    plt.title("Python Multi-threaded Performance vs. Thread Count")
    plt.grid(True, which="both", ls="--")
    
    # Use a logarithmic scale for the x-axis for better visualization of thread counts
    plt.xscale('log', base=2) 
    plt.xticks(df["Threads"], df["Threads"]) # Ensure all thread counts are shown as labels

    plt.tight_layout()

    output_image_file = "python_thread_scaling_chart.png"
    plt.savefig(output_image_file)
    print(f"\nLine chart saved as '{output_image_file}'")
    plt.show()

if __name__ == "__main__":
    create_scaling_chart()