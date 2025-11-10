#include "common.h"

// Simple queue implementation for URLs
typedef struct Node {
    char* url;
    struct Node* next;
} Node;

typedef struct Queue {
    Node *front, *rear;
} Queue;

void enqueue(Queue* q, char* url) {
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->url = url;
    temp->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

char* dequeue(Queue* q) {
    if (q->front == NULL) return NULL;
    Node* temp = q->front;
    char* url = temp->url;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp);
    return url;
}

// For visited check, we'll use a simple but inefficient array search
int is_visited(char** visited, int count, const char* url) {
    for (int i = 0; i < count; ++i) {
        if (strcmp(visited[i], url) == 0) return 1;
    }
    return 0;
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    CURL* curl = curl_easy_init();
    
    Queue frontier = {NULL, NULL};
    enqueue(&frontier, strdup(STARTING_URL));

    char** visited_urls = malloc(sizeof(char*) * MAX_PAGES_TO_CRAWL);
    int visited_count = 0;
    visited_urls[visited_count++] = strdup(STARTING_URL);

    int pages_crawled = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (frontier.front != NULL && pages_crawled < MAX_PAGES_TO_CRAWL) {
        char* url = dequeue(&frontier);
        char* content = fetch_page(curl, url);
        if (!content) {
            free(url);
            continue;
        }

        pages_crawled++;

        htmlDocPtr doc = htmlReadMemory(content, strlen(content), url, NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
        if(doc) {
            xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
            if(xpathCtx) {
                process_table_data(doc, xpathCtx);
                char* next_link = find_next_link(doc, xpathCtx);
                if (next_link) {
                    char absolute_url[2048];
                    char* last_slash = strrchr(url, '/');
                    if(last_slash) {
                        strncpy(absolute_url, url, last_slash - url + 1);
                        absolute_url[last_slash - url + 1] = '\0';
                        strcat(absolute_url, next_link);

                        if (!is_visited(visited_urls, visited_count, absolute_url) && visited_count < MAX_PAGES_TO_CRAWL) {
                            visited_urls[visited_count++] = strdup(absolute_url);
                            enqueue(&frontier, strdup(absolute_url));
                        }
                    }
                    free(next_link);
                }
                xmlXPathFreeContext(xpathCtx);
            }
            xmlFreeDoc(doc);
        }
        
        free(content);
        free(url);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double total_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    json_t *results = json_object();
    json_object_set_new(results, "language", json_string("C"));
    json_object_set_new(results, "version", json_string("Single-Threaded"));
    json_object_set_new(results, "threads", json_integer(1));
    json_object_set_new(results, "pages_crawled", json_integer(pages_crawled));
    json_object_set_new(results, "total_time_s", json_real(total_time));
    json_object_set_new(results, "pages_per_second", json_real(pages_crawled / total_time));

    char* json_str = json_dumps(results, JSON_INDENT(0));
    printf("%s\n", json_str);

    free(json_str);
    json_decref(results);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    for(int i = 0; i < visited_count; ++i) free(visited_urls[i]);
    free(visited_urls);
    xmlCleanupParser();

    return 0;
}
