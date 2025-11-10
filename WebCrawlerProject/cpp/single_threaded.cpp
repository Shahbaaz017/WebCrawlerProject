#include "common.hpp"

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();

    std::queue<std::string> frontier;
    frontier.push(STARTING_URL);
    std::set<std::string> visited_urls;
    visited_urls.insert(STARTING_URL);

    int pages_crawled = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (!frontier.empty() && pages_crawled < MAX_PAGES_TO_CRAWL) {
        std::string url = frontier.front();
        frontier.pop();

        std::string content = fetch_page(curl, url);
        if (content.empty()) continue;
        
        pages_crawled++;

        GumboOutput* output = gumbo_parse(content.c_str());
        
        process_table_data(output->root);
        std::string next_link = find_next_link(output->root);
        
        gumbo_destroy_output(&kGumboDefaultOptions, output);
        
        if (!next_link.empty()) {
            // In C++, we need to resolve relative URLs manually
            // This is a simplified resolver
            std::string base_url = url.substr(0, url.rfind('/') + 1);
            std::string absolute_url = base_url + next_link;

            if (visited_urls.find(absolute_url) == visited_urls.end()) {
                visited_urls.insert(absolute_url);
                frontier.push(absolute_url);
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_time = end_time - start_time;

    nlohmann::json results;
    results["language"] = "C++";
    results["version"] = "Single-Threaded";
    results["threads"] = 1;
    results["pages_crawled"] = pages_crawled;
    results["total_time_s"] = total_time.count();
    results["pages_per_second"] = pages_crawled / total_time.count();
    
    std::cout << results.dump() << std::endl;

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return 0;
}
