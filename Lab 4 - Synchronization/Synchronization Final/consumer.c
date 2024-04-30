//This is a test

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h>

typedef struct product{
    char id[100];
    int count;
}Prod;

typedef struct buffer{
    int size;
    int *prod;
    int count;
    int get;
    int put;
    int consid;
    pthread_mutex_t lock;
    pthread_cond_t is_not_full;
    pthread_cond_t is_not_empty;
}Buff;

typedef struct argument{
    Buff* buff1;
    Buff* buff2;
}Arg;

typedef struct arg2{
    Buff* buff;
    char id[100];
}Arg2;

void put(Buff *buffer, Prod *ReadBuff);

int get(Buff *buffer, char *id);

void* distributor(void* args){
    Arg* arguments = (Arg*) args;
    Buff* prod1 = (Buff*) arguments->buff1;
    Buff* prod2 = (Buff*) arguments->buff2;
    int count = 0;
    while(count != 2){
            struct product ReadBuff;
            read(STDIN_FILENO, &ReadBuff, sizeof(struct product));
            if(ReadBuff.count == -1){
                count++;
            }
            if(strcmp(ReadBuff.id, "product_1") == 0){
                put(prod1, &ReadBuff);
            }else{
                put(prod2, &ReadBuff);
            }
    }
    fflush(stdout);
    pthread_exit(NULL);
}

void* ProdThread(void* args){
    Arg2* arguments = (Arg2*) args;
    Buff* prod = (Buff*) arguments->buff;
    char id[100];
    strcpy(id, arguments->id);
    int count = 0;
    while(count != 1){
        int getVal = get(prod, id);
        if(getVal == -1){
            count++;
        }
        // printf("Get value: %d\n\n", getVal);
    }
    fflush(stdout);
    pthread_exit(NULL);
}

int main(){
    struct buffer prod1 = {
        .count = 0, 
        .get = 0, 
        .put = 0, 
        .size = 20, 
        .lock = PTHREAD_MUTEX_INITIALIZER, 
        .is_not_full = PTHREAD_COND_INITIALIZER, 
        .is_not_empty = PTHREAD_COND_INITIALIZER
    };
    prod1.prod = malloc(sizeof(int) * prod1.size);
    struct buffer prod2 = {
        .count = 0, 
        .get = 0, 
        .put = 0, 
        .size = 25, 
        .lock = PTHREAD_MUTEX_INITIALIZER, 
        .is_not_full = PTHREAD_COND_INITIALIZER, 
        .is_not_empty = PTHREAD_COND_INITIALIZER
    };
    prod2.prod = malloc(sizeof(int) * prod2.size);

    // FILE* result = fopen("result.txt", "w+");
    // int desc = fileno(result);
    // dup2(desc, STDOUT_FILENO);

    int file = open("result.txt", O_RDWR | O_CREAT, 0777);
    dup2(file, STDOUT_FILENO);
    close(file);

    pthread_t threads[5];
    struct argument args = {.buff1 = &prod1, .buff2 = &prod2};
    if((pthread_create(&threads[0], NULL, distributor, (void *) &args)) != 0){
        perror("Distributor Thread Creation Error");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    struct arg2 args2 = {.buff = &prod1, .id = "product_1"};
    for(int i = 1; i < 3; i++){
        if((pthread_create(&threads[i], NULL, ProdThread, (void *) &args2)) != 0){
            perror("Thread Creation Error");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }
    struct arg2 args3 = {.buff = &prod2, .id = "product_2"};
    for(int i = 3; i < 5; i++){
        if((pthread_create(&threads[i], NULL, ProdThread, (void *) &args3)) != 0){
            perror("Thread Creation Error");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }
    for(int i = 0; i < 5; i++){
        if(pthread_join(threads[i], NULL) != 0){
            perror("Join Error");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }
    // fclose(result);
}

void put(Buff *buffer, Prod *ReadBuff){
    fflush(stdout);
    pthread_mutex_lock(&(buffer->lock));
    while(buffer->count == buffer->size){
        if((pthread_cond_wait(&(buffer->is_not_full), &(buffer->lock))) != 0){
            perror("Wait Failure");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }
    buffer->prod[buffer->put] = ReadBuff->count;
    buffer->count = buffer->count + 1;
    buffer->put = (buffer->put + 1) % buffer->size;
    // printf("PUT: ID: %s, Prod Count: %d, Prod Get: %d, Prod Put: %d, Prod Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
    // fflush(stdout);
    int sig = pthread_cond_signal(&(buffer->is_not_empty));
    if(sig != 0){
        perror("Signal Failure");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    pthread_mutex_unlock(&(buffer->lock));
}

int get(Buff *buffer, char *id){
    fflush(stdout);
    pthread_mutex_lock(&(buffer->lock));
    int get = 0;
    while(buffer->count == 0){
        if((pthread_cond_wait(&(buffer->is_not_empty), &(buffer->lock))) != 0){
            perror("Wait Failure");
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
    }
    get = buffer->prod[buffer->get];
    buffer->prod[buffer->get] = 0;
    buffer->count = buffer->count - 1;
    buffer->get = (buffer->get + 1) % buffer->size;
    buffer->consid = buffer->consid + 1;
    fflush(stdout);
    printf("Product ID: %s, Thread ID: %ld, Production Count: %d, Consumer Count: %d\n\n", id, pthread_self(), get, buffer->consid);
    // printf("ID: %s, Prod Count: %d, Prod Get: %d, Prod Put: %d, Prod Size: %d\n", id, buffer->count, buffer->get, buffer->put, buffer->size);
    int sig = pthread_cond_signal(&(buffer->is_not_full));
    if(sig != 0){
        perror("Signal Failure");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    pthread_mutex_unlock(&(buffer->lock));
    return get;
}