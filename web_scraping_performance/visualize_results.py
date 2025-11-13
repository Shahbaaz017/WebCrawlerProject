import pandas as pd
import matplotlib.pyplot as plt

def create_performance_chart(csv_file="results.csv"):
    """
    Reads performance data from a CSV file and generates a bar chart.
    """
    try:
        # Read the CSV data into a pandas DataFrame
        df = pd.read_csv(csv_file)
    except FileNotFoundError:
        print(f"Error: The file '{csv_file}' was not found.")
        print("Please run 'run_experiments.py' first to generate the results.")
        return

    # Sort the DataFrame by 'Average Time (s)' to make the chart easier to read
    df_sorted = df.sort_values(by="Average Time (s)")

    print("--- Performance Results (sorted fastest to slowest) ---")
    print(df_sorted)

    # --- Create the Bar Chart ---
    # Set the size of the plot
    plt.figure(figsize=(12, 7))

    # Create the bar plot
    plt.bar(df_sorted["Experiment"], df_sorted["Average Time (s)"], color='skyblue')

    # Add titles and labels for clarity
    plt.ylabel("Average Time (s)")
    plt.title("Web Scraper Performance Comparison")
    
    # Rotate the x-axis labels to prevent them from overlapping
    plt.xticks(rotation=45, ha="right") 
    
    # Add a grid for better readability
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    
    # Ensure the layout is tight so labels don't get cut off
    plt.tight_layout()

    # Save the chart to a file
    output_image_file = "performance_comparison_chart.png"
    plt.savefig(output_image_file)
    print(f"\nChart saved as '{output_image_file}'")

    # Display the chart in a window
    plt.show()


if __name__ == "__main__":
    create_performance_chart()