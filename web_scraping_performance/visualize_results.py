import pandas as pd
import matplotlib.pyplot as plt

def create_performance_chart(csv_file="main_comparison_results.csv"):
    try:
        df = pd.read_csv(csv_file)
    except FileNotFoundError:
        print(f"Error: The file '{csv_file}' was not found.")
        return

    df_sorted = df.sort_values(by="Average Time (s)")
    print("--- Main Comparison Results (sorted fastest to slowest) ---")
    print(df_sorted)

    plt.figure(figsize=(12, 8))
    plt.bar(df_sorted["Experiment"], df_sorted["Average Time (s)"], color='skyblue')
    plt.ylabel("Average Time (s) (Lower is Better)")
    plt.title("Web Scraper Performance Comparison (5000 Pages)")
    plt.xticks(rotation=45, ha="right") 
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    plt.tight_layout()

    output_image_file = "main_comparison_chart.png"
    plt.savefig(output_image_file)
    print(f"\nBar chart saved as '{output_image_file}'")
    plt.show()

if __name__ == "__main__":
    create_performance_chart()