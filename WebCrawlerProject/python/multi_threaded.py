import requests
import json
import time
from bs4 import BeautifulSoup
from urllib.parse import urljoin, urlparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import threading
import sys

# --- Configuration ---
STARTING_URL = "http://192.168.0.141:5000/page_0.html"
MAX_PAGES_TO_CRAWL = 20
# --------------------

class MultiThreadedCrawler:
    def __init__(self, start_url, max_pages, num_threads):
        self.start_url = start_url
        self.max_pages = max_pages
        self.num_threads = num_threads
        
        self.frontier = [start_url]
        self.visited_urls = {start_url}
        
        self.base_netloc = urlparse(start_url).netloc
        self.pages_crawled_lock = threading.Lock()
        self.pages_crawled = 0
        
        self.frontier_lock = threading.Lock()

    def crawl_page(self, url):
        """Fetches, parses, and processes a single page. Returns the next link found."""
        try:
            response = requests.get(url, timeout=10)
            if response.status_code != 200:
                return None

            soup = BeautifulSoup(response.text, 'html.parser')
            
            # 1. Process data from the table (CPU-bound)
            self.process_page_data(soup)
            
            # 2. Extract the single "Next" link (I/O-bound)
            next_link_tag = soup.find('a', string='Next')
            if next_link_tag:
                abs_url = urljoin(url, next_link_tag['href'])
                if urlparse(abs_url).netloc == self.base_netloc:
                    return abs_url
            return None
            
        except requests.RequestException:
            return None

    def run(self):
        start_time = time.time()
        
        with ThreadPoolExecutor(max_workers=self.num_threads) as executor:
            # We start with the initial URL in the frontier
            futures = {executor.submit(self.crawl_page, self.start_url)}
            
            while futures and self.pages_crawled < self.max_pages:
                
                for future in as_completed(futures):
                    futures.remove(future)
                    next_link = future.result()
                    
                    with self.pages_crawled_lock:
                        if self.pages_crawled < self.max_pages:
                            self.pages_crawled += 1
                        else:
                            break 
                    
                    if next_link:
                        with self.frontier_lock:
                            if next_link not in self.visited_urls:
                                self.visited_urls.add(next_link)
                                # Submit the next task if we are still under the limit
                                if self.pages_crawled + len(futures) < self.max_pages:
                                    futures.add(executor.submit(self.crawl_page, next_link))

                    if self.pages_crawled >= self.max_pages:
                        break

        executor.shutdown(wait=False, cancel_futures=True)

        end_time = time.time()
        total_time = end_time - start_time

        results = {
            "language": "Python",
            "version": "Multi-Threaded",
            "threads": self.num_threads,
            "pages_crawled": self.pages_crawled,
            "total_time_s": round(total_time, 4),
            "pages_per_second": round(self.pages_crawled / total_time if total_time > 0 else 0, 2)
        }
        print(json.dumps(results))
        
    def process_page_data(self, soup):
        """Finds the table and performs a calculation on its cells."""
        table = soup.find('table')
        if not table: return
        total_close = 0
        for row in table.find_all('tr')[1:]:
            cells = row.find_all('td')
            if len(cells) > 4:
                try:
                    total_close += float(cells[4].get_text())
                except (ValueError, IndexError):
                    continue

if __name__ == '__main__':
    num_threads = int(sys.argv[1]) if len(sys.argv) > 1 else 4
    crawler = MultiThreadedCrawler(
        start_url=STARTING_URL, 
        max_pages=MAX_PAGES_TO_CRAWL, 
        num_threads=num_threads
    )
    crawler.run()
