#include "common.hpp"
#include <omp.h>
#include <vector>

int main(int argc, char* argv[]) {
    int num_threads = (argc > 1) ? std::stoi(argv[1]) : 4;
    omp_set_num_threads(num_threads);
    curl_global_init(CURL_GLOBAL_ALL);

    std::vector<std::string> frontier;
    frontier.push_back(STARTING_URL);
    std::set<std::string> visited_urls;
    visited_urls.insert(STARTING_URL);

    int pages_crawled = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (!frontier.empty() && pages_crawled < MAX_PAGES_TO_CRAWL) {
        std::vector<std::string> current_frontier = frontier;
        frontier.clear();
        std::vector<std::string> next_frontier_candidates;

        #pragma omp parallel
        {
            CURL* curl = curl_easy_init();
            std::vector<std::string> local_next_frontier;

            #pragma omp for nowait
            for (size_t i = 0; i < current_frontier.size(); ++i) {
                if (__atomic_load_n(&pages_crawled, __ATOMIC_RELAXED) >= MAX_PAGES_TO_CRAWL) continue;

                std::string url = current_frontier[i];
                std::string content = fetch_page(curl, url);
                if (content.empty()) continue;

                __atomic_fetch_add(&pages_crawled, 1, __ATOMIC_RELAXED);

                GumboOutput* output = gumbo_parse(content.c_str());
                process_table_data(output->root);
                std::string next_link = find_next_link(output->root);
                gumbo_destroy_output(&kGumboDefaultOptions, output);

                if (!next_link.empty()) {
                    std::string base_url = url.substr(0, url.rfind('/') + 1);
                    std::string absolute_url = base_url + next_link;
                    local_next_frontier.push_back(absolute_url);
                }
            }

            #pragma omp critical
            {
                next_frontier_candidates.insert(next_frontier_candidates.end(), local_next_frontier.begin(), local_next_frontier.end());
            }
            curl_easy_cleanup(curl);
        }

        for (const auto& url : next_frontier_candidates) {
            if (visited_urls.find(url) == visited_urls.end()) {
                visited_urls.insert(url);
                frontier.push_back(url);
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_time = end_time - start_time;

    nlohmann::json results;
    results["language"] = "C++";
    results["version"] = "OpenMP";
    results["threads"] = num_threads;
    results["pages_crawled"] = pages_crawled;
    results["total_time_s"] = total_time.count();
    results["pages_per_second"] = pages_crawled / total_time.count();
    
    std::cout << results.dump() << std::endl;

    curl_global_cleanup();
    return 0;
}
