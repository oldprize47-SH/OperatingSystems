#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_PATH_LEN 256

// data types
typedef struct {
    char **data;
    int size;
    int front;
    int rear;
} Buffer;

typedef struct {
    Buffer *buf;
    char *start_dir;
    char *search_word;
    int *found_total;
    int *num_files;
    int num_threads;
} SharedArgs;

typedef struct {
    long tid;
    SharedArgs *args;
} ThreadArg;

// function decls
void *producer(void *arg);
void *consumer(void *arg);
void scan_dir(char *path, SharedArgs *args);
void put(const char *path, SharedArgs *args);
char *get(SharedArgs *args);

// semaphores
sem_t empty, full, mutex, update_mutex;

int main(int argc, char *argv[]) {
    int opt;
    char *dir = NULL;
    int buf_size = 0;
    int num_threads = 0;
    char *search_word = NULL;

    Buffer buf;
    int found_total = 0;
    int num_files = 0;

    // read opts
    while ((opt = getopt(argc, argv, "b:t:d:w:")) != -1) {
        switch (opt) {
            case 'b':
                buf_size = atoi(optarg);
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'd':
                dir = optarg;
                break;
            case 'w':
                search_word = optarg;
                break;
            default:
                fprintf(stderr,
                        "Usage: %s -b <buffer_size> -t <num_threads> -d <directory> -w <word>\n",
                        argv[0]);
                return 1;
        }
    }
    if (buf_size <= 0 || num_threads <= 0 || !dir || !search_word) {
        fprintf(stderr, "All args (-b -t -d -w) are required.\n");
        return 1;
    }

    printf("Buffer size=%d, Num threads=%d, Directory=%s, SearchWord=%s\n",
           buf_size, num_threads, dir, search_word);

    // init buffer
    buf.data = calloc(buf_size, sizeof(char *));
    buf.size = buf_size;
    buf.front = buf.rear = 0;

    // init semaphores
    sem_init(&empty, 0, buf_size);
    sem_init(&full, 0, 0);
    sem_init(&mutex, 0, 1);
    sem_init(&update_mutex, 0, 1);

    // thread vars
    pthread_t prod_tid;
    pthread_t *cons_tids = malloc(num_threads * sizeof *cons_tids);

    // fill shared args
    SharedArgs shared_args = {
        .buf = &buf,
        .start_dir = dir,
        .search_word = search_word,
        .found_total = &found_total,
        .num_files = &num_files,
        .num_threads = num_threads};

    // make producer
    ThreadArg prod_arg = {.tid = -1, .args = &shared_args};
    pthread_create(&prod_tid, NULL, producer, &prod_arg);

    // make consumers
    ThreadArg *cons_args = malloc(num_threads * sizeof *cons_args);
    for (int i = 0; i < num_threads; ++i) {
        cons_args[i] = (ThreadArg){.tid = i, .args = &shared_args};
        pthread_create(&cons_tids[i], NULL, consumer, &cons_args[i]);
    }

    // wait all
    pthread_join(prod_tid, NULL);
    for (int i = 0; i < num_threads; ++i) pthread_join(cons_tids[i], NULL);

    // final result
    printf("Total found = %d (Num files=%d)\n", found_total, num_files);

    // free memory
    free(cons_tids);
    free(cons_args);
    for (int i = 0; i < buf.size; ++i) free(buf.data[i]);
    free(buf.data);
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);
    sem_destroy(&update_mutex);
    return 0;
}

// producer: scan dir and push paths
void *producer(void *arg) {
    ThreadArg *targ = (ThreadArg *)arg;
    SharedArgs *args = targ->args;
    scan_dir(args->start_dir, args);

    // push NULL for terminate
    for (int i = 0; i < args->num_threads; ++i) put(NULL, args);
    return NULL;
}

// consumer: search word in file
void *consumer(void *arg) {
    ThreadArg *targ = (ThreadArg *)arg;
    long tid = targ->tid;
    SharedArgs *args = targ->args;
    printf("[Thread#%ld] started searching '%s'...\n", tid, args->search_word);

    while (1) {
        char *path = get(args);
        if (!path) break;  // NULL marker

        FILE *fp = fopen(path, "r");
        if (!fp) {
            free(path);
            continue;
        }

        int found_local = 0;
        char *line = NULL;
        size_t len = 0;
        ssize_t read_len;

        char *word_lower = strdup(args->search_word);
        for (int i = 0; word_lower[i]; ++i) word_lower[i] = tolower(word_lower[i]);

        while ((read_len = getline(&line, &len, fp)) != -1) {
            char *line_lower = strdup(line);
            for (int i = 0; line_lower[i]; ++i) line_lower[i] = tolower(line_lower[i]);
            char *p = line_lower;
            while ((p = strstr(p, word_lower))) {
                ++found_local;
                ++p;
            }
            free(line_lower);
        }

        fclose(fp);
        if (line) free(line);
        free(word_lower);

        int file_idx;
        sem_wait(&update_mutex);
        file_idx = *(args->num_files);
        *(args->found_total) += found_local;
        (*(args->num_files))++;
        sem_post(&update_mutex);

        printf("[Thread#%ld-%d] %s: %d found\n", tid, file_idx, path, found_local);
        free(path);
    }
    return NULL;
}

// scan directory
void scan_dir(char *path, SharedArgs *args) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *ent;
    while ((ent = readdir(dir))) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

        if (strlen(path) + strlen(ent->d_name) + 2 > MAX_PATH_LEN) {
            fprintf(stderr, "Path too long, skip: %s/%s\n", path, ent->d_name);
            continue;
        }
        char full[MAX_PATH_LEN];
        snprintf(full, sizeof full, "%s/%s", path, ent->d_name);

        struct stat st;
        if (lstat(full, &st) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(st.st_mode))
            scan_dir(full, args);  // dir
        else if (S_ISREG(st.st_mode))
            put(full, args);  // file
    }
    closedir(dir);
}

// buffer funtions
void put(const char *path, SharedArgs *args) {
    Buffer *buf = args->buf;
    sem_wait(&empty);
    sem_wait(&mutex);

    if (path != NULL)
        buf->data[buf->rear] = strdup(path);
    else
        buf->data[buf->rear] = NULL;

    buf->rear = (buf->rear + 1) % buf->size;

    sem_post(&mutex);
    sem_post(&full);
}

char *get(SharedArgs *args) {
    Buffer *buf = args->buf;
    sem_wait(&full);
    sem_wait(&mutex);

    char *path = buf->data[buf->front];
    buf->front = (buf->front + 1) % buf->size;

    sem_post(&mutex);
    sem_post(&empty);
    return path;
}
