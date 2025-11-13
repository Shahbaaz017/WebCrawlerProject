#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>

#define NUM_THREADS 10
#define NUM_PAGES 5000

// Thread-safe function to perform a HEAD request.
// Each thread calls this function for its assigned URLs.
void scrape_url_thread_safe(const char* url) {
    // Each thread MUST have its own CURL handle.
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}

// A struct to pass arguments to each worker thread
typedef struct {
    int thread_id;
    char** urls; // Pointer to the master array of URL strings
} ThreadArgs;

// The function that each worker thread will execute
void* worker_function(void* args) {
    ThreadArgs* thread_args = (ThreadArgs*)args;
    
    // Distribute the work: each thread handles every N-th URL
    // e.g., thread 0 handles 0, 10, 20...; thread 1 handles 1, 11, 21...
    for (int i = thread_args->thread_id; i < NUM_PAGES; i += NUM_THREADS) {
        scrape_url_thread_safe(thread_args->urls[i]);
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <base_url>\n", argv[0]);
        return 1;
    }
    const char* BASE_URL = argv[1];

    // --- Phase 1: URL Generation ---
    // Allocate memory for an array of 5000 char pointers
    char** urls_to_scrape = (char**)malloc(NUM_PAGES * sizeof(char*));
    if (urls_to_scrape == NULL) {
        fprintf(stderr, "Failed to allocate memory for URL array\n");
        return 1;
    }

    // Populate the array with all 5000 URL strings
    for (int i = 0; i < NUM_PAGES; i++) {
        // Allocate enough memory for each URL string
        urls_to_scrape[i] = (char*)malloc(512 * sizeof(char));
        if (urls_to_scrape[i] == NULL) {
            fprintf(stderr, "Failed to allocate memory for a URL string\n");
            // Basic cleanup before exiting
            for (int j = 0; j < i; j++) free(urls_to_scrape[j]);
            free(urls_to_scrape);
            return 1;
        }
        snprintf(urls_to_scrape[i], 512, "%s/%d", BASE_URL, i);
    }

    curl_global_init(CURL_GLOBAL_ALL);

    // --- Phase 2: Thread Creation and Execution ---
    pthread_t threads[NUM_THREADS];
    ThreadArgs thread_args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].urls = urls_to_scrape;
        pthread_create(&threads[i], NULL, worker_function, (void*)&thread_args[i]);
    }

    // --- Phase 3: Wait for all threads to complete ---
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    curl_global_cleanup();

    // --- Phase 4: Cleanup Memory ---
    // Free each individual URL string
    for (int i = 0; i < NUM_PAGES; i++) {
        free(urls_to_scrape[i]);
    }
    // Free the main array of pointers
    free(urls_to_scrape);

    return 0;
}