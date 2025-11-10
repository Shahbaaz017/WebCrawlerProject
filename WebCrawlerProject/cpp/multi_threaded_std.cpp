#include "common.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

std::mutex visited_urls_mutex;
std::set<std::string> visited_urls_global;

void worker(SafeQueue<std::string>& frontier, std::atomic<int>& pages_crawled) {
    CURL* curl = curl_easy_init();
    std::string url;

    while (pages_crawled.load() < MAX_PAGES_TO_CRAWL) {
        if (frontier.try_pop(url)) {
            std::string content = fetch_page(curl, url);
            if (content.empty()) continue;

            // Increment only after successful fetch
            pages_crawled++;

            GumboOutput* output = gumbo_parse(content.c_str());
            process_table_data(output->root);
            std::string next_link = find_next_link(output->root);
            gumbo_destroy_output(&kGumboDefaultOptions, output);

            if (!next_link.empty()) {
                std::string base_url = url.substr(0, url.rfind('/') + 1);
                std::string absolute_url = base_url + next_link;

                std::lock_guard<std::mutex> lock(visited_urls_mutex);
                if (visited_urls_global.find(absolute_url) == visited_urls_global.end()) {
                    visited_urls_global.insert(absolute_url);
                    frontier.push(absolute_url);
                }
            }
        } else {
            // If queue is empty, wait a bit to see if other threads populate it
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    curl_easy_cleanup(curl);
}

int main(int argc, char* argv[]) {
    int num_threads = (argc > 1) ? std::stoi(argv[1]) : 4;
    curl_global_init(CURL_GLOBAL_ALL);

    SafeQueue<std::string> frontier;
    frontier.push(STARTING_URL);
    visited_urls_global.insert(STARTING_URL);

    std::atomic<int> pages_crawled(0);
    
    auto start_time = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, std::ref(frontier), std::ref(pages_crawled));
    }

    for (auto& t : threads) {
        t.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_time = end_time - start_time;

    nlohmann::json results;
    results["language"] = "C++";
    results["version"] = "std::thread";
    results["threads"] = num_threads;
    results["pages_crawled"] = pages_crawled.load();
    results["total_time_s"] = total_time.count();
    results["pages_per_second"] = pages_crawled.load() / total_time.count();
    
    std::cout << results.dump() << std::endl;

    curl_global_cleanup();
    return 0;
}
