#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>

#define NUM_THREADS 10
#define NUM_PAGES 5000

void scrape_url_thread_safe(const char* url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        // THE FIX: Crucial for multi-threaded performance.
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}

typedef struct {
    int thread_id;
    char** urls;
} ThreadArgs;

void* worker_function(void* args) {
    ThreadArgs* thread_args = (ThreadArgs*)args;
    for (int i = thread_args->thread_id; i < NUM_PAGES; i += NUM_THREADS) {
        scrape_url_thread_safe(thread_args->urls[i]);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;
    const char* BASE_URL = argv[1];

    char** urls_to_scrape = (char**)malloc(NUM_PAGES * sizeof(char*));
    if (!urls_to_scrape) return 1;

    for (int i = 0; i < NUM_PAGES; i++) {
        urls_to_scrape[i] = (char*)malloc(512 * sizeof(char));
        if (!urls_to_scrape[i]) return 1;
        snprintf(urls_to_scrape[i], 512, "%s/%d", BASE_URL, i);
    }

    curl_global_init(CURL_GLOBAL_ALL);
    pthread_t threads[NUM_THREADS];
    ThreadArgs thread_args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].urls = urls_to_scrape;
        pthread_create(&threads[i], NULL, worker_function, (void*)&thread_args[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    curl_global_cleanup();
    for (int i = 0; i < NUM_PAGES; i++) free(urls_to_scrape[i]);
    free(urls_to_scrape);

    return 0;
}