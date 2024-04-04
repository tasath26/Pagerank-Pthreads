#include "pagerank.h"

int flag = 0;

graph_t init_graph(long nodes){

    graph_t new_graph;
    new_graph.vertices = nodes;

    new_graph.neighbors = (vertex_t**)malloc(nodes * sizeof(vertex_t*));
    if(new_graph.neighbors == NULL){
        printf("Error in malloc\n");
        exit(1);
    }


    for(long i = 0; i < nodes; i++){
        new_graph.neighbors[i] = (vertex_t*)malloc(sizeof(vertex_t));
        new_graph.neighbors[i]->id = i;
        new_graph.neighbors[i]->rank = 1.0;
        new_graph.neighbors[i]->neighbor_num = 0;
        new_graph.neighbors[i]->next =NULL;
        new_graph.neighbors[i]->outgoing_num = 0;
        pthread_mutex_init(&new_graph.neighbors[i]->lock, NULL);
    }

    return new_graph;
}

vertex_t *vertex_insert(long src,long dest){

    long source_node = src;
    long destination_node = dest;

    if (PR_graph.neighbors[destination_node] == NULL) {
        // Allocate memory for PR_graph.neighbors[destination_node]
        PR_graph.neighbors[destination_node] = (vertex_t*)malloc(sizeof(vertex_t));
        if (PR_graph.neighbors[destination_node] == NULL) {
            printf("Error in malloc\n");
            exit(1);
        }
        // Initialize the newly allocated vertex
        PR_graph.neighbors[destination_node]->next = NULL; // Assuming initialization
        PR_graph.neighbors[destination_node]->neighbor_num = 0; // Assuming initialization
    }



    vertex_t *new_vertex = (vertex_t*)malloc(sizeof(vertex_t));
    if(new_vertex == NULL){
        printf("Error in malloc\n");
        exit(1);
    }

    new_vertex->id = source_node;
    new_vertex->rank = 1.0;
    new_vertex->next = PR_graph.neighbors[destination_node]->next;
    PR_graph.neighbors[destination_node]->next = new_vertex;
    PR_graph.neighbors[destination_node]->neighbor_num++;

    PR_graph.neighbors[source_node]->outgoing_num++;

    return new_vertex;
    
}


void pageRank(void *args){
    int curr_thread = *((int*)args);

    int from = curr_thread * (total_nodes/total_threads);
    int to = (curr_thread + 1) * (total_nodes/total_threads);
    long neighbors;
    double long curr_rank;

    if(curr_thread == total_threads - 1){
        to = total_nodes - 1;
    }
    
    for(int i = 0; i < 50; i++){
        for(long j = from; j < to; j++){
            
            
            neighbors = PR_graph.neighbors[j]->neighbor_num;
            curr_rank = PR_graph.neighbors[j]->rank;

            if(neighbors == 0){
                continue;
            }

            pthread_mutex_lock(&PR_graph.neighbors[j]->lock);
            
            PR_graph.neighbors[j]->rank = 0.15 + PR_sum(j)*0.85;

            pthread_mutex_unlock(&PR_graph.neighbors[j]->lock);
        }
   }
    
}

double long PR_sum(long vertex){

    double long sum = 0;
    
    vertex_t* temp = PR_graph.neighbors[vertex];
    while(temp){
        
        temp = temp->next;
        if(!temp){ break; }
        
        if(PR_graph.neighbors[temp->id]->outgoing_num != 0){
            sum += PR_graph.neighbors[temp->id]->rank / PR_graph.neighbors[temp->id]->outgoing_num;
        }
        
    }
    
    return sum;

}

void free_memory(){
    
    for(long i = 0; i < total_nodes; i++){
        free(PR_graph.neighbors[i]);
    }

    free(PR_graph.neighbors);
}

void generate_csv_file(){
    FILE *fp = fopen("pagerank.csv","w");
    if(fp == NULL){
        printf("could not generate csv file\n");
        exit(1);
    }

    fprintf(fp, "Node No., Pagerank\n");

    for(long i = 0; i < total_nodes; i++){
        fprintf(fp, "%ld, %Lf\n",i,PR_graph.neighbors[i]->rank);
    }
}

int main(int argc, char** argv){

    if(argc != 3){
        printf("Argument error\nUsage: ./ask1_4579 <filename> <num of threads>\n");
        exit(1);
    }

    char* filename = argv[1];
    total_threads = atoi(argv[2]);
    char buf[1000];
    long source, dest;
    total_nodes = 0;
    int* a;
    double exec_time;
    
    if(total_threads <= 0 || total_threads > 4){
        printf("Thread number acceptable: 1 - 4");
        exit(1);
    }
    pthread_t threads[total_threads];

    FILE *fp = fopen(filename, "r");
    if(fp == NULL){
        printf("File could not open\n");
        return -1;
    }

    /* Find number of nodes in the graph */
    while(fgets(buf,BUF_SIZE,fp)){
        switch(buf[0]){
            case '#':
                break;
            default:
                sscanf(buf, "%ld %ld", &source, &dest);
                total_nodes = source > dest ? source : dest;
        }
    }

    fclose(fp);
    
    total_nodes++;
    PR_graph = init_graph(total_nodes);

    FILE *fp2 = fopen(filename, "r");
    if(fp2 == NULL){
        printf("File could not open\n");
        return -1;
    }

    while(fgets(buf,BUF_SIZE,fp2)){
        switch(buf[0]){
            case '#':
                break;
            default:
                sscanf(buf, "%ld %ld", &source, &dest);
                vertex_insert(source,dest);
        }
    }

    fclose(fp2);


    clock_gettime(CLOCK_MONOTONIC, &start_time);
    for(int i = 0; i < total_threads; i++){
        a = malloc(sizeof(int));
        *a = i;

        if(i == total_threads - 1){
            flag = 1;
        }
        if(pthread_create(&threads[i], NULL, (void*)pageRank, a) == -1){
            printf("Error in generating threads\n");
            exit(1);
        }
    }

    for(int i = 0; i < total_threads; i++){
        if(pthread_join(threads[i], NULL) == -1){
            printf("Error in Joining thread\n");
            exit(1);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &finish_time);

    exec_time = (finish_time.tv_sec - start_time.tv_sec) + (finish_time.tv_nsec - start_time.tv_nsec) / 1.0e9;
    
    generate_csv_file();

    printf("Total execution time with %d threads: %f secs\n", total_threads ,exec_time);

    free_memory();

    return 0;
    
}
