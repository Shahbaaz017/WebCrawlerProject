import requests
import time

def run_single_threaded_scraper(base_url):
    """
    Sequentially scrapes 5000 pages from a server one by one.

    This corrected version generates all 5000 URLs directly and uses
    efficient HEAD requests to provide a proper performance baseline.
    """
    # Loop from 0 to 4999 to get all 5000 pages.
    for i in range(5000):
        # Construct the correct URL format, e.g., "http://.../page_42.html"
        page_url = f"{base_url}/page_{i}.html"
        
        try:
            # Use a HEAD request for efficiency. We only need to confirm the
            # page exists, not download its content for this benchmark.
            # This provides a fair comparison to the optimized C/C++ versions.
            requests.head(page_url, timeout=10)
        
        except requests.exceptions.RequestException as e:
            # If any request fails, print an error and stop.
            # This helps in debugging server or network issues.
            print(f"\nError scraping {page_url}: {e}")
            break
            
    # The function doesn't need to return anything for the benchmark.
    return

# This block allows the script to be run directly for a standalone test.
if __name__ == '__main__':
    # Define the base URL for direct testing.
    TEST_BASE_URL = "http://192.168.0.141:5000"
    
    print(f"--- Running standalone test for corrected Python Single-Threaded Scraper ---")
    print(f"Target Server: {TEST_BASE_URL}")
    print("Scraping 5000 pages sequentially... Please wait.")
    
    # Use a high-precision timer to measure the execution.
    start_time = time.perf_counter()

    # Run the scraper function.
    run_single_threaded_scraper(TEST_BASE_URL)

    end_time = time.perf_counter()
    duration = end_time - start_time

    print("\n--- Test Complete ---")
    print(f"Total time taken: {duration:.4f} seconds")