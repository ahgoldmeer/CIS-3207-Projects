// #include <string.h>
// #include <string.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <dirent.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <sys/stat.h>
// #include <time.h>
// #include <pthread.h>

// typedef struct product{
//     char id[100];
//     int count;
// }Prod;

// typedef struct buffer{
//     int size;
//     int *prod;
//     int count;
//     int get;
//     int put;
//     pthread_mutex_t lock;
//     pthread_cond_t full;
//     pthread_cond_t empty;
// }Buff;

// typedef struct argument{
//     struct buffer buff1;
//     struct buffer buff2;
// }Arg;

// void put(Buff *buffer, Prod *ReadBuff);

// int get(Buff *buffer, Prod *ReadBuff);

// void* distributor(void* args){
//     Arg* arguments = (Arg*) args;
//     struct buffer prod1 = arguments->buff1;
//     struct buffer prod2 = arguments->buff2;
//     int count = 0;
//     int getVal = 0;
//     while(count != 2){
//             struct product ReadBuff;
//             read(STDIN_FILENO, &ReadBuff, sizeof(struct product));
//             if(ReadBuff.count == -1){
//                 count++;
//             }
//             if(strcmp(ReadBuff.id, "product_1") == 0){
//                 put(&prod1, &ReadBuff);
//                 getVal = get(&prod1, &ReadBuff);//For Testing Only
//                 printf("Get value: %d\n\n", getVal);
//             }else{
//                 put(&prod2, &ReadBuff);
//                 getVal = get(&prod2, &ReadBuff);//For Testing Only
//                 printf("Get value: %d\n\n", getVal);

//             }
//     }
//     pthread_exit(NULL);
// }

// int main(){
//     struct buffer prod1 = {.count = 0, .get = 0, .put = 0, .size = 20, .lock = PTHREAD_MUTEX_INITIALIZER, .full = PTHREAD_COND_INITIALIZER, .empty = PTHREAD_COND_INITIALIZER};
//     prod1.prod = malloc(sizeof(int) * prod1.size);
//     struct buffer prod2 = {.count = 0, .get = 0, .put = 0, .size = 25, .lock = PTHREAD_MUTEX_INITIALIZER, .full = PTHREAD_COND_INITIALIZER, .empty = PTHREAD_COND_INITIALIZER};
//     prod2.prod = malloc(sizeof(int) * prod2.size);
//     pthread_t threads[5];
//     struct argument args = {.buff1 = prod1, .buff2 = prod2};
//     if((pthread_create(&threads[0], NULL, &distributor, (void *) &args)) != 0){
//         perror("Distributor Thread Creation Error");
//     }
//     pthread_join(threads[0], NULL);
//     // for(int i = 1; i < 3; i++){
//     //     if((pthread_create(&threads[i], NULL, Prod1Thread, (void *) &prod1)) != 0){
//     //         perror("Thread Creation Error");
//     //     }
//     // }
//     // for(int i = 3; i < 5; i++){
//     //     if((pthread_create(&threads[i], NULL, Prod2Thread, (void *) &prod2)) != 0){
//     //         perror("Thread Creation Error");
//     //     }
//     // }
//     // for(int i = 0; i < 5; i++){
//     //     pthread_join(threads[i], NULL);
//     // }
// }

// //New Version in Local for put and get with correct version of mutual exclusion

// // void put(Buff *buffer, Prod *ReadBuff){
// //     if(buffer->count != buffer->size){
// //         pthread_mutex_lock(&(buffer->lock));
// //     }else{
// //         pthread_cond_wait(&(buffer->full), &(buffer->lock));
// //         pthread_cond_signal(&(buffer->full));
// //     } 
// //     buffer->prod[buffer->put] = ReadBuff->count;
// //     buffer->count++;
// //     buffer->put = (buffer->put + 1) % buffer->size;
// //     printf("PUT: ID: %s, Prod Count: %d, Prod Get: %d, Prod Put: %d, Prod Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
// //     pthread_mutex_unlock(&(buffer->lock));
// // }


// // void get(Buff *buffer, Prod *ReadBuff){
// //     int get = 0;
// //     if(buffer->count != 0){
// //         pthread_mutex_lock(&(buffer->lock));
// //     }else{
// //         pthread_cond_wait(&(buffer->full), &(buffer->lock));
// //         pthread_cond_signal(&(buffer->full));
// //     }
// //     get = buffer->prod[buffer->get];
// //     buffer->prod[buffer->get] = 0;
// //     buffer->count = buffer->count - 1;
// //     buffer->get = (buffer->get + 1) % buffer->size;
// //     printf("GET: ID: %s, Prod1 Count: %d, Prod1 Get: %d, Prod1 Put: %d, Prod1 Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
// //     printf("Get value: %d\n\n", get); 
// //     pthread_mutex_unlock(&(buffer->lock));
// // }

// void put(Buff *buffer, Prod *ReadBuff){
//     // if(buffer->count != buffer->size){
//     //     pthread_mutex_lock(&(buffer->lock));
//     // }else{
//     //     pthread_cond_wait(&(buffer->full), &(buffer->lock));
//     //     pthread_cond_signal(&(buffer->full));
//     // } 
//     // buffer->prod[buffer->put] = ReadBuff->count;
//     // buffer->count++;
//     // buffer->put = (buffer->put + 1) % buffer->size;
//     // printf("PUT: ID: %s, Prod Count: %d, Prod Get: %d, Prod Put: %d, Prod Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
//     // pthread_mutex_unlock(&(buffer->lock));
//     pthread_mutex_lock(&(buffer->lock));
//     while(buffer->count == buffer->size){
//         pthread_cond_wait(&(buffer->full), &(buffer->lock));
//     }
//     buffer->prod[buffer->put] = ReadBuff->count;
//     buffer->count++;
//     buffer->put = (buffer->put + 1) % buffer->size;
//     printf("PUT: ID: %s, Prod Count: %d, Prod Get: %d, Prod Put: %d, Prod Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
//     //pthread_cond_signal(&(buffer->full));
//     pthread_cond_signal(&(buffer->empty));//???
//     pthread_mutex_unlock(&(buffer->lock));
// }


// int get(Buff *buffer, Prod *ReadBuff){
//     // int get = 0;
//     // if(buffer->count != 0){
//     //     pthread_mutex_lock(&(buffer->lock));
//     // }else{
//     //     pthread_cond_wait(&(buffer->full), &(buffer->lock));
//     //     pthread_cond_signal(&(buffer->full));
//     // }
//     // get = buffer->prod[buffer->get];
//     // buffer->prod[buffer->get] = 0;
//     // buffer->count = buffer->count - 1;
//     // buffer->get = (buffer->get + 1) % buffer->size;
//     // printf("GET: ID: %s, Prod1 Count: %d, Prod1 Get: %d, Prod1 Put: %d, Prod1 Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
//     // printf("Get value: %d\n\n", get); 
//     // pthread_mutex_unlock(&(buffer->lock));
//     int get = 0;
//     pthread_mutex_lock(&(buffer->lock));
//     while(buffer->count == 0){
//         pthread_cond_wait(&(buffer->empty), &(buffer->lock));
//     }
//     get = buffer->prod[buffer->get];
//     buffer->prod[buffer->get] = 0;
//     buffer->count = buffer->count - 1;
//     buffer->get = (buffer->get + 1) % buffer->size;
//     printf("GET: ID: %s, Prod1 Count: %d, Prod1 Get: %d, Prod1 Put: %d, Prod1 Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
//     pthread_cond_signal(&(buffer->full));//???
//     pthread_mutex_unlock(&(buffer->lock));
//     return get;
// }

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
    pthread_cond_t full;
    pthread_cond_t empty;
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
                //get(&prod1, &ReadBuff);//For Testing Only
            }else{
                put(&prod2, &ReadBuff);
                //get(&prod2, &ReadBuff);//For Testing Only
            }
    }
    pthread_exit(NULL);
}

void* ProdThread(void* args){
    int i = 0;
    printf("Test %d\n", i++);
    Arg2* arguments = (Arg2*) args;
    printf("Test %d\n", i++);
    struct buffer prod = arguments->buff;
    printf("Test %d\n", i++);
    char id[100];
    printf("Test %d\n", i++);
    strcpy(id, arguments->id);
    printf("Test %d\n", i++);
    int count = 0;
    printf("Test %d\n", i++);
    while(count != 1){
        printf("Test %d\n", i++);
        int getVal = get(&prod, id);//For Testing Only
        if(getVal == -1){
            count++;
        }
        printf("Test %d\n", i++);
        printf("Get value: %d\n\n", getVal);
        printf("Test %d\n", i++);
    }
    printf("Test %d\n", i++);
    pthread_exit(NULL);
}

int main(){
    struct buffer prod1 = {.count = 0, .get = 0, .put = 0, .size = 20, .lock = PTHREAD_MUTEX_INITIALIZER, .full = PTHREAD_COND_INITIALIZER, .empty = PTHREAD_COND_INITIALIZER};
    prod1.prod = malloc(sizeof(int) * prod1.size);
    struct buffer prod2 = {.count = 0, .get = 0, .put = 0, .size = 25, .lock = PTHREAD_MUTEX_INITIALIZER, .full = PTHREAD_COND_INITIALIZER, .empty = PTHREAD_COND_INITIALIZER};
    prod2.prod = malloc(sizeof(int) * prod2.size);
    pthread_t threads[5];
    struct argument args = {.buff1 = prod1, .buff2 = prod2};
    if((pthread_create(&threads[0], NULL, &distributor, (void *) &args)) != 0){
        perror("Distributor Thread Creation Error");
    }
    //pthread_join(threads[0], NULL);
    for(int i = 1; i < 3; i++){
        struct arg2 args2 = {.buff = prod1, .id = "product_1"};
        if((pthread_create(&threads[i], NULL, ProdThread, (void *) &args2)) != 0){
            perror("Thread Creation Error");
        }
    }
    for(int i = 3; i < 5; i++){
        struct arg2 args3 = {.buff = prod2, .id = "product_2"};
        if((pthread_create(&threads[i], NULL, ProdThread, (void *) &args3)) != 0){
            perror("Thread Creation Error");
        }
    }
    for(int i = 0; i < 5; i++){
        pthread_join(threads[i], NULL);
    }
}

//Correct way to implement mutual exclusion:
//1) Acquire lock
//2) If buffer is full/empty --> cond_wait
//2b) What does cond_wait do: Releases lock, waits, and then REAQUIRES LOCK
//3) Run code
//4) Signal
//5) Unlock

//Locks should be outside of here?
//Should be controlling the calls instead, linked the the count variable in producer/consumer???

void put(Buff *buffer, Prod *ReadBuff){
    // if(buffer->count != buffer->size){
    //     pthread_mutex_lock(&(buffer->lock));
    // }else{
    //     pthread_cond_wait(&(buffer->full), &(buffer->lock));
    //     pthread_cond_signal(&(buffer->full));
    // } 
    // buffer->prod[buffer->put] = ReadBuff->count;
    // buffer->count++;
    // buffer->put = (buffer->put + 1) % buffer->size;
    // printf("PUT: ID: %s, Prod Count: %d, Prod Get: %d, Prod Put: %d, Prod Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
    // pthread_mutex_unlock(&(buffer->lock));
    pthread_mutex_lock(&(buffer->lock));
    while(buffer->count == buffer->size){
        pthread_cond_wait(&(buffer->full), &(buffer->lock));
    }
    buffer->prod[buffer->put] = ReadBuff->count;
    buffer->count++;
    buffer->put = (buffer->put + 1) % buffer->size;
    printf("PUT: ID: %s, Prod Count: %d, Prod Get: %d, Prod Put: %d, Prod Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
    //pthread_cond_signal(&(buffer->full));
    pthread_cond_signal(&(buffer->empty));//???
    pthread_mutex_unlock(&(buffer->lock));
}


int get(Buff *buffer, char *id){
    // int get = 0;
    // if(buffer->count != 0){
    //     pthread_mutex_lock(&(buffer->lock));
    // }else{
    //     pthread_cond_wait(&(buffer->full), &(buffer->lock));
    //     pthread_cond_signal(&(buffer->full));
    // }
    // get = buffer->prod[buffer->get];
    // buffer->prod[buffer->get] = 0;
    // buffer->count = buffer->count - 1;
    // buffer->get = (buffer->get + 1) % buffer->size;
    // printf("GET: ID: %s, Prod1 Count: %d, Prod1 Get: %d, Prod1 Put: %d, Prod1 Size: %d\n", ReadBuff->id, buffer->count, buffer->get, buffer->put, buffer->size);
    // printf("Get value: %d\n\n", get); 
    // pthread_mutex_unlock(&(buffer->lock));
    int get = 0;
    pthread_mutex_lock(&(buffer->lock));
    while(buffer->count == 0){
        pthread_cond_wait(&(buffer->empty), &(buffer->lock));
    }
    get = buffer->prod[buffer->get];
    buffer->prod[buffer->get] = 0;
    buffer->count = buffer->count - 1;
    buffer->get = (buffer->get + 1) % buffer->size;
    printf("GET: ID: %s, Prod Count: %d, Prod Get: %d, Prod Put: %d, Prod Size: %d\n", id, buffer->count, buffer->get, buffer->put, buffer->size);
    pthread_cond_signal(&(buffer->full));//???
    pthread_mutex_unlock(&(buffer->lock));
    return get;
}