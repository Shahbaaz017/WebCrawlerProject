#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <omp.h> // The OpenMP header

// Thread-safe function to perform a HEAD request.
// Each thread in the parallel region will call this.
void scrape_url_c(const char* url) {
    // Each thread needs its own CURL handle.
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}

int main(int argc, char* argv[]) {
    // Check if the base URL was passed as an argument.
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <base_url>\n", argv[0]);
        return 1;
    }
    // Get the base URL from the first command-line argument.
    const char* BASE_URL = argv[1];
    
    // Initialize libcurl once, globally.
    curl_global_init(CURL_GLOBAL_ALL);

    // This pragma is the core of OpenMP. It tells the compiler to
    // create a pool of threads and distribute the loop iterations among them.
    #pragma omp parallel for
    for (int i = 0; i < 5000; i++) {
        char current_url[512];
        // Each thread builds its own URL string for the page it's assigned.
        snprintf(current_url, sizeof(current_url), "%s/%d", BASE_URL, i);
        scrape_url_c(current_url);
    }
    // An implicit barrier exists here; the program waits for all threads to finish.

    // Clean up libcurl's global resources.
    curl_global_cleanup();
    return 0;
}