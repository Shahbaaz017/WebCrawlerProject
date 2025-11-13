#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>

// This callback function is called by libcurl when it receives data.
// Instead of a custom struct, we write directly to a std::string.
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to scrape a single URL and return its content as a std::string.
std::string scrape_url(const std::string& url) {
    CURL* curl_handle;
    CURLcode res;
    std::string readBuffer; // The string that will hold the HTML content

    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &readBuffer);
        
        res = curl_easy_perform(curl_handle);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            readBuffer = ""; // Return empty string on failure
        }
        curl_easy_cleanup(curl_handle);
    }
    return readBuffer;
}

// Simple function using std::string methods to find the next link.
std::string find_next_link(const std::string& html_content, const std::string& base_url) {
    const std::string start_tag = "<a href=\"";
    const std::string end_tag = "\" class=\"nav-link\">";
    
    size_t start_pos = html_content.find(start_tag);
    if (start_pos == std::string::npos) {
        return ""; // Not found
    }
    
    start_pos += start_tag.length();
    
    size_t end_pos = html_content.find(end_tag, start_pos);
    if (end_pos == std::string::npos) {
        return ""; // Not found
    }
    
    std::string relative_path = html_content.substr(start_pos, end_pos - start_pos);
    
    // Concatenate to form the full URL
    return base_url + "/" + relative_path;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <base_url>" << std::endl;
        return 1;
    }
    const std::string BASE_URL = argv[1];
    std::string next_url_to_scrape = BASE_URL + "/0"; // Server starts at /0

    curl_global_init(CURL_GLOBAL_ALL);

    // This loop will now run 5000 times
    while (!next_url_to_scrape.empty()) {
        std::string html = scrape_url(next_url_to_scrape);
        if (!html.empty()) {
            // Your server has a simple link format, we can just find the link tag
            size_t href_pos = html.find("<a href=");
            if (href_pos != std::string::npos) {
                 next_url_to_scrape = BASE_URL + html.substr(href_pos + 9, 1); // Simplified for links like '/1'
                 // A more robust parser would be better here, but this works for simple cases.
                 // Assuming next link is always at the end
                 size_t start = html.find("href='");
                 if(start != std::string::npos){
                    start += 6;
                    size_t end = html.find("'", start);
                    next_url_to_scrape = BASE_URL + html.substr(start, end-start);
                 } else {
                    next_url_to_scrape = "";
                 }
            } else {
                next_url_to_scrape = "";
            }
        } else {
            next_url_to_scrape = "";
        }
    }
    
    curl_global_cleanup();
    return 0;
}