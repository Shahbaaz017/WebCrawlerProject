#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// A simple function to perform a HEAD request for a given URL in C.
void scrape_url(const char* url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        // Use a HEAD request (CURLOPT_NOBODY) as we don't need the page body.
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        
        // Perform the request.
        curl_easy_perform(curl);
        
        // Cleanup the handle.
        curl_easy_cleanup(curl);
    }
}

int main(int argc, char* argv[]) {
    // Ensure the base URL was passed as an argument.
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <base_url>\n", argv[0]);
        return 1; // Indicate an error.
    }
    // Get the base URL from the command-line arguments.
    const char* BASE_URL = argv[1];

    // Initialize libcurl globally.
    curl_global_init(CURL_GLOBAL_ALL);

    // This is the main sequential scraping loop.
    for (int i = 0; i < 5000; i++) {
        char current_url[512];
        // Safely construct the correct URL string for each page.
        snprintf(current_url, sizeof(current_url), "%s/page_%d.html", BASE_URL, i);
        scrape_url(current_url);
    }

    // Clean up libcurl's global resources.
    curl_global_cleanup();
    return 0;
}