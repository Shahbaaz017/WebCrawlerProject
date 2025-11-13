import time
import csv
import sys

# Import only the Python multi-threaded scraper
from python.multi_threaded import scraper as py_multi

# --- Configuration ---
BASE_URL = "http://192.168.0.141:5000"
RUNS_PER_EXPERIMENT = 2

# Define the thread counts you want to test
PYTHON_THREAD_COUNTS = [1, 2, 4, 8, 16, 32, 64, 128, 256]

def main():
    """Runs the Python multi-threaded scraper with various thread counts."""
    all_results = []
    
    print(f"--- Starting Python Thread Scaling Benchmark ---")
    print(f"Target Server: {BASE_URL}")

    for count in PYTHON_THREAD_COUNTS:
        name = f"{count} Threads"
        timings = []
        print(f"\nRunning with: {name}")
        
        for i in range(RUNS_PER_EXPERIMENT):
            try:
                start_time = time.perf_counter()
                py_multi.run_multi_threaded_scraper(BASE_URL, max_workers=count)
                end_time = time.perf_counter()
                
                duration = end_time - start_time
                timings.append(duration)
                sys.stdout.write(f"  Run {i+1}/{RUNS_PER_EXPERIMENT}: {duration:.4f}s\n")
                sys.stdout.flush()
            except Exception as e:
                print(f"  An unexpected error occurred with {name}: {e}")
                break

        if timings:
            avg_time = sum(timings) / len(timings)
            print(f"  Average Time: {avg_time:.4f}s")
            all_results.append({"Threads": count, "Average Time (s)": avg_time})

    output_file = "python_thread_scaling_results.csv"
    print(f"\n--- Benchmark Complete ---")
    print(f"Saving scaling results to {output_file}")
    
    if all_results:
        with open(output_file, 'w', newline='') as csvfile:
            fieldnames = ["Threads", "Average Time (s)"]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(all_results)
    
    print("Done.")

if __name__ == "__main__":
    main()