import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin

def run_single_threaded_scraper(base_url):
    """
    Performs a single-threaded web scrape, starting from a base URL.

    Args:
        base_url (str): The base URL of the server (e.g., 'http://localhost:8000').

    Returns:
        list: A list of dictionaries, where each dictionary is a scraped item.
              Returns an empty list if an error occurs.
    """
    # The first page to start scraping is always page_0.html
    start_url = f"{base_url}/page_0.html"
    
    url_to_scrape = start_url
    all_product_data = []

    # Loop as long as there is a new URL to scrape
    while url_to_scrape:
        try:
            response = requests.get(url_to_scrape)
            # Raise an exception for bad status codes (4xx or 5xx)
            response.raise_for_status() 

            soup = BeautifulSoup(response.text, 'html.parser')

            # --- Data Extraction ---
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
                        all_product_data.append(product)

            # --- Find the Next Page Link ---
            next_link_tag = soup.find('a', class_='nav-link')
            
            if next_link_tag and 'href' in next_link_tag.attrs:
                # Use urljoin to correctly form the next absolute URL
                url_to_scrape = urljoin(url_to_scrape, next_link_tag['href'])
            else:
                # No more pages to scrape
                url_to_scrape = None

        except requests.exceptions.RequestException as e:
            print(f"ERROR: Could not fetch {url_to_scrape}. Reason: {e}")
            return [] # Return an empty list to signify failure

    return all_product_data

# This block allows the script to be run directly for testing purposes
if __name__ == '__main__':
    # Define a default base URL for direct testing
    TEST_BASE_URL = "http://localhost:8000"
    
    print(f"--- Running single-threaded test for {TEST_BASE_URL} ---")
    
    # Run the scraper function
    data = run_single_threaded_scraper(TEST_BASE_URL)
    
    if data:
        print(f"Success! Scraped {len(data)} items.")
    else:
        print("Scraping failed or returned no data.")