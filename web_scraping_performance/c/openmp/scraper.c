#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <omp.h>

#define NUM_PAGES 5000

void scrape_url_c(const char* url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        // THE FIX for parallel performance
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    const char* BASE_URL = argv[1];
    
    curl_global_init(CURL_GLOBAL_ALL);

    #pragma omp parallel for
    for (int i = 0; i < NUM_PAGES; i++) {
        char current_url[512];
        // --- FIX: Correct URL Format ---
        snprintf(current_url, sizeof(current_url), "%s/page_%d.html", BASE_URL, i);
        scrape_url_c(current_url);
    }

    curl_global_cleanup();
    return 0;
}