import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin
from concurrent.futures import ThreadPoolExecutor

# This is a global function that each thread will execute
def scrape_page(url):
    """
    Scrapes a single page and returns the data found.
    This function is designed to be called by a worker thread.

    Args:
        url (str): The URL of the page to scrape.

    Returns:
        list: A list of product data dictionaries from this page, or an empty list on error.
    """
    try:
        response = requests.get(url)
        response.raise_for_status()
        
        soup = BeautifulSoup(response.text, 'html.parser')
        page_data = []

        data_table = soup.find('table', id='data-table')
        if data_table:
            for row in data_table.find('tbody').find_all('tr'):
                cells = row.find_all('td')
                if len(cells) == 3:
                    product = {
                        "Product ID": cells[0].get_text(strip=True),
                        "Item Name": cells[1].get_text(strip=True),
                        "Stock": int(cells[2].get_text(strip=True))
                    }
                    page_data.append(product)
        return page_data
    except requests.exceptions.RequestException:
        # Silently fail for a single page, or you could log this
        return []

def run_multi_threaded_scraper(base_url, max_workers=4):
    """
    Performs a multi-threaded web scrape using a ThreadPoolExecutor.

    Args:
        base_url (str): The base URL of the server (e.g., 'http://localhost:8000').
        max_workers (int): The number of threads to use for scraping.

    Returns:
        list: A list of all scraped items from all pages.
    """
    # --- Phase 1: Discover all page URLs ---
    # For this phase, we do a quick single-threaded crawl just to get the links.
    urls_to_visit = set()
    url_queue = [f"{base_url}/page_0.html"]
    
    current_url = url_queue.pop(0)
    
    while current_url:
        if current_url in urls_to_visit:
            break
        urls_to_visit.add(current_url)

        try:
            response = requests.get(current_url)
            response.raise_for_status()
            soup = BeautifulSoup(response.text, 'html.parser')
            
            next_link_tag = soup.find('a', class_='nav-link')
            if next_link_tag and 'href' in next_link_tag.attrs:
                current_url = urljoin(current_url, next_link_tag['href'])
            else:
                current_url = None
        except requests.exceptions.RequestException as e:
            print(f"ERROR: Could not discover URLs. Reason: {e}")
            return []

    # --- Phase 2: Concurrent Scraping ---
    # Now we use a thread pool to scrape all the discovered URLs in parallel.
    all_product_data = []
    
    # The 'with' statement ensures threads are cleaned up properly
    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        # executor.map runs scrape_page on each URL in the list
        # It returns results in the same order as the input URLs
        results = executor.map(scrape_page, urls_to_visit)
        
        # Process results from each thread
        for page_result in results:
            all_product_data.extend(page_result)
            
    return all_product_data


# This block allows the script to be run directly for testing purposes
if __name__ == '__main__':
    TEST_BASE_URL = "http://localhost:8000"
    
    print(f"--- Running multi-threaded test for {TEST_BASE_URL} ---")
    
    # Run the scraper function
    data = run_multi_threaded_scraper(TEST_BASE_URL)
    
    if data:
        print(f"Success! Scraped {len(data)} items using multiple threads.")
    else:
        print("Scraping failed or returned no data.")