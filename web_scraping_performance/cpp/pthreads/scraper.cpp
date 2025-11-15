#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
#include <curl/curl.h>

#define NUM_THREADS 16 // A good number to start with
#define NUM_PAGES 5000

void scrape_single_url_thread_safe(const std::string& url) {
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

struct ThreadArgs {
    int thread_id;
    const std::vector<std::string>* urls;
};

void* worker_function(void* args) {
    ThreadArgs* thread_args = static_cast<ThreadArgs*>(args);
    for (size_t i = thread_args->thread_id; i < thread_args->urls->size(); i += NUM_THREADS) {
        scrape_single_url_thread_safe(thread_args->urls->at(i));
    }
    return nullptr;
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
    pthread_t threads[NUM_THREADS];
    ThreadArgs thread_args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_args[i].thread_id = i;
        thread_args[i].urls = &urls_to_scrape;
        pthread_create(&threads[i], nullptr, worker_function, &thread_args[i]);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], nullptr);
    }

    curl_global_cleanup();
    return 0;
}