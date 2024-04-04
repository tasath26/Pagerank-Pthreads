#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 1000

typedef struct vertex{
    long id;
    double long rank;
    long neighbor_num;
    struct vertex *next;
    pthread_mutex_t lock;
    long outgoing_num;
}vertex_t;

typedef struct graph{
    long vertices;
    struct vertex **neighbors;
}graph_t;

/* Global Variables */
graph_t PR_graph;
long total_nodes;
int total_threads;

struct timespec start_time,finish_time;


/* Function Prototypes */
graph_t init_graph(long nodes);

vertex_t *vertex_insert(long src, long dest);

void printrank(long nodes);

void pageRank(void *args);

double long PR_sum(long vertex);

void generate_csv_file();

void free_memory();