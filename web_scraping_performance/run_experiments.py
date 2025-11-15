import time
import subprocess
import csv
import sys

from python.single_threaded import scraper as py_single
from python.multi_threaded import scraper as py_multi

# --- Configuration ---
BASE_URL = "http://192.168.0.141:5000"
RUNS_PER_EXPERIMENT = 2
PYTHON_MT_THREADS = 16 # A good fixed number for comparison

# --- Experiment Definitions ---
EXPERIMENTS = [
    {"name": "C++ Single-Threaded", "type": "executable", "command": ["./scraper_cpp_single", BASE_URL]},
    {"name": "C++ Pthreads",        "type": "executable", "command": ["./scraper_cpp_pthreads", BASE_URL]},
    {"name": "C++ OpenMP",          "type": "executable", "command": ["./scraper_cpp_openmp", BASE_URL]},
    {"name": "C Single-Threaded",   "type": "executable", "command": ["./scraper_c_single", BASE_URL]},
    {"name": "C Pthreads",          "type": "executable", "command": ["./scraper_c_pthreads", BASE_URL]},
    {"name": "C OpenMP",            "type": "executable", "command": ["./scraper_c_openmp", BASE_URL]},
    {"name": "Python Single-Threaded", "type": "python", "function": py_single.run_single_threaded_scraper},
    {"name": f"Python MT ({PYTHON_MT_THREADS} Threads)", "type": "python", "function": py_multi.run_multi_threaded_scraper},
]

def run_experiment(experiment_config):
    exp_type = experiment_config["type"]
    start_time = time.perf_counter()
    if exp_type == "executable":
        subprocess.run(experiment_config["command"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    elif exp_type == "python":
        if "MT" in experiment_config["name"]:
            experiment_config["function"](BASE_URL, max_workers=PYTHON_MT_THREADS)
        else:
            experiment_config["function"](BASE_URL)
    end_time = time.perf_counter()
    return end_time - start_time

def main():
    all_results = []
    print(f"--- Starting Main Performance Benchmark ---")
    print(f"Target Server: {BASE_URL}")

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
            except Exception as e:
                print(f"  ERROR running {name}: {e}")
                break
        if timings:
            avg_time = sum(timings) / len(timings)
            print(f"  Average Time: {avg_time:.4f}s")
            all_results.append({"Experiment": name, "Average Time (s)": avg_time})

    output_file = "main_comparison_results.csv"
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