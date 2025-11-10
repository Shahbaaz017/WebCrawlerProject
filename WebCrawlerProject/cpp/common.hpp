#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <chrono>
#include <gumbo.h>
#include <curl/curl.h>
#include <mutex>
#include <atomic>
#include "json.hpp" // nlohmann/json

// --- Configuration ---
const std::string STARTING_URL = "http://192.168.0.141:5000/page_0.html";
const int MAX_PAGES_TO_CRAWL = 500;
// --------------------

// Struct to hold curl response
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function for libcurl to write received data into a string
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    auto *mem = static_cast<std::string*>(userp);
    mem->append(static_cast<char*>(contents), realsize);
    return realsize;
}

// Fetches the content of a URL using libcurl
std::string fetch_page(CURL* curl, const std::string& url) {
    std::string response_string;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // 10 second timeout
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        return "";
    }
    return response_string;
}

// Recursively search for the "Next" link
std::string find_next_link(GumboNode* node) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return "";
    }
    if (node->v.element.tag == GUMBO_TAG_A && gumbo_get_attribute(&node->v.element.attributes, "href")) {
        GumboNode* text_node = static_cast<GumboNode*>(node->v.element.children.data[0]);
        if (text_node && text_node->type == GUMBO_NODE_TEXT && std::string(text_node->v.text.text) == "Next") {
            return gumbo_get_attribute(&node->v.element.attributes, "href")->value;
        }
    }
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        std::string link = find_next_link(static_cast<GumboNode*>(children->data[i]));
        if (!link.empty()) {
            return link;
        }
    }
    return "";
}

// Recursively search for the table and process its data
void process_table_data(GumboNode* node) {
    if (node->type != GUMBO_NODE_ELEMENT) return;

    if (node->v.element.tag == GUMBO_TAG_TABLE) {
        GumboVector* tbody_children = &static_cast<GumboNode*>(node->v.element.children.data[0])->v.element.children;
        double total_close = 0.0;

        for (unsigned int i = 1; i < tbody_children->length; ++i) { // Skip header row
            GumboNode* tr_node = static_cast<GumboNode*>(tbody_children->data[i]);
            if (tr_node->type != GUMBO_NODE_ELEMENT || tr_node->v.element.tag != GUMBO_TAG_TR) continue;

            GumboVector* td_children = &tr_node->v.element.children;
            if (td_children->length > 4) {
                GumboNode* td_node = static_cast<GumboNode*>(td_children->data[4]);
                if (td_node->v.element.children.length > 0) {
                     GumboNode* text_node = static_cast<GumboNode*>(td_node->v.element.children.data[0]);
                    if (text_node->type == GUMBO_NODE_TEXT) {
                        try {
                            total_close += std::stod(text_node->v.text.text);
                        } catch (const std::invalid_argument& e) {
                            // ignore parse error
                        }
                    }
                }
            }
        }
        return; // Found and processed the table, no need to search further
    }
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        process_table_data(static_cast<GumboNode*>(children->data[i]));
    }
}

// A simple thread-safe queue for std::thread version
template<typename T>
class SafeQueue {
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(value));
    }
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
};

#endif // COMMON_HPP
