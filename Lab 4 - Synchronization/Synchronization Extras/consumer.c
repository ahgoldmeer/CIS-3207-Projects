#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <pthread.h>

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
    pthread_mutex_t lock;
    pthread_cond_t is_not_full;
    pthread_cond_t is_not_empty;
}Buff;

typedef struct argument{
    struct buffer buff1;
    struct buffer buff2;
}Arg;

typedef struct arg2{
    struct buffer buff;
    char id[100];
}Arg2;

void put(Buff *buffer, Prod *ReadBuff);

int get(Buff *buffer, char *id);

void* distributor(void* args){
    Arg* arguments = (Arg*) args;
    struct buffer prod1 = arguments->buff1;
    struct buffer prod2 = arguments->buff2;
    int count = 0;
    while(count != 2){
            struct product ReadBuff;
            read(STDIN_FILENO, &ReadBuff, sizeof(struct product));
            if(ReadBuff.count == -1){
                count++;
            }
            if(strcmp(ReadBuff.id, "product_1") == 0){
                put(&prod1, &ReadBuff);
                pthread_cond_signal(&(prod1.is_not_empty));
                pthread_cond_broadcast(&(prod1.is_not_empty));
                //get(&prod1, &ReadBuff);//For Testing Only
            }else{
                put(&prod2, &ReadBuff);
                pthread_cond_broadcast(&(prod2.is_not_empty));
                //get(&prod2, &ReadBuff);//For Testing Only
            }
    }
    // pthread_cond_broadcast(&(prod1.is_not_empty));
    // pthread_cond_broadcast(&(prod2.is_not_empty));
    pthread_exit(NULL);
}

void* ProdThread(void* args){
    Arg2* arguments = (Arg2*) args;
    struct buffer prod = arguments->buff;
    char id[100];
    strcpy(id, arguments->id);
    int count = 0;
    while(count != 1){
        puts("Before Get\n");
        int getVal = get(&prod, id);
        if(getVal == -1){
            count++;
        }
        printf("Get value: %d\n\n", getVal);
    }
    pthread_exit(NULL);
}

int main(){
    struct buffer prod1 = {.count = 0, .get = 0, .put = 0, .size = 20, .lock = PTHREAD_MUTEX_INITIALIZER, .is_not_full = PTHREAD_COND_INITIALIZER, .is_not_empty = PTHREAD_COND_INITIALIZER};
    prod1.prod = malloc(sizeof(int) * prod1.size);
    struct buffer prod2 = {.count = 0, .get = 0, .put = 0, .size = 25, .lock = PTHREAD_MUTEX_INITIALIZER, .is_not_full = PTHREAD_COND_INITIALIZER, .is_not_empty = PTHREAD_COND_INITIALIZER};
    prod2.prod = malloc(sizeof(int) * prod2.size);
    pthread_t threads[5];
    struct argument args = {.buff1 = prod1, .buff2 = prod2};
    if((pthread_create(&threads[0], NULL, &distributor, (void *) &args)) != 0){
        perror("Distributor Thread Creation Error");
        exit(EXIT_FAILURE);
    }
    struct arg2 args2 = {.buff = prod1, .id = "product_1"};
    if((pthread_create(&threads[1], NULL, &ProdThread, (void *) &args2)) != 0){
        perror("Distributor Thread Creation Error");
        exit(EXIT_FAILURE);
    }
    struct arg2 args3 = {.buff = prod2, .id = "product_2"};
    if((pthread_create(&threads[2], NULL, &ProdThread, (void *) &args3)) != 0){
        perror("Distributor Thread Creation Error");
        exit(EXIT_FAILURE);
    }
    // for(int i = 1; i < 3; i++){
    //     struct arg2 args2 = {.buff = prod1, .id = "product_1"};
    //     if((pthread_create(&threads[i], NULL, ProdThread, (void *) &args2)) != 0){
    //         perror("Thread Creation Error");
    //         exit(EXIT_FAILURE);
    //     }
    // }
    // for(int i = 3; i < 5; i++){
    //     struct arg2 args3 = {.buff = prod2, .id = "product_2"};
    //     if((pthread_create(&threads[i], NULL, ProdThread, (void *) &args3)) != 0){
    //         perror("Thread Creation Error");
    //         exit(EXIT_FAILURE);
    //     }
    // }
    // for(int i = 0; i < 5; i++){
    //     if(pthread_join(threads[i], NULL) != 0){
    //         perror("Join Error");
    //         exit(EXIT_FAILURE);
    //     }
    // }
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    //pthread_join(threads[2], NULL);
}

void put(Buff *buffer, Prod *ReadBuff){
    pthread_mutex_lock(&(buffer->lock));
    while(buffer->count >= buffer->size){
        pthread_cond_wait(&(buffer->is_not_full), &(buffer->lock));
    }
    buffer->prod[buffer->put] = ReadBuff->count;
    buffer->count = buffer->count + 1;
    buffer->put = (buffer->put + 1) % buffer->size;
    printf("PUT: ID: %s, Prod Count: %d, Prod Get: %d, Prod Put: %d, Prod Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
    pthread_cond_signal(&(buffer->is_not_empty));
    pthread_cond_broadcast(&(buffer->is_not_empty));
    pthread_mutex_unlock(&(buffer->lock));
    // pthread_cond_signal(&(buffer->is_not_empty));
    // pthread_cond_broadcast(&(buffer->is_not_empty));
}

int get(Buff *buffer, char *id){
    pthread_mutex_lock(&(buffer->lock));
    int get = 0;
    while(buffer->count <= 0){
        puts("Before Wait\n");
        pthread_cond_wait(&(buffer->is_not_empty), &(buffer->lock));
        puts("After Wait\n");
    }
    get = buffer->prod[buffer->get];
    buffer->prod[buffer->get] = 0;
    buffer->count = buffer->count - 1;
    buffer->get = (buffer->get + 1) % buffer->size;
    printf("GET: ID: %s, Prod Count: %d, Prod Get: %d, Prod Put: %d, Prod Size: %d\n", id, buffer->count, buffer->get, buffer->put, buffer->size);
    puts("Before Signal\n");
    pthread_cond_signal(&(buffer->is_not_full));
    pthread_cond_broadcast(&(buffer->is_not_full));
    puts("Before Unlock\n");
    pthread_mutex_unlock(&(buffer->lock));
    puts("After Unlock\n");
    // pthread_cond_signal(&(buffer->is_not_full));
    // pthread_cond_broadcast(&(buffer->is_not_full));

    return get;
}