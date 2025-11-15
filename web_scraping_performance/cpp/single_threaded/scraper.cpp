#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>

// A simple function to perform a HEAD request for a given URL.
// We use a HEAD request because we only need to confirm the page exists,
// not download its content, making the benchmark faster and more focused.
void scrape_url(const std::string& url) {
    // A CURL handle should be created for each request in a simple loop.
    CURL* curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);
        // Use CURLOPT_NOBODY for a HEAD request.
        curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1L);
        
        // Perform the request.
        curl_easy_perform(curl_handle);
        
        // Clean up this handle.
        curl_easy_cleanup(curl_handle);
    }
}

int main(int argc, char* argv[]) {
    // Check if the base URL was passed as a command-line argument.
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <base_url>" << std::endl;
        return 1; // Exit with an error.
    }
    // Get the base URL from the first argument.
    const std::string BASE_URL = argv[1];

    // Initialize libcurl once for the entire program.
    curl_global_init(CURL_GLOBAL_ALL);

    // This is the main loop that performs the sequential scraping.
    for (int i = 0; i < 5000; ++i) {
        // Correctly construct the URL for each page.
        std::string current_url = BASE_URL + "/page_" + std::to_string(i) + ".html";
        scrape_url(current_url);
    }

    // Clean up libcurl's global resources.
    curl_global_cleanup();
    return 0;
}