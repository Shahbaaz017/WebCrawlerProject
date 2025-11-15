import time
import csv
import sys
import threading
import psutil
import subprocess
import os

# --- Configuration ---
BASE_URL = "http://192.168.0.141:5000"
RUNS_PER_EXPERIMENT = 2
# Define the thread counts to test for C++ OpenMP
CPP_THREAD_COUNTS = [1, 2, 4, 8, 12, 16, 24, 32] 
EXECUTABLE_PATH = "./scraper_cpp_openmp"

# --- Memory Monitoring ---
peak_memory_usage = 0

def memory_monitor(pid, stop_event):
    """
    Monitors the memory usage of an external process given its PID.
    """
    global peak_memory_usage
    peak_memory_usage = 0
    try:
        process = psutil.Process(pid)
        while not stop_event.is_set():
            rss_memory = process.memory_info().rss
            if rss_memory > peak_memory_usage:
                peak_memory_usage = rss_memory
            time.sleep(0.01)
    except psutil.NoSuchProcess:
        # The process might finish before the monitor can attach
        pass

def main():
    all_results = []
    baseline_time = 0

    print(f"--- Starting C++ OpenMP Thread Scaling Benchmark ---")
    print(f"Target Executable: {EXECUTABLE_PATH}")

    for count in CPP_THREAD_COUNTS:
        name = f"{count} Threads"
        timings = []
        peak_memories = []
        print(f"\nRunning with: {name}")

        for i in range(RUNS_PER_EXPERIMENT):
            # --- Set up the environment for this specific run ---
            # We copy the current environment and set OMP_NUM_THREADS
            run_env = os.environ.copy()
            run_env["OMP_NUM_THREADS"] = str(count)

            # --- Start the subprocess and get its PID ---
            # Popen starts the process without blocking
            process = subprocess.Popen(
                [EXECUTABLE_PATH, BASE_URL], 
                env=run_env,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
            
            # --- Setup and start monitoring ---
            stop_monitoring = threading.Event()
            monitor_thread = threading.Thread(target=memory_monitor, args=(process.pid, stop_monitoring))
            
            start_time = time.perf_counter()
            monitor_thread.start()
            
            # Wait for the C++ program to finish
            process.wait()
            end_time = time.perf_counter()

            # --- Stop monitoring and collect results ---
            stop_monitoring.set()
            monitor_thread.join()
            
            duration = end_time - start_time
            timings.append(duration)
            peak_memories.append(peak_memory_usage)
            
            sys.stdout.write(f"  Run {i+1}/{RUNS_PER_EXPERIMENT}: {duration:.4f}s | Peak Memory: {peak_memory_usage / 1024 / 1024:.2f} MB\n")
            sys.stdout.flush()
        
        if timings:
            avg_time = sum(timings) / len(timings)
            avg_peak_memory_mb = (sum(peak_memories) / len(peak_memories)) / 1024 / 1024

            if count == 1:
                baseline_time = avg_time
                speedup = 1.0
            else:
                speedup = baseline_time / avg_time if baseline_time > 0 else 0

            print(f"  Average Time: {avg_time:.4f}s")
            print(f"  Average Peak Memory: {avg_peak_memory_mb:.2f} MB")
            print(f"  Speedup vs 1 Thread: {speedup:.2f}x")

            all_results.append({
                "Threads": count, 
                "Average Time (s)": avg_time,
                "Peak Memory (MB)": avg_peak_memory_mb,
                "Speedup": speedup
            })

    output_file = "cpp_openmp_scaling_results.csv"
    print(f"\n--- Benchmark Complete ---")
    print(f"Saving C++ scaling results to {output_file}")
    
    if all_results:
        with open(output_file, 'w', newline='') as csvfile:
            fieldnames = ["Threads", "Average Time (s)", "Peak Memory (MB)", "Speedup"]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(all_results)
    
    print("Done.")

if __name__ == "__main__":
    main()