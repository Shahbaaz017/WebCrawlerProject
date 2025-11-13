#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <omp.h> // Include the OpenMP header

// libcurl callback to write data into a std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Thread-safe function to scrape a single URL.
void scrape_single_url(const std::string& url) {
    CURL* curl_handle;
    std::string readBuffer;

    // Each thread in the parallel region needs its own CURL handle
    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl_handle);
        if (res != CURLE_OK) {
            // Use a pragma to ensure console output is not jumbled
            #pragma omp critical
            {
                std::cerr << "[Thread " << omp_get_thread_num() << "] curl_easy_perform() failed for "
                          << url << ": " << curl_easy_strerror(res) << std::endl;
            }
        } else {
            #pragma omp critical
            {
                std::cout << "[Thread " << omp_get_thread_num() << "] Successfully scraped page: " << url << std::endl;
            }
        }
        curl_easy_cleanup(curl_handle);
    }
}


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <base_url>" << std::endl;
        return 1;
    }
    const std::string BASE_URL = argv[1];

    // --- Phase 1: URL Discovery ---
    std::vector<std::string> urls_to_scrape;
    for (int i = 0; i < 5000; ++i) {
        urls_to_scrape.push_back(BASE_URL + "/" + std::to_string(i));
    }

    curl_global_init(CURL_GLOBAL_ALL);

    // --- Phase 2: Parallel Scraping with OpenMP ---
    #pragma omp parallel for
    for (size_t i = 0; i < urls_to_scrape.size(); ++i) {
        scrape_single_url(urls_to_scrape[i]);
    }

    curl_global_cleanup();
    return 0;
}