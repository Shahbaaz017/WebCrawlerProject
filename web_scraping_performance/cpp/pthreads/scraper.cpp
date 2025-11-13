#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
#include <curl/curl.h>

#define NUM_THREADS 4

// libcurl callback to write data into a std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Thread-safe function to scrape a single URL. Each thread calls this.
void scrape_single_url_thread_safe(const std::string& url, int thread_id) {
    CURL* curl_handle;
    std::string readBuffer;

    // Each thread must have its own CURL handle
    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl_handle);
        if (res != CURLE_OK) {
            std::cerr << "[Thread " << thread_id << "] curl_easy_perform() failed for " 
                      << url << ": " << curl_easy_strerror(res) << std::endl;
        } else {
            // In a real app, you would parse 'readBuffer' here.
            std::cout << "[Thread " << thread_id << "] Successfully scraped page: " << url << std::endl;
        }
        curl_easy_cleanup(curl_handle);
    }
}

// Arguments to be passed to each worker thread
struct ThreadArgs {
    int thread_id;
    const std::vector<std::string>* urls; // Pointer to the vector of URLs
};

// The function that each worker thread will execute.
// It must be a C-style function for Pthreads.
void* worker_function(void* args) {
    ThreadArgs* thread_args = static_cast<ThreadArgs*>(args);

    // Distribute work: each thread handles every N-th URL
    for (size_t i = thread_args->thread_id; i < thread_args->urls->size(); i += NUM_THREADS) {
        scrape_single_url_thread_safe(thread_args->urls->at(i), thread_args->thread_id);
    }

    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <base_url>" << std::endl;
        return 1;
    }
    const std::string BASE_URL = argv[1];

    // --- Phase 1: URL Discovery (Single-threaded) ---
    std::vector<std::string> urls_to_scrape;
    std::string current_url = BASE_URL + "/0";
    
    // Assuming pages are named 0, 1, 2, ... 4999
    for (int i = 0; i < 5000; ++i) {
        urls_to_scrape.push_back(BASE_URL + "/" + std::to_string(i));
    }
    
    // ... (the rest of the main function with pthread_create and pthread_join is the same) ...
    // --- Phase 2 & 3 from previous Pthreads code ---
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