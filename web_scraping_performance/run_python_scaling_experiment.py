import time
import csv
import sys
import threading
import psutil  # The new library for memory tracking

# Import the Python multi-threaded scraper
from python.multi_threaded import scraper as py_multi

# --- Configuration ---
BASE_URL = "http://192.168.0.141:5000"
RUNS_PER_EXPERIMENT = 2
# We must start with 1 thread to calculate speedup
PYTHON_THREAD_COUNTS = [1, 2, 4, 8, 16, 32, 64, 128, 256]

# --- Memory Monitoring ---
# This global variable will be used by our monitor thread
peak_memory_usage = 0

def memory_monitor(stop_event):
    """
    A function that runs in a separate thread to track peak memory usage.
    """
    global peak_memory_usage
    peak_memory_usage = 0
    
    # Get the current process
    process = psutil.Process()
    
    # Poll memory usage every 10 milliseconds until the main task is done
    while not stop_event.is_set():
        # Get Resident Set Size (RSS) memory, a common metric
        rss_memory = process.memory_info().rss
        if rss_memory > peak_memory_usage:
            peak_memory_usage = rss_memory
        time.sleep(0.01)

def main():
    """
    Runs the Python scraper with various thread counts, capturing time,
    peak memory usage, and calculating speedup.
    """
    all_results = []
    baseline_time = 0  # To store the time for 1 thread (T_single)
    
    print(f"--- Starting Python Thread Scaling Benchmark ---")
    print(f"Target Server: {BASE_URL}")

    for count in PYTHON_THREAD_COUNTS:
        name = f"{count} Threads"
        timings = []
        peak_memories = []
        print(f"\nRunning with: {name}")
        
        for i in range(RUNS_PER_EXPERIMENT):
            # --- Setup for monitoring ---
            stop_monitoring = threading.Event()
            monitor_thread = threading.Thread(target=memory_monitor, args=(stop_monitoring,))
            
            # Start the monitor thread
            monitor_thread.start()
            
            # --- Run the main task ---
            start_time = time.perf_counter()
            py_multi.run_multi_threaded_scraper(BASE_URL, max_workers=count)
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
                speedup = 1.0  # Speedup is 1.0 for the baseline
            else:
                if baseline_time > 0:
                    speedup = baseline_time / avg_time
                else:
                    speedup = 0 # Avoid division by zero

            print(f"  Average Time: {avg_time:.4f}s")
            print(f"  Average Peak Memory: {avg_peak_memory_mb:.2f} MB")
            print(f"  Speedup vs 1 Thread: {speedup:.2f}x")
            
            all_results.append({
                "Threads": count, 
                "Average Time (s)": avg_time,
                "Peak Memory (MB)": avg_peak_memory_mb,
                "Speedup": speedup
            })

    output_file = "python_thread_scaling_results.csv"
    print(f"\n--- Benchmark Complete ---")
    print(f"Saving scaling results to {output_file}")
    
    if all_results:
        with open(output_file, 'w', newline='') as csvfile:
            # Add the new column names
            fieldnames = ["Threads", "Average Time (s)", "Peak Memory (MB)", "Speedup"]
            writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(all_results)
    
    print("Done.")

if __name__ == "__main__":
    main()