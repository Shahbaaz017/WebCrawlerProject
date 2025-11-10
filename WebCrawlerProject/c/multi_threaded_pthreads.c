#include "common.h"

// Thread-safe queue for Pthreads
typedef struct TSQueueNode {
    char* url;
    struct TSQueueNode* next;
} TSQueueNode;

typedef struct {
    TSQueueNode *front, *rear;
    pthread_mutex_t mutex;
} TSQueue;

void ts_enqueue(TSQueue* q, char* url) {
    TSQueueNode* temp = (TSQueueNode*)malloc(sizeof(TSQueueNode));
    temp->url = url;
    temp->next = NULL;
    pthread_mutex_lock(&q->mutex);
    if (q->rear == NULL) {
        q->front = q->rear = temp;
    } else {
        q->rear->next = temp;
        q->rear = temp;
    }
    pthread_mutex_unlock(&q->mutex);
}

char* ts_dequeue(TSQueue* q) {
    pthread_mutex_lock(&q->mutex);
    if (q->front == NULL) {
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    }
    TSQueueNode* temp = q->front;
    char* url = temp->url;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    pthread_mutex_unlock(&q->mutex);
    free(temp);
    return url;
}

// Thread-safe set (using a mutex-protected array)
typedef struct {
    char** urls;
    int count;
    int capacity;
    pthread_mutex_t mutex;
} TSSet;

int ts_set_add(TSSet* s, char* url) {
    pthread_mutex_lock(&s->mutex);
    for (int i = 0; i < s->count; ++i) {
        if (strcmp(s->urls[i], url) == 0) {
            pthread_mutex_unlock(&s->mutex);
            return 0; // Already exists
        }
    }
    if (s->count >= s->capacity) { // For simplicity, we don't realloc
        pthread_mutex_unlock(&s->mutex);
        return 0;
    }
    s->urls[s->count++] = strdup(url);
    pthread_mutex_unlock(&s->mutex);
    return 1; // Added successfully
}

// Struct for worker thread arguments
typedef struct {
    TSQueue* frontier;
    TSSet* visited;
    int* pages_crawled;
    pthread_mutex_t* count_mutex;
} WorkerArgs;

void* worker_thread(void* args) {
    WorkerArgs* wargs = (WorkerArgs*)args;
    CURL* curl = curl_easy_init();

    while (1) {
        int current_pages;
        pthread_mutex_lock(wargs->count_mutex);
        current_pages = *(wargs->pages_crawled);
        pthread_mutex_unlock(wargs->count_mutex);
        if (current_pages >= MAX_PAGES_TO_CRAWL) break;

        char* url = ts_dequeue(wargs->frontier);
        if (url == NULL) {
            struct timespec req = { .tv_sec = 0, .tv_nsec = 10000000L }; 
            nanosleep(&req, NULL); // Wait 10ms if queue is empty
            continue;
        }

        char* content = fetch_page(curl, url);
        if (!content) {
            free(url);
            continue;
        }

        pthread_mutex_lock(wargs->count_mutex);
        if (*(wargs->pages_crawled) < MAX_PAGES_TO_CRAWL) {
            (*(wargs->pages_crawled))++;
        }
        pthread_mutex_unlock(wargs->count_mutex);
        
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
                         if(ts_set_add(wargs->visited, absolute_url)) {
                             ts_enqueue(wargs->frontier, strdup(absolute_url));
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
    curl_easy_cleanup(curl);
    return NULL;
}

int main(int argc, char* argv[]) {
    int num_threads = (argc > 1) ? atoi(argv[1]) : 4;
    curl_global_init(CURL_GLOBAL_ALL);

    TSQueue frontier = {NULL, NULL, PTHREAD_MUTEX_INITIALIZER};
    ts_enqueue(&frontier, strdup(STARTING_URL));

    TSSet visited = {malloc(sizeof(char*) * MAX_PAGES_TO_CRAWL), 0, MAX_PAGES_TO_CRAWL, PTHREAD_MUTEX_INITIALIZER};
    ts_set_add(&visited, STARTING_URL);

    int pages_crawled = 0;
    pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

    WorkerArgs args = {&frontier, &visited, &pages_crawled, &count_mutex};
    pthread_t threads[num_threads];
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, worker_thread, &args);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double total_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    json_t *results = json_object();
    json_object_set_new(results, "language", json_string("C"));
    json_object_set_new(results, "version", json_string("Pthreads"));
    json_object_set_new(results, "threads", json_integer(num_threads));
    json_object_set_new(results, "pages_crawled", json_integer(pages_crawled));
    json_object_set_new(results, "total_time_s", json_real(total_time));
    json_object_set_new(results, "pages_per_second", json_real(pages_crawled / total_time));

    char* json_str = json_dumps(results, JSON_INDENT(0));
    printf("%s\n", json_str);

    free(json_str);
    json_decref(results);
    curl_global_cleanup();
    for(int i = 0; i < visited.count; ++i) free(visited.urls[i]);
    free(visited.urls);
    xmlCleanupParser();
    return 0;
}
