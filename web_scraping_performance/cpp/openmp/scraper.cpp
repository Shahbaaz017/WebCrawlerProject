#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <omp.h>

#define NUM_PAGES 5000

void scrape_single_url(const std::string& url) {
    CURL* curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1L);
        // THE FIX for parallel performance
        curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1L);
        curl_easy_perform(curl_handle);
        curl_easy_cleanup(curl_handle);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    const std::string BASE_URL = argv[1];

    std::vector<std::string> urls_to_scrape;
    urls_to_scrape.reserve(NUM_PAGES);
    for (int i = 0; i < NUM_PAGES; ++i) {
        // --- FIX: Correct URL Format ---
        urls_to_scrape.push_back(BASE_URL + "/page_" + std::to_string(i) + ".html");
    }

    curl_global_init(CURL_GLOBAL_ALL);

    #pragma omp parallel for
    for (size_t i = 0; i < urls_to_scrape.size(); ++i) {
        scrape_single_url(urls_to_scrape[i]);
    }

    curl_global_cleanup();
    return 0;
}