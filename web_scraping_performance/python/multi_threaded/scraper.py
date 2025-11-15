import requests
from concurrent.futures import ThreadPoolExecutor

def scrape_page(url):
    try:
        # Use HEAD request for speed, just like C/C++ versions
        requests.head(url, timeout=10)
        return True
    except requests.exceptions.RequestException:
        return False

def run_multi_threaded_scraper(base_url, max_workers=16):
    # --- FIX: Correct URL Format ---
    urls_to_visit = [f"{base_url}/page_{i}.html" for i in range(5000)]

    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        list(executor.map(scrape_page, urls_to_visit))
            
    return len(urls_to_visit)