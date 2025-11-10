import requests
import json
import time
from bs4 import BeautifulSoup
from urllib.parse import urljoin, urlparse
import sys

# --- Configuration ---
STARTING_URL = "http://192.168.0.141:5000/page_0.html"
MAX_PAGES_TO_CRAWL = 20
# --------------------

class SingleThreadedCrawler:
    def __init__(self, start_url, max_pages):
        self.start_url = start_url
        self.max_pages = max_pages
        self.visited_urls = set()
        self.frontier = [start_url]
        self.base_netloc = urlparse(start_url).netloc
        self.pages_crawled = 0

    def run(self):
        start_time = time.time()
        
        while self.frontier and self.pages_crawled < self.max_pages:
            url = self.frontier.pop(0)
            if url in self.visited_urls:
                continue
            
            try:
                response = requests.get(url, timeout=10)
                if response.status_code == 200:
                    self.visited_urls.add(url)
                    self.pages_crawled += 1
                    
                    soup = BeautifulSoup(response.text, 'html.parser')
                    
                    # REVISED LOGIC: Process the HTML table
                    self.process_page_data(soup)
                    
                    # REVISED LOGIC: Find the single "Next" link
                    next_link = soup.find('a', string='Next')
                    if next_link:
                        abs_url = urljoin(url, next_link['href'])
                        if urlparse(abs_url).netloc == self.base_netloc:
                            if abs_url not in self.visited_urls and abs_url not in self.frontier:
                                self.frontier.append(abs_url)
            except requests.RequestException:
                continue
        
        end_time = time.time()
        total_time = end_time - start_time
        
        results = {
            "language": "Python",
            "version": "Single-Threaded",
            "threads": 1,
            "pages_crawled": self.pages_crawled,
            "total_time_s": round(total_time, 4),
            "pages_per_second": round(self.pages_crawled / total_time if total_time > 0 else 0, 2)
        }
        print(json.dumps(results))

    def process_page_data(self, soup):
        """Finds the table and performs a calculation on its cells."""
        table = soup.find('table')
        if not table:
            return
        
        total_close = 0
        # Find all table rows, skipping the header row (tr[1:])
        for row in table.find_all('tr')[1:]:
            cells = row.find_all('td')
            # Check if the row has enough cells and the 5th cell (index 4) exists
            if len(cells) > 4:
                try:
                    close_price = float(cells[4].get_text())
                    total_close += close_price
                except (ValueError, IndexError):
                    continue # Ignore rows with malformed data

if __name__ == '__main__':
    crawler = SingleThreadedCrawler(start_url=STARTING_URL, max_pages=MAX_PAGES_TO_CRAWL)
    crawler.run()
