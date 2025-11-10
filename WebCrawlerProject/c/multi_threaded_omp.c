#include "common.h"
#include <omp.h>

// Simple dynamic array for string vectors
typedef struct {
    char** items;
    int size;
    int capacity;
} StringVector;

void vector_add(StringVector* v, char* item) {
    if (v->size >= v->capacity) {
        v->capacity *= 2;
        v->items = realloc(v->items, sizeof(char*) * v->capacity);
    }
    v->items[v->size++] = item;
}

// Inefficient visited check for simplicity
int is_visited_omp(char** visited, int count, const char* url) {
    for (int i = 0; i < count; ++i) {
        if (strcmp(visited[i], url) == 0) return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    int num_threads = (argc > 1) ? atoi(argv[1]) : 4;
    omp_set_num_threads(num_threads);
    curl_global_init(CURL_GLOBAL_ALL);

    StringVector frontier = {(char**)malloc(sizeof(char*)*16), 0, 16};
    vector_add(&frontier, strdup(STARTING_URL));

    StringVector visited = {(char**)malloc(sizeof(char*)*MAX_PAGES_TO_CRAWL), 0, MAX_PAGES_TO_CRAWL};
    vector_add(&visited, strdup(STARTING_URL));

    int pages_crawled = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (frontier.size > 0 && pages_crawled < MAX_PAGES_TO_CRAWL) {
        StringVector current_frontier = frontier;
        frontier.items = (char**)malloc(sizeof(char*)*16);
        frontier.size = 0;
        frontier.capacity = 16;
        
        StringVector next_frontier_candidates = {(char**)malloc(sizeof(char*)*16), 0, 16};

        #pragma omp parallel
        {
            CURL* curl = curl_easy_init();
            StringVector local_next_frontier = {(char**)malloc(sizeof(char*)*16), 0, 16};

            #pragma omp for nowait
            for (int i = 0; i < current_frontier.size; i++) {
                if (__atomic_load_n(&pages_crawled, __ATOMIC_RELAXED) >= MAX_PAGES_TO_CRAWL) continue;

                char* url = current_frontier.items[i];
                char* content = fetch_page(curl, url);
                if (!content) continue;

                __atomic_fetch_add(&pages_crawled, 1, __ATOMIC_RELAXED);
                
                htmlDocPtr doc = htmlReadMemory(content, strlen(content), url, NULL, HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
                if(doc) {
                    xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
                    if(xpathCtx) {
                        process_table_data(doc, xpathCtx);
                        char* next_link = find_next_link(doc, xpathCtx);
                        if(next_link) {
                            char absolute_url[2048];
                            char* last_slash = strrchr(url, '/');
                             if(last_slash) {
                                strncpy(absolute_url, url, last_slash - url + 1);
                                absolute_url[last_slash - url + 1] = '\0';
                                strcat(absolute_url, next_link);
                                vector_add(&local_next_frontier, strdup(absolute_url));
                            }
                            free(next_link);
                        }
                        xmlXPathFreeContext(xpathCtx);
                    }
                    xmlFreeDoc(doc);
                }
                free(content);
            }

            #pragma omp critical
            {
                for(int i = 0; i < local_next_frontier.size; ++i) {
                    vector_add(&next_frontier_candidates, local_next_frontier.items[i]);
                }
            }
            free(local_next_frontier.items);
            curl_easy_cleanup(curl);
        }

        for (int i=0; i < current_frontier.size; ++i) free(current_frontier.items[i]);
        free(current_frontier.items);

        for (int i = 0; i < next_frontier_candidates.size; ++i) {
            if (!is_visited_omp(visited.items, visited.size, next_frontier_candidates.items[i])) {
                if (visited.size < visited.capacity) {
                    vector_add(&visited, strdup(next_frontier_candidates.items[i]));
                    vector_add(&frontier, strdup(next_frontier_candidates.items[i]));
                }
            }
            free(next_frontier_candidates.items[i]);
        }
        free(next_frontier_candidates.items);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double total_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    json_t *results = json_object();
    json_object_set_new(results, "language", json_string("C"));
    json_object_set_new(results, "version", json_string("OpenMP"));
    json_object_set_new(results, "threads", json_integer(num_threads));
    json_object_set_new(results, "pages_crawled", json_integer(pages_crawled));
    json_object_set_new(results, "total_time_s", json_real(total_time));
    json_object_set_new(results, "pages_per_second", json_real(pages_crawled / total_time));

    char* json_str = json_dumps(results, JSON_INDENT(0));
    printf("%s\n", json_str);

    free(json_str);
    json_decref(results);
    curl_global_cleanup();
    for(int i = 0; i < visited.size; ++i) free(visited.items[i]);
    free(visited.items);
    xmlCleanupParser();
    return 0;
}
