#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// A simple function to perform a HEAD request for a given URL.
void scrape_url(const char* url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        // Set the URL to fetch
        curl_easy_setopt(curl, CURLOPT_URL, url);
        // Set a timeout for the request
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        // Use a HEAD request (CURLOPT_NOBODY) as we don't need the page body.
        // This is much faster and uses less bandwidth.
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        
        // Perform the request. We don't care about the return code for the benchmark.
        curl_easy_perform(curl);
        
        // Cleanup the handle
        curl_easy_cleanup(curl);
    }
}

int main(int argc, char* argv[]) {
    // Ensure the base URL was passed as an argument
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <base_url>\n", argv[0]);
        return 1; // Indicate an error
    }
    // Get the base URL from the command-line arguments
    const char* BASE_URL = argv[1];

    // Initialize libcurl globally, once per program
    curl_global_init(CURL_GLOBAL_ALL);

    // Loop sequentially from page 0 to 4999
    for (int i = 0; i < 5000; i++) {
        char current_url[512];
        // Build the full URL string (e.g., "http://.../0", "http://.../1", etc.)
        snprintf(current_url, sizeof(current_url), "%s/%d", BASE_URL, i);
        scrape_url(current_url);
    }

    // Clean up libcurl's global resources
    curl_global_cleanup();
    return 0;
}