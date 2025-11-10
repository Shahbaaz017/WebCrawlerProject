#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <jansson.h>
#include <pthread.h>

// --- Configuration ---
#define STARTING_URL "http://192.168.0.141:5000/page_0.html"
#define MAX_PAGES_TO_CRAWL 20
// --------------------

// Struct to hold curl response
typedef struct {
    char *buffer;
    size_t len;
    size_t buflen;
} Memory;

// Callback for libcurl to write data into our Memory struct
static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *data) {
    size_t realsize = size * nmemb;
    Memory *mem = (Memory *)data;

    // Expand buffer if needed
    while (mem->buflen < mem->len + realsize + 1) {
        mem->buffer = realloc(mem->buffer, mem->buflen * 2);
        mem->buflen *= 2;
    }
    
    memcpy(&(mem->buffer[mem->len]), ptr, realsize);
    mem->len += realsize;
    mem->buffer[mem->len] = 0;

    return realsize;
}

// Fetches a page using libcurl
char* fetch_page(CURL* curl, const char* url) {
    Memory chunk;
    chunk.len = 0;
    chunk.buflen = 1024 * 64; // Start with 64KB
    chunk.buffer = malloc(chunk.buflen);
    chunk.buffer[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk.buffer);
        return NULL;
    }
    return chunk.buffer;
}

// Uses XPath to find the next link
char* find_next_link(xmlDocPtr doc, xmlXPathContextPtr xpathCtx) {
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((xmlChar*)"//a[text()='Next']", xpathCtx);
    if (xpathObj == NULL || xmlXPathNodeSetIsEmpty(xpathObj->nodesetval)) {
        if(xpathObj) xmlXPathFreeObject(xpathObj);
        return NULL;
    }
    
    xmlNodePtr node = xpathObj->nodesetval->nodeTab[0];
    xmlChar* href = xmlGetProp(node, (xmlChar*)"href");
    
    char* link = strdup((char*)href);
    xmlFree(href);
    xmlXPathFreeObject(xpathObj);
    return link;
}

// Uses XPath to process table data
void process_table_data(xmlDocPtr doc, xmlXPathContextPtr xpathCtx) {
    xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((xmlChar*)"//table/tbody/tr", xpathCtx);
    if (xpathObj == NULL || xmlXPathNodeSetIsEmpty(xpathObj->nodesetval)) {
        if(xpathObj) xmlXPathFreeObject(xpathObj);
        return;
    }

    double total_close = 0.0;
    xmlNodeSetPtr nodes = xpathObj->nodesetval;
    for (int i = 0; i < nodes->nodeNr; ++i) {
        xmlNodePtr row = nodes->nodeTab[i];
        xmlNodePtr cell = row->children;
        int cell_count = 0;
        while(cell) {
            if (cell->type == XML_ELEMENT_NODE && strcmp((char*)cell->name, "td") == 0) {
                if (cell_count == 4) {
                    xmlChar* content = xmlNodeGetContent(cell);
                    if (content) {
                        total_close += atof((char*)content);
                        xmlFree(content);
                    }
                    break;
                }
                cell_count++;
            }
            cell = cell->next;
        }
    }
    xmlXPathFreeObject(xpathObj);
}

#endif // COMMON_H
