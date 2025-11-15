import time
import sys

# Import the specific function we want to test
from web_scraping_performance.python.single_threaded.scraper import run_single_threaded_scraper

# --- Configuration ---
BASE_URL = "http://192.168.0.141:5000"

def main():
    """
    A simple script to run and time only the single-threaded Python scraper.
    """
    print("--- Starting standalone test for Python Single-Threaded Scraper ---")
    print(f"Target Server: {BASE_URL}")
    print("Scraping 5000 pages... Please wait.")

    # Use a high-precision timer
    start_time = time.perf_counter()

    # Run the scraper function
    run_single_threaded_scraper(BASE_URL)

    end_time = time.perf_counter()
    
    duration = end_time - start_time

    print("\n--- Test Complete ---")
    print(f"Total time taken: {duration:.4f} seconds")

if __name__ == "__main__":
    main()