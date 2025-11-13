import time
import subprocess
import csv
import sys

# Import the Python scraper functions
from python.single_threaded import scraper as py_single
from python.multi_threaded import scraper as py_multi

# --- Configuration ---
# Set the new base URL for the server with 5000 pages
BASE_URL = "http://192.168.0.141:5000"

# Scraping 5000 pages takes time. Set runs to 1 for a quick test,
# or 2-3 for a more stable average.
RUNS_PER_EXPERIMENT = 2

# The commands are updated to pass the BASE_URL as a command-line argument
EXPERIMENTS = [
    # C++ Experiments
    {"name": "C++ Single-Threaded", "type": "executable", "command": ["./scraper_cpp_single", BASE_URL]},
    {"name": "C++ Pthreads",        "type": "executable", "command": ["./scraper_cpp_pthreads", BASE_URL]},
    {"name": "C++ OpenMP",          "type": "executable", "command": ["./scraper_cpp_openmp", BASE_URL]},
    
    # C Experiments
    {"name": "C Single-Threaded",   "type": "executable", "command": ["./scraper_c_single", BASE_URL]},
    {"name": "C Pthreads",          "type": "executable", "command": ["./scraper_c_pthreads", BASE_URL]},
    {"name": "C OpenMP",            "type": "executable", "command": ["./scraper_c_openmp", BASE_URL]},

    # Python Experiments (they already accept the URL as a function argument)
    {"name": "Python Single-Threaded", "type": "python", "function": py_single.run_single_threaded_scraper},
    {"name": "Python Multi-Threaded",  "type": "python", "function": py_multi.run_multi_threaded_scraper},
]

def run_experiment(experiment_config):
    """Runs a single experiment and returns the execution time."""
    exp_type = experiment_config["type"]
    
    start_time = time.perf_counter()

    if exp_type == "executable":
        # Run C/C++ executables, hiding their output for a clean benchmark report
        subprocess.run(
            experiment_config["command"], 
            check=True, 
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL
        )
    elif exp_type == "python":
        # Call Python functions directly
        experiment_config["function"](BASE_URL)
    
    end_time = time.perf_counter()
    return end_time - start_time

def main():
    """Main function to run all experiments and save results."""
    all_results = []
    
    print(f"--- Starting Web Scraping Performance Benchmark (5000 Pages) ---")
    print(f"Target Server: {BASE_URL}")
    print(f"Running each experiment {RUNS_PER_EXPERIMENT} times...")

    for experiment in EXPERIMENTS:
        name = experiment['name']
        timings = []
        print(f"\nRunning: {name}")
        
        for i in range(RUNS_PER_EXPERIMENT):
            try:
                duration = run_experiment(experiment)
                timings.append(duration)
                sys.stdout.write(f"  Run {i+1}/{RUNS_PER_EXPERIMENT}: {duration:.4f}s\n")
                sys.stdout.flush()
            except subprocess.CalledProcessError as e:
                print(f"  ERROR running {name}. Command failed: {e}")
                break
            except Exception as e:
                print(f"  An unexpected error occurred with {name}: {e}")
                break

        if timings:
            avg_time = sum(timings) / len(timings)
            print(f"  Average Time: {avg_time:.4f}s")
            all_results.append({"Experiment": name, "Average Time (s)": avg_time})

    output_file = "results_5000_pages.csv"
    print(f"\n--- Benchmark Complete ---")
    print(f"Saving results to {output_file}")
    
    if all_results:
        with open(output_file, 'w', newline='') as csvfile:
            fieldnames = ["Experiment", "Average Time (s)"]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(all_results)
    
    print("Done.")

if __name__ == "__main__":
    main()